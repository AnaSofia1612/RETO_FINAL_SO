/**
 * @file main.c
 * @brief Reto 03 SO — Editor de archivos con optimización de Bus I/O.
 *
 * Pipeline completo:
 *   [Texto del usuario]
 *       → editarTexto()          (Persona 1)
 *       → preparar_bloques()     (Persona 1)
 *       → comprimir_bloques()    (Persona 2) ← TU MÓDULO
 *       → guardar_archivo()      (Persona 3)
 *       → leer_archivo()         (Persona 3)
 *       → descomprimir_bloques() (Persona 2) ← TU MÓDULO
 *       → texto recuperado
 *
 * Para medir rendimiento:
 *   make strace   → strace -c ./editor
 *   make time     → time ./editor
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "io.h"
#include "editorTexto.h"
#include "compresion.h"

#define DIR_ARCHIVOS "archivos"
 

int main() {
    printf("=== Reto 03 SO — Editor con optimización I/O ===\n\n");

    char*  texto_usuario = NULL;
    char** bloques_4kb   = NULL;
    int    num_bloques   = 0;

    /* ═══════════════════════════════════════════════════════════════════
     * FASE 1 — CAPTURA Y SEGMENTACIÓN  
     * ═══════════════════════════════════════════════════════════════════ */
    printf("[FASE 1] Iniciando editor dinámico...\n");
    texto_usuario = editarTexto();

    if (texto_usuario == NULL || strlen(texto_usuario) == 0) {
        printf("Error: No se capturó texto o el editor falló.\n");
        if (texto_usuario) free(texto_usuario);
        return EXIT_FAILURE;
    }

    printf("\n[FASE 1] Segmentando texto en bloques de 4KB...\n");
    bloques_4kb = preparar_bloques(texto_usuario, &num_bloques);

    if (bloques_4kb == NULL) {
        printf("Error crítico al preparar los bloques.\n");
        free(texto_usuario);
        return EXIT_FAILURE;
    }

    printf("[FASE 1] %d bloque(s) de 4096 bytes generados.\n\n", num_bloques);

    /* Inspección técnica de bloques */
    printf("--- INSPECCIÓN DE BLOQUES ---\n");
    for (int i = 0; i < num_bloques; i++) {
        printf("Bloque [%d] dir=%p | primeros 40 chars: [%.40s...]\n",
               i, (void*)bloques_4kb[i], bloques_4kb[i]);
    }
    printf("\n");

    /* ═══════════════════════════════════════════════════════════════════
     * FASE 2 — COMPRESIÓN RLE + CONSTRUCCIÓN DEL PAYLOAD  
     * ═══════════════════════════════════════════════════════════════════ */
    printf("[FASE 2] Comprimiendo bloques con RLE...\n");

    int            payload_size = 0;
    unsigned char* payload      = comprimir_bloques(bloques_4kb,
                                                     num_bloques,
                                                     &payload_size);

    if (payload == NULL) {
        printf("Error crítico en la compresión.\n");
        goto cleanup_bloques;
    }

    /* Mostrar metadata del payload generado */
    imprimir_metadata(payload, payload_size);

    /* Ratio de compresión */
    int bytes_originales = (int)strlen(texto_usuario);
    printf("[FASE 2] Texto original : %d bytes\n", bytes_originales);
    printf("[FASE 2] Payload final  : %d bytes (incluyendo headers)\n",
           payload_size);
    if (bytes_originales > 0) {
        printf("[FASE 2] Ratio          : %.2f%%\n\n",
               100.0 * payload_size / bytes_originales);
    }

    /* ═══════════════════════════════════════════════════════════════════
     * FASE 3 — ESCRITURA A DISCO  
     * Usa open() + write() en bloques de 4KB → minimiza syscalls
     * ═══════════════════════════════════════════════════════════════════ */
    printf("[FASE 3] Guardando payload comprimido en disco...\n");
    guardar_archivo("file.bin", payload, payload_size);
    printf("[FASE 3] Archivo guardado: file.bin\n\n");

    /* El payload ya fue guardado; liberamos */
    free(payload);
    payload = NULL;

    /* ═══════════════════════════════════════════════════════════════════
     * FASE 4 — LECTURA CON mmap()  
     * mmap() mapea el archivo directo al espacio de direcciones del proceso
     * → 0 copias extra entre kernel space y user space
     * ═══════════════════════════════════════════════════════════════════ */
    printf("[FASE 4] Leyendo archivo con mmap()...\n");
    int            leido_size = 0;
    unsigned char* leido      = leer_archivo("file.bin", &leido_size);

    if (!leido) {
        printf("Error al leer file.bin\n");
        goto cleanup_bloques;
    }
    printf("[FASE 4] %d bytes leídos desde disco.\n\n", leido_size);

    /* ═══════════════════════════════════════════════════════════════════
     * FASE 5 — DESCOMPRESIÓN Y VERIFICACIÓN  
     * ═══════════════════════════════════════════════════════════════════ */
    printf("[FASE 5] Descomprimiendo y verificando integridad...\n");
    int   out_len     = 0;
    char* recuperado  = descomprimir_bloques(leido, leido_size, &out_len);

    if (recuperado) {
        printf("\n=== TEXTO RECUPERADO ===\n%s\n========================\n\n",
               recuperado);

        /* Verificación de integridad byte a byte */
        if (strcmp(texto_usuario, recuperado) == 0) {
            printf("[OK] Verificación de integridad: TEXTO IDÉNTICO AL ORIGINAL\n");
        } else {
            printf("[WARN] Diferencias detectadas entre original y recuperado.\n");
        }
        free(recuperado);
    }

    free(leido);

    /* ═══════════════════════════════════════════════════════════════════
     * LIMPIEZA DE MEMORIA
     * ═══════════════════════════════════════════════════════════════════ */
cleanup_bloques:
    printf("\n[SISTEMA] Liberando memoria dinámica...\n");

    if (texto_usuario) free(texto_usuario);

    if (bloques_4kb) {
        for (int i = 0; i < num_bloques; i++) free(bloques_4kb[i]);
        free(bloques_4kb);
    }

    printf("\n=== Pipeline completo. Analiza con: strace -c ./editor ===\n");
    return EXIT_SUCCESS;
}