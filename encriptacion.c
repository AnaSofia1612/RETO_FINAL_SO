#define _DEFAULT_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // Para explicit_bzero, que es más seguro para limpiar la memoria de la llave
#include "encriptacion.h"

/**
 * @brief Función interna (estática) que realiza la operación XOR byte a byte.
 * Al ser simétrica, sirve tanto para encriptar como para desencriptar.
 */
static unsigned char* procesar_xor(const unsigned char* data, int size, const char* llave, int* out_size) {
    if (data == NULL || size <= 0 || llave == NULL || out_size == NULL) {
        return NULL;
    }

    int longitud_llave = strlen(llave);
    if (longitud_llave == 0) {
        fprintf(stderr, "Error: La llave de cifrado no puede estar vacía.\n");
        return NULL;
    }

    // En cifrado XOR, el tamaño de salida es idéntico al de entrada
    *out_size = size;

    // Reservar memoria en RAM para el buffer resultante
    unsigned char* resultado = (unsigned char*)malloc(*out_size);
    if (resultado == NULL) {
        perror("Error al asignar memoria para el cifrado");
        return NULL;
    }

    // Aplicar la máscara XOR cíclica byte a byte
    for (int i = 0; i < size; i++) {
        // El operador % hace que la llave se repita cíclicamente si es más corta que los datos
        resultado[i] = data[i] ^ (unsigned char)llave[i % longitud_llave];
    }

    return resultado;
}

/**
 * @brief Implementación de la función pública para Encriptar.
 */
unsigned char* encriptar(unsigned char* data, int size, const char* llave, int* out_size) {
    if (llave == NULL) return NULL;
    
    int len_llave = strlen(llave);
    
    // Hacer copia local de la llave
    char* llave_local = malloc(len_llave + 1);
    if (!llave_local) return NULL;
    strcpy(llave_local, llave);
    
    // 1. Procesar el cifrado XOR con la copia local
    unsigned char* datos_cifrados = procesar_xor(data, size, llave_local, out_size);
    
    // 2. BORRAR la copia local inmediatamente
    explicit_bzero(llave_local, len_llave);
    free(llave_local);
    
    return datos_cifrados;
}

/**
 * @brief Implementación de la función pública para Desencriptar.
 */
unsigned char* desencriptar(unsigned char* data, int size, const char* llave, int* out_size) {
    if (llave == NULL) return NULL;
    
    int len_llave = strlen(llave);
    
    // Hacer copia local de la llave
    char* llave_local = malloc(len_llave + 1);
    if (!llave_local) return NULL;
    strcpy(llave_local, llave);
    
    // 1. Procesar el descifrado con la copia local
    unsigned char* datos_descifrados = procesar_xor(data, size, llave_local, out_size);
    
    // 2. BORRAR la copia local inmediatamente
    explicit_bzero(llave_local, len_llave);
    free(llave_local);
    
    return datos_descifrados;
}