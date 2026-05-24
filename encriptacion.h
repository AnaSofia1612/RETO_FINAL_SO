#ifndef ENCRIPTACION_H
#define ENCRIPTACION_H

/**
 * @file 
 * @brief 

/**
 * @brief Encripta un bloque de datos utilizando una llave simétrica.
 * * @param data Puntero a los datos en texto claro o comprimidos.
 * @param size Tamaño en bytes de los datos de entrada.
 * @param llave Cadena de caracteres que representa la contraseña de cifrado.
 * @param out_size Puntero donde se almacenará el tamaño final del buffer cifrado.
 * @return unsigned char* Puntero al nuevo buffer cifrado (asignado dinámicamente con malloc).
 * Debe ser liberado (free) por el llamador.
 */
unsigned char* encriptar(unsigned char* data, int size, const char* llave, int* out_size);

/**
 * @brief Desencripta un bloque de datos utilizando una llave simétrica.
 * * @param data Puntero a los datos cifrados.
 * @param size Tamaño en bytes de los datos cifrados de entrada.
 * @param llave Cadena de caracteres que representa la contraseña de descifrado.
 * @param out_size Puntero donde se almacenará el tamaño final del buffer descifrado.
 * @return unsigned char* Puntero al nuevo buffer descifrado (asignado dinámicamente con malloc).
 * Debe ser liberado (free) por el llamador.
 */
unsigned char* desencriptar(unsigned char* data, int size, const char* llave, int* out_size);

#endif 