/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
#if !defined(QUEUE_H)
#define QUEUE_H

/**
 * @file  queue.h
 * @brief Contiene librerie delle funzioni implementate in queue.c
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

/**
 * @struct Node
 * @brief contiene info di un nodo della coda
 *
 * @var connfd fd del client che ha fatto richiesta
 * @var next puntatore a elemento successivo in coda
 */
typedef struct Node
{
  long connfd;
  struct Node * next;
}Node;

/**
 * @struct Queue_t
 * @brief contiene info della coda
 *
 * @var head puntatore alla testa della coda
 * @var last puntatore all'ultimo elemento della coda
 */
typedef struct Queue_t{
  Node *head;
  Node *last;
}Queue;

/**
 * @function initQueue
 * @brief inizializza la coda
 *
 * @returns una queue inizializzata
 */
Queue *initQueue();

/**
 * @function DeleteQueue
 * @brief elimina coda
 *
 * @param q puntatore a coda da eliminare
 */
void DeleteQueue(Queue *q);

/**
 * @function Push
 * @brief inserisce fd in coda
 *
 * @param q coda in cui inserire fd
 * @param connfd fd da inserire in coda
 */
void Push(Queue *q,long connfd);

/**
 * @function Pop
 * @brief estrae primo elemento della coda
 *
 * @param q coda da cui estrarre fd
 *
 * @returns fd estratto
 */
long Pop(Queue *q);
#endif
