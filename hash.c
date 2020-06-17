#include <stdio.h>
#include "hash.h"
#define TAM_INI 7

enum estados_celda {OCUPADO, VACIO, BORRADO};

// Estructuras auxiliares

typedef struct celda{
    void* valor;
    char* clave;
    enum estados_celda estado;
} celda_t;

// Estructuras

struct hash{
    hash_destruir_dato_t destruir_dato;
    void** datos;
    size_t tamanio;
};

// Primitivas

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash->datos = malloc(sizeof(void*) * TAM_INI);
    if(!hash->datos){
        free(hash);
        return NULL;
    }
    hash->tamanio = TAM_INI;
    hash->destruir_dato = destruir_dato;
    return hash;
}