#include <stdio.h>
#include <string.h>
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
    celda_t** datos;
    size_t tamanio;
};

// Funcion de Hash

//Nose si meter esto aca, o si al final le mandamos el README con estos links
//https://medium.com/swlh/hash-tables-in-c-with-the-djb2-algorithm-21f14ba7ca88
//https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f

size_t hash_func(char *str, size_t tam_hash){
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % tam_hash;
}

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

//Todavia no la termine, tengo agregarle que encuentre una posicion donde guardar
//en caso de que la pos que devuelva hash este ocupada

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    char* copia_clave = strdup(clave);
    if(!copia_clave) return false;
    celda_t* celda = malloc(sizeof(celda_t));
    if(!celda){
        free(copia_clave);
        return false;
    }
    size_t pos = hash_func(copia_clave, hash->tamanio);
    if(hash->datos[pos]->estado == OCUPADO){
        void* dato_anterior = hash->datos[pos]->valor;
        hash->datos[pos]->valor = dato;
        hash->destruir_dato(dato_anterior);
    }
    celda->clave = copia_clave;
    celda->valor = dato;
    celda->estado = OCUPADO;
    return true;
}