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

size_t hash_func(char *str){
    unsigned long hash = 5381;
    int c;
    while((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

bool validar_redimension(hash_t* hash){
    size_t factor_de_carga = hash_cantidad(hash) / hash->tamanio;
    if(factor_de_carga < 0.7) return true;
    return false;
}

hash_t* hash_redimension(hash_t* hash){
    hash_destruir_dato_t destruir_dato;
    if(!hash->destruir_dato){
        destruir_dato = NULL;
    }else{
        destruir_dato = hash->destruir_dato;
    }
    hash_t* hash_nuevo = hash_crear(destruir_dato);
    if(!hash_nuevo) return NULL;
    hash_nuevo->tamanio = (hash->tamanio) * 2;
    for(int i = 0; i < hash->tamanio; i++){
        if(hash->tabla[i].estado == OCUPADO){
            char* clave_tabla_vieja = hash->tabla[i].clave;
            void* dato_tabla_vieja = hash_obtener(hash, clave_tabla_vieja);
            hash_guardar(hash_nuevo, clave_tabla_vieja, dato_tabla_vieja);
        }
    }
    hash_destruir(hash);
    return hash_nuevo;
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

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return false;
    size_t pos = hash_func(copia_clave) % hash->tamanio;
    size_t i = pos;
    while (hash->tabla[i].estado == OCUPADO){
        if (hash->tabla[i].clave == copia_clave){
            hash->tabla[i].valor = dato;
            return true;
        }
        i = (i + 1) % hash->tamanio;
        if (i == pos){
            //hash = hash_redimension(hash);
        }
    }
    hash->tabla[i].clave = copia_clave;
    hash->tabla[i].valor = dato;
    hash->tabla[i].estado = OCUPADO;
    hash->cantidad++;
    return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave) % hash->tamanio;
    size_t i = pos;
    while (hash->tabla[i].estado != VACIO){
        if (hash->tabla[i].estado == OCUPADO  &&  strcmp(hash->tabla[i].clave, clave) == 0){
            void* dato = hash->tabla[i].valor;
            free(hash->tabla[i].clave);
            free(copia_clave);
            hash->tabla[i].estado = BORRADO;
            return dato;

        }
        i = (i + 1) % hash->tamanio;
        if (i == pos){
            break;
        }
    }
    free(copia_clave);
    return NULL;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave) % hash->tamanio;
    size_t i = pos; 
    while (hash->tabla[i].estado != VACIO){
        if (hash->tabla[i].estado == OCUPADO  &&  strcmp(hash->tabla[i].clave, clave) == 0){
            void* dato = hash->tabla[i].valor;
            free(copia_clave);
            return dato;
        }
        i = (i + 1) % hash->tamanio;
        if (i == pos){
            break;
        }
    }
    free(copia_clave);
    return NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave) % hash->tamanio;
    size_t i = pos;
    while (hash->tabla[i].estado == OCUPADO){
        if (strcmp(hash->tabla[i].clave, clave) == 0){
            free(copia_clave);
            return true;
        }
        i = (i+1) % hash->tamanio;
        if (i == pos){
            break;
        }
    }
    free(copia_clave);
    return false;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}

void hash_destruir(hash_t *hash){
    for(int i = 0; i < hash->tamanio; i++){
        if(hash->tabla[i].estado == OCUPADO && hash->destruir_dato){
            hash->destruir_dato(hash->tabla[i].valor);
            free(hash->tabla[i].clave);
        }
    }
    free(hash->tabla);
    free(hash);
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
    return NULL;
}

bool hash_iter_avanzar(hash_iter_t *iter){
    return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    return NULL;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return true;
}

void hash_iter_destruir(hash_iter_t* iter){
}
