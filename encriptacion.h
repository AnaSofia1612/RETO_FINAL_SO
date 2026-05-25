/**
 * @file encriptacion.h
 * @brief Módulo de cifrado simétrico XOR en RAM.
 *        La llave nunca se hardcodea y se borra con explicit_bzero() tras usarse.
 */

#ifndef ENCRIPTACION_H
#define ENCRIPTACION_H

/**
 * @brief Encripta un bloque de datos utilizando una llave simétrica.
 * @param data     Puntero a los datos en texto claro o comprimidos.
 * @param size     Tamaño en bytes de los datos de entrada.
 * @param llave    Cadena de caracteres que representa la contraseña de cifrado.
 * @param out_size Puntero donde se almacenará el tamaño final del buffer cifrado.
 * @return unsigned char* Buffer cifrado (asignado con malloc; liberar con free()).
 */
unsigned char* encriptar(unsigned char* data, int size, const char* llave, int* out_size);

/**
 * @brief Desencripta un bloque de datos utilizando una llave simétrica.
 * @param data     Puntero a los datos cifrados.
 * @param size     Tamaño en bytes de los datos cifrados de entrada.
 * @param llave    Cadena de caracteres que representa la contraseña de descifrado.
 * @param out_size Puntero donde se almacenará el tamaño final del buffer descifrado.
 * @return unsigned char* Buffer descifrado (asignado con malloc; liberar con free()).
 */
unsigned char* desencriptar(unsigned char* data, int size, const char* llave, int* out_size);

#endif /* ENCRIPTACION_H */