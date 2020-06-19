#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#define TAM_INI 7
#define CANT_INI 0

typedef enum estados_celda {VACIO, OCUPADO, BORRADO} estado_celda_t;

// Estructuras auxiliares

typedef struct celda{
    void* valor;
    char* clave;
    estado_celda_t estado;
} celda_t;

// Estructuras

struct hash{
    celda_t* tabla; // lo volvi a cambiar pq necesitamos un arreglo de struct, no uno de punter a struct.
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
    hash->tabla = calloc(1, sizeof(celda_t) * TAM_INI);
    if(!hash->tabla){
        free(hash);
        return NULL;
    }
    hash->tamanio = TAM_INI;
    hash->cantidad = CANT_INI;
    if(destruir_dato)
        hash->destruir_dato = destruir_dato;
    return hash;
}
