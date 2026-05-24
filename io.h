/**
 * @file io.h
 * @brief Módulo de I/O optimizado para el editor de archivos.
 * Escritura con open()/write() en bloques de 4KB y lectura con mmap().
 * Seguridad de llave con mlock() y explicit_bzero().
 */

#ifndef IO_H
#define IO_H

#include <stddef.h>

/* Guarda datos binarios en disco en bloques de 4KB */
void guardar_archivo(const char* filename, unsigned char* data, int size);

/* Lee archivo binario con mmap(), retorna buffer (liberar con free()) */
unsigned char* leer_archivo(const char* filename, int* size_out);

/* Bloquea la llave en RAM con mlock() para evitar que vaya al Swap */
void proteger_llave(unsigned char* llave, int size);

/* Borra la llave de RAM con explicit_bzero() y libera mlock() */
void borrar_llave(unsigned char* llave, int size);

#endif /* IO_H */