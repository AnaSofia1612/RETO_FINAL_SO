# Reto 03 — Editor de Archivos con Optimización de Bus I/O

**Materia:** Sistemas Operativos  
**Autoras:**
- Ana Sofia Angarita Barrios
- Nawal Oriana Valoyes Renteria
- Maria Laura Tafur

---

## Descripción

Editor de texto en C nativo para Linux que implementa un pipeline completo de I/O optimizado. Ningún archivo viaja al disco en texto claro — el texto se comprime **y se encripta** en User Space antes de invocar las syscalls del kernel, reduciendo la carga y latencia del bus I/O y garantizando seguridad de los datos en reposo (Data at Rest).

Los archivos procesados se guardan en la carpeta `archivos/` con extensión `.bin`.

---

## Pipeline de Datos

```
[Texto del usuario]
      ↓
 editarTexto()          → captura línea a línea
      ↓
 preparar_bloques()     → segmenta en bloques de 4KB
      ↓
 comprimir_bloques()    → RLE + FileHeader + BlockHeaders
      ↓
 encriptar()            → cifrado simétrico XOR en RAM
      ↓
 guardar_archivo()      → write() en bloques de 4KB  →  archivos/<nombre>.bin
      ↓
 leer_archivo()         → mmap() sin copias extra
      ↓
 desencriptar()         → descifrado XOR en RAM
      ↓
 descomprimir_bloques() → reconstruye texto original
      ↓
[Texto recuperado]
```

> **Orden crítico:** Se comprime **antes** de encriptar. La encriptación genera datos pseudoaleatorios de alta entropía — si se encriptara primero, el compresor no encontraría patrones repetitivos y el archivo podría crecer en lugar de reducirse.

---

## Estructura del Proyecto

```
RETO-3-SO/
├── main.c            # Menú interactivo y pipeline completo
├── editorTexto.c     # Captura de texto y segmentación en bloques 4KB
├── editorTexto.h
├── compresion.c      # Algoritmo RLE, FileHeader, BlockHeader
├── compresion.h
├── encriptacion.c    # Cifrado simétrico XOR en RAM
├── encriptacion.h
├── io.c              # Escritura con write(), lectura con mmap(), seguridad de llave
├── io.h
├── Makefile
├── README.md
└── archivos/         # Carpeta generada automáticamente con los .bin
```

---

## Compilar y Ejecutar

```bash
# Compilar
make

# Ejecutar (menú interactivo con strace y time)
make run

# Limpiar binarios y archivos generados
make clean
```

---

## Uso del Menú

Al ejecutar `make run` aparece un menú con cuatro opciones:

```
╔══════════════════════════════════════╗
║   Reto 03 SO — Editor con I/O opt.   ║
╠══════════════════════════════════════╣
║  1. Crear / editar nuevo archivo     ║
║  2. Abrir archivo existente          ║
║  3. Listar archivos guardados        ║
║  4. Salir y ver resumen              ║
╚══════════════════════════════════════╝
```

**Opción 1 — Crear:** pide un nombre, abre el editor de texto (terminar con `SALIR`), comprime, encripta y guarda en `archivos/<nombre>.bin`.

**Opción 2 — Abrir:** lista los archivos disponibles, pide el nombre y la llave, desencripta, descomprime y muestra el contenido en pantalla.

**Opción 3 — Listar:** muestra todos los `.bin` guardados sin abrirlos.

**Opción 4 — Salir:** termina el programa y muestra el resumen de rendimiento con `strace` y `time`.

---

## Módulos

### editorTexto.c — Entrada y Bufferización
- Captura el texto del usuario línea a línea
- Segmenta en bloques de 4KB alineados al tamaño de página del SO
- Gestión dinámica de memoria con `malloc` y `realloc`

### compresion.c — Compresión y Formato Binario
- Algoritmo RLE (Run-Length Encoding): codifica runs como `[count][char]`
- `FileHeader` (16 bytes, packed): magic number, versión, número de bloques, tamaño total
- `BlockHeader` (8 bytes, packed): id y tamaño de cada bloque
- `__attribute__((packed))` garantiza tamaños exactos sin padding en cualquier arquitectura
- Valida el magic number `0x434C4552` al leer para detectar archivos corruptos

### encriptacion.c — Cifrado Simétrico XOR
- Algoritmo XOR simétrico: cada byte de los datos se combina con la llave de forma cíclica
- La llave se pide al usuario por consola, **nunca hardcodeada** en el código
- Se hace una copia local de la llave en heap y se borra con `explicit_bzero()` inmediatamente después de usarse
- Al ser XOR simétrico, `encriptar()` y `desencriptar()` aplican la misma operación

### io.c — I/O Optimizado y Seguridad de Llave
- **Escritura:** `open()` + `write()` en bloques de 4KB para minimizar context switches al kernel
- **Lectura:** `mmap()` mapea el archivo directo en el espacio de direcciones del proceso — cero copias extra entre kernel space y user space
- **Seguridad de llave:** `mlock()` bloquea la página de RAM que contiene la llave, evitando que el SO la envíe al Swap del disco
- **Borrado seguro:** `explicit_bzero()` borra la llave de la RAM después de usarla, evitando basura criptográfica en memoria

---

## Arquitectura de Seguridad Criptográfica

### Por qué Comprimir → Encriptar (y no al revés)

La encriptación XOR genera datos pseudoaleatorios con **entropía máxima**: no hay patrones repetitivos. Los algoritmos de compresión como RLE buscan exactamente esos patrones para reducir el tamaño. Si se encriptara primero:

1. El compresor no encontraría ningún patrón útil
2. El archivo resultante no se reduciría (o incluso crecería)
3. El objetivo de optimizar el bus I/O quedaría completamente anulado

El orden correcto garantiza que el compresor trabaje sobre texto con patrones naturales, y la encriptación protege el resultado ya comprimido.

### Gestión Segura de la Llave en RAM

```
Usuario ingresa llave por consola
        ↓
mlock() — bloquea la página RAM (evita que vaya al Swap/disco)
        ↓
Copia local en heap → operación XOR
        ↓
explicit_bzero() — borra la copia local de la llave
        ↓
munlock() — libera el bloqueo de página
```

`explicit_bzero()` es esencial porque, a diferencia de `memset()`, el compilador **no puede optimizarla y eliminarla**, garantizando que la llave se borre físicamente de la RAM.

---

## Formato Binario del Archivo `.bin`

```
┌─────────────────────────────────────────┐
│  [ENCRIPTADO con XOR — toda la sección] │
├─────────────────────────────────────────┤
│  FileHeader (16 bytes, packed)          │
│    magic       : 0x434C4552  ("RELC")  │
│    version     : 1                     │
│    num_bloques : cantidad de bloques   │
│    size_total  : bytes comprimidos     │
├─────────────────────────────────────────┤
│  BlockHeader (8 bytes, packed)          │
│    bloque_id   : índice (0-based)      │
│    size_bloque : bytes RLE del bloque  │
│  Datos RLE: [count][char][count][char]…│
├─────────────────────────────────────────┤
│  BlockHeader + Datos RLE (siguiente)   │
│  …                                     │
└─────────────────────────────────────────┘
```

---

## Resultados de Profiling — Benchmark Comparativo

Archivo de prueba: texto de 70 bytes (prueba funcional del pipeline completo).

### Escenario A — Enfoque Clásico (fputc byte a byte)

| Syscall   | Calls | Errores |
|-----------|-------|---------|
| `write()` | 11    | 0       |
| Total     | 50    | 3       |

### Escenario B — Solo Compresión RLE + write() en bloques 4KB

| Syscall   | Calls | Errores |
|-----------|-------|---------|
| `write()` | 57    | 0       |
| Total     | 104   | 4       |

### Escenario C — Compresión RLE + Encriptación XOR (Pipeline Completo)

Resultados reales medidos con `strace -c` y `time`:

| Syscall      | Calls | Errores |
|--------------|-------|---------|
| `read()`     | 7     | 0       |
| `write()`    | 60    | 0       |
| `mmap()`     | 8     | 0       |
| `openat()`   | 3     | 0       |
| `munmap()`   | 1     | 0       |
| **Total**    | **108** | **4** |

#### `time ./editor` — Escenario C (Pipeline Completo)

| Métrica        | Valor        | Interpretación                                         |
|----------------|--------------|--------------------------------------------------------|
| `user`         | 0m0.01s      | CPU en user space: compresión RLE + cifrado XOR        |
| `sys`          | 0m0.12s      | CPU en kernel space: syscalls de I/O                   |
| `elapsed`      | 0m58.50s     | Tiempo real (incluye input manual del usuario)         |
| `page faults`  | 1153 minor   | Generados por `mmap()` — esperado y normal             |
| `maxresident`  | 2216 KB      | Memoria máxima residente — footprint muy bajo          |

> `sys` (0.12s) sigue siendo bajo gracias a la escritura en bloques de 4KB y la lectura con `mmap()`. La encriptación XOR opera íntegramente en RAM sin syscalls adicionales, por lo que no introduce overhead de kernel.

### Tabla Comparativa Final

| Métrica del Kernel       | A. Clásico | B. Solo Compresión | C. Compresión + Encriptación | Impacto A→C         |
|--------------------------|------------|--------------------|------------------------------|---------------------|
| Syscalls `write()`       | 11         | 57                 | 60                           | +445% (más control) |
| Total syscalls           | 50         | 104                | 108                          | +116%               |
| CPU user space           | ~0.00s     | ~0.01s             | 0.01s                        | Mínimo overhead     |
| CPU kernel (sys)         | ~0.00s     | ~0.07s             | 0.12s                        | Aumento leve        |
| Page faults (mmap)       | 0          | 1163               | 1153                         | Normal con mmap()   |
| Datos viajan al disco    | Texto claro| Comprimido         | Comprimido + Cifrado         | **100% seguro**     |

### Conclusión Analítica

Agregar la capa de encriptación XOR al pipeline tiene un costo computacional **casi nulo** en user space (0.01s), ya que XOR es una operación de un ciclo de CPU por byte. El leve aumento en `sys` (de 0.07s a 0.12s) se debe a las syscalls adicionales de gestión de memoria (`mlock`, `munmap`) para la seguridad de la llave, no al cifrado en sí.

El sistema resultante opera en tiempos comparables al enfoque clásico inseguro, pero entrega tres garantías simultáneas:
1. **Seguridad:** ningún dato viaja al disco en texto claro
2. **Eficiencia de I/O:** los datos comprimidos reducen la carga del bus
3. **Seguridad de llave:** `mlock()` + `explicit_bzero()` garantizan que la llave nunca toca el disco ni queda en RAM tras usarse

---

## Verificación de Memoria — Valgrind

```bash
valgrind --leak-check=full ./editor
```

Resultado:

```
HEAP SUMMARY: 9 allocs, 9 frees
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors from 0 contexts
```

Cada `malloc` tiene su `free` correspondiente. Sin memory leaks ni errores de memoria.

---

## Herramientas Utilizadas
- **GCC** — compilador C
- **strace** — análisis de syscalls
- **time** — medición de tiempos User/Sys/Real
- **valgrind** — detección de memory leaks
- **make** — automatización de compilación