#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#define TAM_INI 17
#define CANT_INI 0
#define POS_INVALIDA -1

typedef enum estados_celda {VACIO, OCUPADO, BORRADO} estado_celda_t;

/* Definición del struct auxiliar celda */
typedef struct celda{
    void* valor;
    char* clave;
    estado_celda_t estado;
} celda_t;

/* Definición del struct hash */
struct hash{
    celda_t* tabla;
    size_t tamanio;
    size_t cantidad_ocupados;
    size_t cantidad_borrados;
    hash_destruir_dato_t destruir_dato;
};

/* Definición del struct hash_iter */
struct hash_iter{
	size_t actual;
    const hash_t* hash;
};

/* *****************************************************************
 *                    FUNCIONES AUXILIARES
 * *****************************************************************/

size_t hash_func(char* str, size_t length) {
	size_t b = 378551;
	size_t a = 63689;
	size_t hash = 0;
	size_t i = 0;
	for (i = 0; i < length; str++, i++){
		hash = hash * a + (*str);
		a = a * b;
	}
	return hash;
}

size_t linear_probing(const hash_t *hash, size_t i){
    return (i + 1) % hash->tamanio;
}

bool necesita_redimension(hash_t* hash){
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
    celda_t* nueva_tabla = celda_crear((hash->tamanio)*2);   
    if(!nueva_tabla){
        return false;
    }
    celda_t* tabla_vieja = hash->tabla;
    size_t tamanio_viejo = hash->tamanio;
    hash->tabla = nueva_tabla;
    hash->tamanio = tamanio_viejo * 2;
    hash->cantidad_ocupados = CANT_INI;
    hash->cantidad_borrados = CANT_INI;
    for(int i = 0; i < tamanio_viejo; i++){
        if(tabla_vieja[i].estado == OCUPADO){
            hash_guardar(hash, tabla_vieja[i].clave, tabla_vieja[i].valor);
            free(tabla_vieja[i].clave);
        }
    }
    free(tabla_vieja);
    return true;
}

size_t buscar_pos(const hash_t* hash, const char* clave){
    char* copia_clave = strdup(clave);
    if (!copia_clave) return POS_INVALIDA;
    size_t pos = hash_func(copia_clave, strlen(copia_clave)) % hash->tamanio;
    size_t i = pos;
    while(hash->tabla[i].estado != VACIO){
        if (hash->tabla[i].estado == OCUPADO  &&  strcmp(hash->tabla[i].clave, copia_clave) == 0){
            free(copia_clave);
            return i;
        }
        i = linear_probing(hash, i);
        if (i == pos){
            break;
        }
    }
    free(copia_clave);
    return POS_INVALIDA;
}


/* *****************************************************************
 *                    PRIMITIVAS DEL HASH
 * *****************************************************************/

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
    if (necesita_redimension(hash)){
        hash_redimension(hash);
    }
    size_t pos = hash_func(copia_clave, strlen(copia_clave)) % hash->tamanio;
    size_t i = pos;
    while (hash->tabla[i].estado != VACIO){
        if (hash->tabla[i].estado == OCUPADO  && strcmp(hash->tabla[i].clave, copia_clave) == 0){
            void* dato_anterior = hash->tabla[i].valor;
            hash->tabla[i].valor = dato;
            if(hash->destruir_dato){
                hash->destruir_dato(dato_anterior);
            }
            free(copia_clave);
            return true;
        }
        i = linear_probing(hash, i);
        if (i == pos){
            break;
        }
    }
    hash->tabla[i].clave = copia_clave;
    hash->tabla[i].valor = dato;
    hash->tabla[i].estado = OCUPADO;
    hash->cantidad_ocupados++;
    return true;
    // char* copia_clave = strdup(clave);
    // if (!copia_clave) return false;
    // if (necesita_redimension(hash)){
    //     hash_redimension(hash);
    // }
    // size_t pos = buscar_pos(hash, clave);
    // if(pos != POS_INVALIDA){
    //     void* dato_anterior = hash->tabla[pos].valor;
    //     hash->tabla[pos].valor = dato;
    //     if(hash->destruir_dato){
    //         hash->destruir_dato(dato_anterior);
    //     }
    //     free(copia_clave);
    //     return true;
    // }
    // pos = hash_func(copia_clave, strlen(copia_clave)) % hash->tamanio;
    // hash->tabla[pos].clave = copia_clave;
    // hash->tabla[pos].valor = dato;
    // hash->tabla[pos].estado = OCUPADO;
    // hash->cantidad_ocupados++;
    // return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
    size_t pos = buscar_pos(hash, clave);
    if(pos == POS_INVALIDA) return NULL;
    free(hash->tabla[pos].clave);
    hash->tabla[pos].estado = BORRADO;
    hash->cantidad_ocupados--;
    hash->cantidad_borrados++;
    return hash->tabla[pos].valor;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    size_t pos = buscar_pos(hash, clave);
    if(pos == POS_INVALIDA) return NULL;
    return hash->tabla[pos].valor;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    size_t pos = buscar_pos(hash, clave);
    return pos != POS_INVALIDA;
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

/* *****************************************************************
 *                    PRIMITIVAS DEL ITERADOR
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if (!iter) return NULL;
    iter->hash = hash;
    if(iter->hash->cantidad_ocupados == 0){
        iter->actual = hash->tamanio;
        return iter;
    }
    size_t i = 0;
    while (iter->hash->tabla[i].estado != OCUPADO){
        i = linear_probing(iter->hash, i);
    }
    iter->actual = i;
    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return false;
    size_t pos = iter->actual;
    for(size_t i = pos+1; i < iter->hash->tamanio; i++){
        if(iter->hash->tabla[i].estado == OCUPADO){
            iter->actual = i;
            return true;
        }
    }
    iter->actual = iter->hash->tamanio;
	return false;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return NULL;
    return iter->hash->tabla[iter->actual].clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return iter->actual == iter->hash->tamanio;
}

void hash_iter_destruir(hash_iter_t* iter){
    free(iter);
}
