/**
 * @file main.c
 * @brief Reto 03 SO — Editor de archivos con optimización de Bus I/O.
 *
 * Pipeline de datos:
 *   [Texto] -> [Bloques 4KB] -> [Compresión RLE]
 *   -> [guardar_archivo()] -> [file.bin]
 *   -> [leer_archivo()]    -> [descomprimir_rle()] -> [Texto recuperado]
 */

#include <stdio.h>
#include <stdlib.h>
#include "io.h"

int main() {
    printf("=== Reto 03 SO — Editor con optimización I/O ===\n\n");

    /*
     * Datos de prueba comprimidos en RLE.
     * En la integración final esto viene del módulo de compresión.
     * Formato: {cantidad, caracter} por par.
     */
    unsigned char datos[] = {
        3,'H', 2,'o', 1,'l', 1,'a', 1,' ',
        1,'M', 1,'u', 1,'n', 1,'d', 1,'o'
    };
    int size = sizeof(datos);

    /* Paso 1: guardar en disco */
    printf("--- PASO 1: Guardando ---\n");
    guardar_archivo("file.bin", datos, size);

    /* Paso 2: leer con mmap */
    printf("\n--- PASO 2: Leyendo ---\n");
    int size_leido = 0;
    unsigned char* leido = leer_archivo("file.bin", &size_leido);
    if (!leido) return 1;

    /* Paso 3: descomprimir y mostrar */
    printf("\n--- PASO 3: Descomprimiendo ---\n");
    int out_len = 0;
    char* texto = descomprimir_rle(leido, size_leido, &out_len);
    if (texto) {
        printf("[IO] Resultado: '%s'\n", texto);
        free(texto);
    }

    free(leido);
    printf("\n=== Pipeline OK — prueba con: strace -c ./editor ===\n");
    return 0;
}