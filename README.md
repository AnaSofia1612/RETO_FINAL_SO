# Reto 03 — Editor de Archivos con Optimización de Bus I/O

**Materia:** Sistemas Operativos  
**Autoras:**
- Ana Sofia Angarita Barrios
- Nawal Oriana Valoyes Renteria
- Maria Laura Tafur

---

## Descripción

Editor de texto en C nativo para Linux que implementa un pipeline completo de I/O optimizado. Ningún archivo viaja al disco en texto claro — el texto se comprime en User Space antes de invocar las syscalls del kernel, reduciendo la carga y latencia del bus I/O.

---

## Pipeline de Datos

[Texto] → [Bloques 4KB] → [Compresión RLE] → [file.bin en disco]
↓
[Texto recuperado] ← [Descompresión] ← [mmap()]

---

## Estructura del Proyecto

Reto_03_SO/
├── main.c          # Punto de entrada y pipeline completo
├── editorTexto.c   # Captura de texto y segmentación en bloques 4KB
├── editorTexto.h
├── compresion.c    # Algoritmo RLE y headers binarios
├── compresion.h
├── io.c            # Escritura con write() y lectura con mmap()
├── io.h
└── Makefile

---

## Compilar y Ejecutar

```bash
# Compilar
make

# Ejecutar
make run

# Profiling de syscalls
make strace

# Medir tiempos User/Sys/Real
make time

# Limpiar binarios
make clean
```

---

## Módulos

### editorTexto.c — Entrada y Bufferización
- Captura el texto del usuario
- Segmenta en bloques de 4KB alineados al tamaño de página del SO
- Gestión dinámica de memoria con `malloc` y `free`

### compresion.c — Compresión y Formato
- Algoritmo RLE (Run-Length Encoding)
- Header binario con magic number `0x434C4552`
- Structs empaquetados con `__attribute__((packed))`

### io.c — I/O Optimizado
- **Escritura:** `open()` + `write()` en bloques de 4KB para minimizar context switches
- **Lectura:** `mmap()` mapea el archivo directo en memoria sin copias extra
- **Descompresión:** reconstrucción del texto original desde el archivo binario

---

## Resultados de Profiling

### strace -c ./editor
| Syscall | Calls | Descripción |
|---------|-------|-------------|
| write() | 2     | Bloques de 4KB — mínimas interrupciones al kernel |
| mmap()  | 9     | Lectura directa en memoria |
| Total   | 42    | Syscalls totales |

### time ./editor
| Métrica | Valor    |
|---------|----------|
| real    | 0m0.003s |
| user    | 0m0.002s |
| sys     | 0m0.000s |

> El tiempo en sys es casi 0 gracias a la reducción de syscalls por bloques de 4KB y mmap().

---

## Herramientas Utilizadas
- **GCC** — compilador C
- **strace** — análisis de syscalls
- **time** — medición de tiempos
- **make** — automatización de compilación

