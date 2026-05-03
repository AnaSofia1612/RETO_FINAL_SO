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
#include "editorTexto.h"

int main() {
    printf("=== Reto 03 SO — Editor con optimización I/O ===\n\n");

   
   char *texto_usuario = NULL;
    char **bloques_4kb = NULL;
    int num_bloques = 0;

    /* --- FASE 1: CAPTURA Y SEGMENTACIÓN  --- */
    printf(" Iniciando editor dinámico...\n");
    texto_usuario = editarTexto(); 
    
    if (texto_usuario == NULL || strlen(texto_usuario) == 0) {
        printf("Error: No se capturó texto o el editor falló.\n");
        if(texto_usuario) free(texto_usuario);
        return EXIT_FAILURE;
    }

    printf("\n Segmentando texto en bloques de 4KB...\n");
    bloques_4kb = preparar_bloques(texto_usuario, &num_bloques);

    if (bloques_4kb == NULL) {
        printf("Error crítico al preparar los bloques.\n");
        free(texto_usuario);
        return EXIT_FAILURE;
    }

    
   
 printf("\n--- INSPECCIÓN TÉCNICA DE BLOQUES ---\n");
 for (int i = 0; i < num_bloques; i++) {
    printf("Bloque [%d] (Dirección: %p):\n", i, (void*)bloques_4kb[i]);
    // Imprimimos los primeros 40 bytes de cada bloque
    printf("  Contenido: [%.40s...]\n", bloques_4kb[i]);
    
    // Verificamos si el final del bloque tiene el memset (ceros)
    if (bloques_4kb[i][4095] == '\0') {
        printf("  Estado: Limpio (Terminador nulo al final OK)\n");
    }
 }
    

    printf("[SISTEMA] Se han generado %d bloque(s) de 4096 bytes.\n", num_bloques);
   /* --- LIMPIEZA DE MEMORIA (Orden Correcto) --- */
    printf("\n[SISTEMA] Liberando memoria dinámica...\n");

    // 1. Liberamos el texto original que devolvió editarTexto()
    if (texto_usuario != NULL) {
        free(texto_usuario); 
    }

    // 2. Liberamos CADA bloque de 4KB individualmente (los "hijos")
    if (bloques_4kb != NULL) {
        for (int i = 0; i < num_bloques; i++) {
            free(bloques_4kb[i]); // Liberamos la memoria de 4096 bytes
        }
        
        // 3. Finalmente liberamos el arreglo de punteros (el "padre")
        free(bloques_4kb); 
    }

    printf("\n=== Prueba finalizada con éxito ===\n");
    return EXIT_SUCCESS;

   
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