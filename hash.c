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
    celda_t* tabla;
    size_t tamanio;
    size_t cantidad;
    hash_destruir_dato_t destruir_dato;
};

struct hash_iter{
	celda_t* actual;
	celda_t* anterior;
    hash_t* hash;
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

celda_t* celda_crear(size_t tamanio_de_tabla, hash_t* hash){
    celda_t* celdas = calloc(1, sizeof(celda_t) * tamanio_de_tabla);
    if(!celdas) return NULL;
    return celdas;
}

hash_t* hash_redimension(hash_t* hash){
    hash_t* hash_nuevo = hash_crear(hash->destruir_dato);
    if(!hash_nuevo) return NULL;
    free(hash_nuevo->tabla);
    hash_nuevo->tabla = celda_crear((hash->tamanio)*2, hash);   
    if(!hash_nuevo->tabla){
        free(hash_nuevo);
        return NULL;
    }
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
    hash->tabla = celda_crear(TAM_INI, hash);
    if(!hash->tabla){
        free(hash);
        return NULL;
    }
    hash->tamanio = TAM_INI;
    hash->cantidad = CANT_INI;
    hash->destruir_dato = destruir_dato;
    return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return false;
    size_t pos = hash_func(copia_clave) % hash->tamanio;
    size_t i = pos;
    while (hash->tabla[i].estado == OCUPADO){
        if (strcmp(hash->tabla[i].clave, copia_clave) == 0){
            void* dato_anterior = hash->tabla[i].valor;
            hash->tabla[i].valor = dato;
            if(hash->destruir_dato){
                hash->destruir_dato(dato_anterior);
            }
            return true;
        }
        i = (i + 1) % hash->tamanio;
        if (i == pos){
            hash = hash_redimension(hash);
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
            hash->cantidad--;
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
        }else if(hash->tabla[i].estado == OCUPADO && !hash->destruir_dato){
            free(hash->tabla[i].clave);
        }
    }
    free(hash->tabla);
    free(hash);
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if (!iter) return NULL;
    iter->hash = hash;
    iter->actual = &hash->tabla[0];
    iter->anterior = NULL;
    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return false;
    char* copia_clave = strdup(iter->actual->clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave) % iter->hash->tamanio;
    size_t sig_pos = (pos + 1) % iter->hash->tamanio;

    celda_t* viejo_actual = iter->actual;
    iter->anterior = viejo_actual;
    iter->actual = &iter->hash->tabla[sig_pos];
	return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    return iter->actual->clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return !iter->actual;
}

void hash_iter_destruir(hash_iter_t* iter){
    free(iter);
}
