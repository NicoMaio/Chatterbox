/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file  history.c
 * @brief Contiene implementazione funzioni dichiarate in history.h
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <history.h>

/**
 * @function initHist
 * @brief inizializza history
 *
 * @param dim_hist dimensione della history
 * @param mtx variabile di mutex della history
 *
 * @returns history inizializzata
 */
storia * initHist(int dim_hist,pthread_mutex_t mtx)
{
	storia * s;
	s = malloc(sizeof(storia));
	memset(s,0,sizeof(storia));
	s->testa = 0;
	s->fine = dim_hist;
	s->messaggi =(message_t**) malloc(sizeof(message_t*)*dim_hist);
	memset(s->messaggi,0,sizeof(message_t*));
	s->msg_in_hist = 0;
	for(int i =0;i<dim_hist;i++)
	{
		s->messaggi[i] = NULL;
	}
	return s;
}

/**
 * @function add
 * @brief aggiunge messaggi alla history
 *
 * @param s history in cui aggiungere msg
 * @param msg messaggio da aggiungere
 * @param mtx variabile di mutex della history
 * @param dim_hist dimensione della history
 */
void add( storia *s,message_t *msg,pthread_mutex_t mtx,int dim_hist)
{
	/* -- prendo mutua esclusione su history -- */
	pthread_mutex_lock(&mtx);
	if(s->testa == 0)
	{
		int i =s->testa;
		int trovato = 0;
		while(i<s->fine && trovato == 0)
		{
			if(s->messaggi[i]==NULL)
			{
				trovato = 1;
			}
			else
			{
				i++;
			}
		}
		if (!trovato)
		{
			memset((char*)s->messaggi[s->testa],0,sizeof(message_t));
			s->messaggi[s->testa] = malloc(sizeof(message_t));
			memset((char*)s->messaggi[s->testa],0,sizeof(message_t));
			setHeader(&s->messaggi[s->testa]->hdr,msg->hdr.op,msg->hdr.sender);
			setData(&s->messaggi[s->testa]->data,msg->data.hdr.receiver,msg->data.buf,msg->data.hdr.len);
			s->fine = s->testa;
			s->testa= (s->testa +1)%dim_hist;
			if(s->msg_in_hist < dim_hist) s->msg_in_hist += 1;
		}
		else
		{
			s->messaggi[i] = malloc(sizeof(message_t));
			memset((char*)s->messaggi[i],0,sizeof(message_t));
			setHeader(&s->messaggi[i]->hdr,msg->hdr.op,msg->hdr.sender);
			setData(&s->messaggi[i]->data,msg->data.hdr.receiver,msg->data.buf,msg->data.hdr.len);
			if(s->msg_in_hist < dim_hist) s->msg_in_hist += 1;
		}
	}
	else
	{
		memset((char*)s->messaggi[s->testa],0,sizeof(message_t));
		s->messaggi[s->testa] = malloc(sizeof(message_t));
		memset((char*)s->messaggi[s->testa],0,sizeof(message_t));
		setHeader(&s->messaggi[s->testa]->hdr,msg->hdr.op,msg->hdr.sender);
		setData(&s->messaggi[s->testa]->data,msg->data.hdr.receiver,msg->data.buf,msg->data.hdr.len);
		s->fine = s->testa;
		s->testa= (s->testa +1)%dim_hist;
		if(s->msg_in_hist < dim_hist) s->msg_in_hist += 1;
	}
	pthread_mutex_unlock(&mtx);
}

/**
 * @function readConta
 * @brief conta il numero di messaggi presenti in history
 *
 * @param s history in cui conta i messaggi presenti
 * @param mtx variabile di mutex della history
 *
 * @returns numero dei messaggi presenti in history
 */
int readConta(storia *s,pthread_mutex_t mtx)
{
	int val;
	/* -- prendo mutua esclusione su history -- */
	pthread_mutex_lock(&mtx);
	val = s->msg_in_hist;
	pthread_mutex_unlock(&mtx);
	return val;
}

/**
 * @function destroyhist
 * @brief distrugge la history
 *
 * @param s history da distruggere
 * @param dim_hist dimensione della history da distruggere
 */
void destroyhist(storia *s,int dim_hist)
{
	if(s!=NULL)
	{
		for(int i =0;i<dim_hist;i++)
		{
			if(s->messaggi[i]!=NULL && s->messaggi[i]->data.buf!=NULL)
			{
						free(s->messaggi[i]->data.buf);
						free(s->messaggi[i]);
			}
		}
	}
	if(s->messaggi != NULL)free(s->messaggi);
	free(s);
}
