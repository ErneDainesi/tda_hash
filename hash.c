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
    size_t cantidad_ocupados;
    size_t cantidad_borrados;
    hash_destruir_dato_t destruir_dato;
};

struct hash_iter{
	celda_t* actual;
	celda_t* anterior;
    const hash_t* hash;
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
    size_t factor_de_carga = (hash_cantidad(hash) + hash->cantidad_borrados) / hash->tamanio;
    if(factor_de_carga > 0.7) return true;
    return false;
}

celda_t* celda_crear(size_t tamanio_de_tabla){
    celda_t* celdas = calloc(1, sizeof(celda_t) * tamanio_de_tabla);
    if(!celdas) return NULL;
    return celdas;
}

bool hash_redimension(hash_t* hash){
    //creo una nueva tabla del doble del tamanio
    celda_t* nueva_tabla = celda_crear((hash->tamanio)*2);   
    if(!nueva_tabla){
        return false;
    }
    //me guardo la tabla vieja 
    celda_t* tabla_vieja = hash->tabla;
    size_t tamanio_viejo = hash->tamanio;
    hash->tabla = nueva_tabla;
    hash->tamanio = tamanio_viejo * 2;
    hash->cantidad_ocupados = CANT_INI;
    hash->cantidad_borrados = CANT_INI;
    //recorro todo el hash viejo y si las celdas estan ocupadas las guardo
    //en la nueva tabla
    for(int i = 0; i < tamanio_viejo; i++){
        if(tabla_vieja[i].estado == OCUPADO){
            hash_guardar(hash, tabla_vieja[i].clave, tabla_vieja[i].valor);
            free(tabla_vieja[i].clave);
        }
    }
    free(tabla_vieja);
    return true;
}


// Primitivas

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash->tabla = celda_crear(TAM_INI);
    if(!hash->tabla){
        free(hash);
        return NULL;
    }
    hash->tamanio = TAM_INI;
    hash->cantidad_ocupados = CANT_INI;
    hash->cantidad_borrados = CANT_INI;
    hash->destruir_dato = destruir_dato;
    return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return false;
    if (validar_redimension(hash)){
        hash_redimension(hash);
    }
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
            break;
        }
    }
    hash->tabla[i].clave = copia_clave;
    hash->tabla[i].valor = dato;
    hash->tabla[i].estado = OCUPADO;
    hash->cantidad_ocupados++;
    return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave) % hash->tamanio;
    size_t i = pos;
    while (hash->tabla[i].estado != VACIO){
        if (hash->tabla[i].estado == OCUPADO  &&  strcmp(hash->tabla[i].clave, copia_clave) == 0){
            void* dato = hash->tabla[i].valor;
            free(hash->tabla[i].clave);
            free(copia_clave);
            hash->tabla[i].estado = BORRADO;
            hash->cantidad_ocupados--;
            hash->cantidad_borrados++;
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
        if (hash->tabla[i].estado == OCUPADO  &&  strcmp(hash->tabla[i].clave, copia_clave) == 0){
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
        if (strcmp(hash->tabla[i].clave, copia_clave) == 0){
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
    return hash->cantidad_ocupados;
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
    iter->anterior = NULL;
    if (iter->hash->cantidad_ocupados == 0){
        iter->actual = NULL;
        return iter;
    }
    size_t i = 0;
    while (iter->hash->tabla[i].estado != OCUPADO){
        i = (i+1) % iter->hash->tamanio;
    }
    iter->actual = &iter->hash->tabla[i];
    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return false;
    if (iter->hash->cantidad_ocupados == 0) return false;
    celda_t* viejo_actual = iter->actual;
    iter->anterior = viejo_actual;
    char* copia_clave = strdup(iter->actual->clave);
    if (!copia_clave) return NULL;
    size_t pos = hash_func(copia_clave) % iter->hash->tamanio;
    size_t i = pos + 1;
    while (!&iter->hash->tabla[i] || iter->hash->tabla[i].estado != OCUPADO){
        i = (i+1) % iter->hash->tamanio;
    }
    iter->actual = &iter->hash->tabla[i];
	return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if (iter->hash->cantidad_ocupados == 0){
        return NULL;
    }
    return iter->actual->clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return !iter->actual;
}

void hash_iter_destruir(hash_iter_t* iter){
    free(iter);
}
