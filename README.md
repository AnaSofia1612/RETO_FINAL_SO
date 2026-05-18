# Reto 03 — Editor de Archivos con Optimización de Bus I/O

**Materia:** Sistemas Operativos  
**Autoras:**
- Ana Sofia Angarita Barrios
- Nawal Oriana Valoyes Renteria
- Maria Laura Tafur

---

## Descripción

Editor de texto en C nativo para Linux que implementa un pipeline completo de I/O optimizado. Ningún archivo viaja al disco en texto claro — el texto se comprime en User Space antes de invocar las syscalls del kernel, reduciendo la carga y latencia del bus I/O.

Los archivos comprimidos se guardan en la carpeta `archivos/` con extensión `.bin`.

---

## Pipeline de Datos

```
[Texto del usuario]
      ↓
 editarTexto()        → captura línea a línea
      ↓
 preparar_bloques()   → segmenta en bloques de 4KB
      ↓
 comprimir_bloques()  → RLE + FileHeader + BlockHeaders
      ↓
 encriptar()          → cifrado simétrico en RAM (pendiente)
      ↓
 guardar_archivo()    → write() en bloques de 4KB  →  archivos/<nombre>.bin
      ↓
 leer_archivo()       → mmap() sin copias extra
      ↓
 desencriptar()       → descifrado en RAM (pendiente)
      ↓
 descomprimir_bloques() → reconstruye texto original
      ↓
[Texto recuperado]
```

---

## Estructura del Proyecto

```
RETO-3-SO/
├── main.c            # Menú interactivo y pipeline completo
├── editorTexto.c     # Captura de texto y segmentación en bloques 4KB
├── editorTexto.h
├── compresion.c      # Algoritmo RLE, FileHeader, BlockHeader
├── compresion.h
├── encriptacion.c    # Cifrado simétrico en RAM (pendiente)
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

# Ejecutar (menú interactivo)
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
║  4. Salir                            ║
╚══════════════════════════════════════╝
```

**Opción 1 — Crear:** pide un nombre, abre el editor de texto (terminar con `SALIR`), comprime y guarda en `archivos/<nombre>.bin`.

**Opción 2 — Abrir:** lista los archivos disponibles, pide el nombre y muestra el contenido descomprimido en pantalla.

**Opción 3 — Listar:** muestra todos los `.bin` guardados sin abrirlos.

**Opción 4 — Salir:** termina el programa y muestra el resumen de rendimiento con strace y time.

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

### encriptacion.c — Cifrado Simétrico *(pendiente)*
- Algoritmo RC4 o XOR con llave simétrica
- La llave se pide al usuario por consola, nunca hardcodeada
- La llave se borra con `explicit_bzero()` después de usarse

### io.c — I/O Optimizado y Seguridad de Llave
- **Escritura:** `open()` + `write()` en bloques de 4KB para minimizar context switches al kernel
- **Lectura:** `mmap()` mapea el archivo directo en el espacio de direcciones del proceso — cero copias extra entre kernel space y user space
- **Seguridad de llave:** `mlock()` bloquea la página de RAM que contiene la llave, evitando que el SO la envíe al Swap del disco
- **Borrado seguro:** `explicit_bzero()` borra la llave de la RAM después de usarla, evitando basura criptográfica en memoria

---

## Formato Binario del Archivo `.bin`

```
┌─────────────────────────────────────────┐
│  FileHeader (16 bytes, packed)           │
│    magic       : 0x434C4552  ("RELC")   │
│    version     : 1                      │
│    num_bloques : cantidad de bloques    │
│    size_total  : bytes comprimidos      │
├─────────────────────────────────────────┤
│  BlockHeader (8 bytes, packed)           │
│    bloque_id   : índice (0-based)       │
│    size_bloque : bytes RLE del bloque   │
│  Datos RLE: [count][char][count][char]… │
├─────────────────────────────────────────┤
│  BlockHeader + Datos RLE (siguiente)    │
│  …                                      │
└─────────────────────────────────────────┘
```

---

## Resultados de Profiling — Benchmark Comparativo

### Escenario A — Enfoque Clásico (fputc byte a byte)
| Syscall   | Calls | 
|-----------|-------|
| `write()` | 11    |
| Total     | 50    |
| Errores   | 3     |

### Escenario B — Compresión RLE + write() en bloques 4KB
| Syscall   | Calls |
|-----------|-------|
| `write()` | 57    |
| Total     | 104   |
| Errores   | 4     |

### Escenario C — Compresión + Encriptación *(pendiente)*
> Se agregará cuando se integre el módulo de encriptación.

### `time ./editor` — Escenario B
| Métrica       | Valor       | Interpretación |
|---------------|-------------|----------------|
| `user`        | 0m0.00s     | CPU en user space (compresión RLE) |
| `sys`         | 0m0.07s     | CPU en kernel space (syscalls) |
| `elapsed`     | 1m41.85s    | Tiempo real incluyendo input del usuario |
| `page faults` | 1163 minor  | Generados por mmap() — esperado y normal |

> `sys` es bajo gracias a la escritura en bloques de 4KB y lectura con `mmap()`.

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

> Cada `malloc` tiene su `free` correspondiente. Sin memory leaks ni errores de memoria.

---

## Herramientas Utilizadas
- **GCC** — compilador C
- **strace** — análisis de syscalls
- **time** — medición de tiempos User/Sys/Real
- **valgrind** — detección de memory leaks
- **make** — automatización de compilación