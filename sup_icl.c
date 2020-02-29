/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file sup_icl.c
 *
 * support file for icl_hash.c implements in mutex some functions.
 */
/* $Id: icl_hash.c 2838 2011-11-22 04:25:02Z mfaverge $ */
/* $UTK_Copyright: $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <config.h>
#include "icl_hash.h"

#include <limits.h>



/* -- Array contenente variabili di mutua esclusione  -- */
/* -- NUM_VAR_MUX_HASH in config.h --*/
static pthread_mutex_t array_mux[NUM_VAR_MUX_HASH] = {PTHREAD_MUTEX_INITIALIZER};

/**
 * Search for an entry in a hash table.
 *
 * @param ht -- the hash table to be searched
 * @param key -- the key of the item to search for
 *
 * @returns pointer to the data corresponding to the key.
 *   If the key was not found, returns NULL.
 */
void *
icl_hash_find_sup(icl_hash_t *ht, void* key)
{

    int val = hash_pjw(key);

    int hashval = val % MAX_DIM_HASH;

    int valmux = (hashval % NUM_VAR_MUX_HASH);


    if(!ht || !key) return NULL;


    pthread_mutex_lock(&array_mux[valmux]);
    void * ok = icl_hash_find(ht,key);
    pthread_mutex_unlock(&array_mux[valmux]);

    return ok;
}

/**
 * Insert an item into the hash table.
 *
 * @param ht -- the hash table
 * @param key -- the key of the new item
 * @param data -- pointer to the new item's data
 *
 * @returns pointer to the new item.  Returns NULL on error.
 */
icl_entry_t *
icl_hash_insert_sup(icl_hash_t *ht, void* key, void *data)
{


    int val = hash_pjw(key);

    int hashval = val % MAX_DIM_HASH;

    int valmux = (hashval % NUM_VAR_MUX_HASH);

    if(!ht || !key) return NULL;


    pthread_mutex_lock(&array_mux[valmux]);
    icl_entry_t * ok = icl_hash_insert(ht,key,data);
    pthread_mutex_unlock(&array_mux[valmux]);
    return ok;
}

/**
 * Free one hash table entry located by key (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param key -- the key of the new item
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns 0 on success, -1 on failure.
 */
int icl_hash_delete_sup(icl_hash_t *ht, void* key, void (*free_key)(void*), void (*free_data)(void*))
{

    int val = hash_pjw(key);

    int hashval = val % MAX_DIM_HASH;

    int valmux = (hashval % NUM_VAR_MUX_HASH);

    if(!ht || !key) return -1;

    pthread_mutex_lock(&array_mux[valmux]);
    int val2 = icl_hash_delete(ht,key, free_key, free_data);
    pthread_mutex_unlock(&array_mux[valmux]);
    return val2;
}


/**
*
* @function icl_m_user_list
* @brief funzione che crea elenco degli utenti registrati
*
* @param h tabella hash
* @param max_name dimensione nome utente
* @param nusers numero utenti registrati
* @returns elenco stringhe nomi degli utenti registrati
**/
char **icl_m_user_list(icl_hash_t* h,int max_name,int nusers){

    char **lista = malloc(sizeof(char*)*nusers);
    for (int i =0;i<nusers;i++){
        lista[i]=malloc(sizeof(char)*max_name+1);
    }
    int i;
    int dim =0;
    int start = 0;
    int end = MAX_DIM_HASH/NUM_VAR_MUX_HASH;
    int part = end;

    icl_entry_t *bucket, *curr;
    /* -- guardo in mutua esclusione zona per zona la tabella hash -- */
    for(i=0;i<NUM_VAR_MUX_HASH;i++){
       pthread_mutex_lock(&(array_mux[i]));
        for(int j = start;j<end;j++){
            bucket = h->buckets[j];

            for(curr=bucket; curr!=NULL; curr=curr->next ) {
                if(curr->key!=NULL){
                    strcpy(lista[dim],(char*)curr->key);
                    dim++;


                }

            }
        }
        pthread_mutex_unlock(&(array_mux[i]));
        start= end;
        end = end+part;

    }

    return lista;
}
