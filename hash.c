#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#define TAM_INI 7
#define CANT_INI 0

enum estados_celda {OCUPADO, VACIO, BORRADO};

// Estructuras auxiliares

typedef struct celda{
    void* valor;
    char* clave;
    enum estados_celda estado;
} celda_t;

// Estructuras

struct hash{
    celda_t* tabla;
    size_t tamanio;
    size_t cantidad;
    hash_destruir_dato_t destruir_dato;
};

// Funciones auxiliares

//Nose si meter esto aca, o si al final le mandamos el README con estos links
//https://medium.com/swlh/hash-tables-in-c-with-the-djb2-algorithm-21f14ba7ca88
//https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f

size_t hash_func(char *str, size_t tam_hash){
    unsigned long hash = 5381;
    int c;
    while((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % tam_hash;
}
// Primitivas

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash->tabla = malloc(sizeof(celda_t) * TAM_INI);
    if(!hash->tabla){
        free(hash);
        return NULL;
    }
    hash->tamanio = TAM_INI;
    hash->cantidad = CANT_INI;
    for(int i = 0; i < hash->tamanio; i++){
        hash->tabla[i].estado = VACIO;
    }
    hash->destruir_dato = destruir_dato;
    return hash;
}

void *hash_borrar(hash_t *hash, const char *clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave, hash->tamanio);
    if (hash->tabla[pos].estado == OCUPADO){
        void* dato = hash->tabla[pos].valor;
        free(hash->tabla[pos].clave);
        free(copia_clave);
        hash->tabla[pos].estado = BORRADO;
        return dato;
    }
    return NULL;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave, hash->tamanio);
    if (hash->tabla[pos].estado == OCUPADO){
        void* dato = hash->tabla[pos].valor;
        return dato;
    }
    free(copia_clave);
    return NULL;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}