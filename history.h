/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
#if!defined(HISTORY_H)
#define HISTORY_H

/**
 * @file  user.h
 * @brief Contiene librerie delle funzioni implementate in history.c
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <message.h>
#include <pthread.h>

/**
 * @struct history
 * @brief contiene elementi di una history
 *
 * @var testa puntatore alla testa della history
 * @var fine puntatore ultimo elemento della history
 * @var msg_in_hist numero msg presenti in history
 * @var messaggi array di message_t con eleneco messaggi
 */
typedef struct history{
	size_t testa;
	size_t fine;
	int msg_in_hist;
	message_t **messaggi;
}storia;

/**
 * @function initHist
 * @brief inizializza history
 *
 * @param dim_hist dimensione della history
 * @param mtx variabile di mutex della history
 *
 * @returns history inizializzata
 */
storia * initHist(int dim_hist,pthread_mutex_t mtx);

/**
 * @function add
 * @brief aggiunge messaggi alla history
 *
 * @param s history in cui aggiungere msg
 * @param msg messaggio da aggiungere
 * @param mtx variabile di mutex della history
 * @param dim_hist dimensione della history
 */
void add( storia *s,message_t *msg,pthread_mutex_t mtx,int dim_hist);

/**
 * @function readConta
 * @brief conta il numero di messaggi presenti in history
 *
 * @param s history in cui conta i messaggi presenti
 * @param mtx variabile di mutex della history
 *
 * @returns numero dei messaggi presenti in history
 */
int readConta(storia *s,pthread_mutex_t mtx);

/**
 * @function destroyhist
 * @brief distrugge la history
 *
 * @param s history da distruggere
 * @param dim_hist dimensione della history da distruggere
 */
void destroyhist(storia *s,int dim_hist);

#endif
