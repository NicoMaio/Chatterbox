/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file
 *
 * Header file for icl_hash routines.
 *
 */
/* $Id$ */
/* $UTK_Copyright: $ */

#ifndef sup_icl_hash_h
#define sup_icl_hash_h

#include <pthread.h>
#include "icl_hash.h"
#include <stdio.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif





/**
 * Search for an entry in a hash table.
 *
 * @param ht -- the hash table to be searched
 * @param key -- the key of the item to search for
 *
 * @returns pointer to the data corresponding to the key.
 *   If the key was not found, returns NULL.
 */
void
* icl_hash_find_sup(icl_hash_t *, void* );

/**
 * Insert an item into the hash table.
 *
 * @param ht -- the hash table
 * @param key -- the key of the new item
 * @param data -- pointer to the new item's data
 *
 * @returns pointer to the new item.  Returns NULL on error.
 */
icl_entry_t
* icl_hash_insert_sup(icl_hash_t *, void*, void *);

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
int icl_hash_delete_sup( icl_hash_t *ht, void* key, void (*free_key)(void*), void (*free_data)(void*) );

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
char **icl_m_user_list(icl_hash_t* h,int max_name,int nusers);

#define icl_hash_foreach(ht, tmpint, tmpent, kp, dp)    \
    for (tmpint=0;tmpint<ht->nbuckets; tmpint++)        \
        for (tmpent=ht->buckets[tmpint];                                \
             tmpent!=NULL&&((kp=tmpent->key)!=NULL)&&((dp=tmpent->data)!=NULL); \
             tmpent=tmpent->next)


#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* sup_icl_hash_h */
