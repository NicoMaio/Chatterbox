/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file  queue.c
 * @brief Contiene implementazione funzioni dichiarate in queue.h
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

/* -- Variabili di condizionamento e di mutua esclusione -- */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

/**
 * @function initQueue
 * @brief inizializza la coda
 *
 * @returns una queue inizializzata
 */
Queue *initQueue()
{
  Queue *q = malloc(sizeof(Queue));
  q->head = NULL;
  q->last = NULL;
  return q;
}

/**
 * @function DeleteQueue
 * @brief elimina coda
 *
 * @param q puntatore a coda da eliminare
 */
void DeleteQueue(Queue *q)
{
  while(q->head != q->last)
  {
      Node *tmp = q->head;
      q->head = q->head->next;
      free(tmp);
  }
  if(q->head != NULL) free(q->head);
}

/**
 * @function Push
 * @brief inserisce fd in coda
 *
 * @param q coda in cui inserire fd
 * @param connfd fd da inserire in coda
 */
void Push(Queue *q,long c_fd)
{
  pthread_mutex_lock(&mux);
  Node * new = malloc(sizeof(Node));
  new -> connfd = c_fd;
  new -> next = NULL;
  if(q->head == NULL)
  {
    q->head = new;
    q->last = new;
  }
  else
  {
    q->last -> next = new;
    q->last = new;
  }
  if (c_fd == -1)
  {
    /* -- arriva info protocollo terminazione
          riattivo tutti i thread worker per
          farli morire -- */
    pthread_cond_broadcast(&cond);
  }
  else
  {
    /* -- risveglio solo un thread worker
          per fargli eseguire la richiesta -- */
    pthread_cond_signal(&cond);
  }
  pthread_mutex_unlock(&mux);
}

/**
 * @function Pop
 * @brief estrae primo elemento della coda
 *
 * @param q coda da cui estrarre fd
 *
 * @returns fd estratto
 */
long Pop(Queue *q){
  pthread_mutex_lock(&mux);
  while(q->head == NULL)
  {
    /* -- metto in wait i thread se la coda è vuota -- */
    pthread_cond_wait(&cond,&mux);
  }
  long ret = q->head->connfd;
  /* -- controllo se long da restituire è diverso da -1
        ovvero il carattere di terminazione -- */
  if( ret != -1)
  {
    Node *tmp = q->head;
    q->head = q->head -> next;
    free(tmp);
  }
  pthread_mutex_unlock(&mux);
  return ret;
}
