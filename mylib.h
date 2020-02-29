/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
#if !defined(MYLIB_H_)
#define MYLIB_H_
/**
 * @file  mylib.h
 * @brief Contiene librerie delle funzioni implementate in chatty.c
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stats.h>
#include <signal.h>
#include <sys/types.h>
#include <message.h>
#if !defined(SYSCALL)
#define SYSCALL(a,b,c) \
    if((a=b)==-1) { perror(c);exit(errno); }
#endif //SYSCALL




/**
 * @function Registra
 * @brief svolge operazione di registrazione utente
 *
 * @param msg messaggio inviato dall'utente che si vuole registrare
 * @param fd fd dell'utente che si vuole registrare
 */
void Registra(message_t msg,long fd);

/**
 * @function Connetti
 * @brief svolge operazione di connessione utente
 *
 * @param msg messaggio inviato dall'utente che si vuole connettere
 * @param fd fd dell'utente che si vuole connettere
 *
 * @returns 1 se utente connesso con successo -1 altrimenti
 */
int Connetti(message_t msg,long fd);

/**
 * @function InviaTxtToOne
 * @brief invia messaggio ad un altro utente registrato
 *
 * @param msg contiene info messaggio da inviare
 * @param fd fd dell'utente che invia
 */
void InviaTxtToOne(message_t msg,long fd);

/**
 * @function InviaTxtToAll
 * @brief invia messaggio a tutti gli utenti registrati
 *
 * @param msg contiene info messaggio da inviare
 * @param fd fd dell'utente che invia
 */
void InviaTxtToAll(message_t msg, long fd);

/**
 * @function InviaFileToOne
 * @brief invia file ad un altro utente registrato
 *
 * @param msg contiene info del file da inviare
 * @param fd fd dell'utente che invia
 */
void InviaFileToOne(message_t msg,long fd);

/**
 * @function DownloadFile
 * @brief svolge richiesta download file
 *
 * @param msg contiene info del file da scaricare
 * @param fd fd dell'utente che richiede DownloadFile
 */
void DownloadFile(message_t msg, long fd);

/**
 * @function InvioUserList
 * @brief invia lista utenti registrati
 *
 * @param msg contiene info per inviare lista utenti registrati a utente sender
 * @param fd fd dell'utente che richiede UserList degli utenti registrati
 */
void InvioUserList(message_t msg, long fd);

/**
 * @function DownloadHistoryMSG
 * @brief scarica l'history dell'utente sender
 *
 * @param msg contiene info per scaricare history
 * @param fd fd dell'utente che richiede download history
 */
void DownloadHistoryMSG(message_t msg,long fd);

/**
 * @function Deregistra
 * @brief elimina da hash utente sender
 *
 * @param msg contiene indo utente da deregistrare
 * @fd fd dell'utente che richiede deregistrazione
 */
void Deregistra(message_t msg,long fd);

/**
 * @function readn
 * @brief usata per leggere buffer
 *
 * @param fd fd da cui leggere buffer
 * @param buf buffer da leggere
 * @param size dimensione del buffer da leggere
 *
 * @returns size se lettura avvenuta con successo di tutti i byte richiesti
 *          -1 se durante la lettura c'pe stato un errore
 *          0 se si è arrivati alla fine del file
 */
static inline int readn(long fd,void * buf,size_t size){
	size_t left=size;
	int r;
	char * bufptr = (char*)buf;
	while(left>0){
		if((r=read((int)fd,bufptr,left))==-1){
			if(errno==EINTR)continue;
			return -1;
		}
		if (r == 0)return 0;
		left -=r;
		bufptr+=r;
	}
	return size;
}

/**
 * @function writen
 * @brief usata per scrivere buffer
 *
 * @param fd fd in cui scrivere buffer
 * @param buf buffer da scrivere
 * @param size dimensione del buffer da scrivere
 *
 * @returns size se scrittura avvenuta con successo di tutti i byte richiesti
 *          -1 se durante la scrittura c'pe stato un errore
 *          0 se si è arrivati alla fine del file
 */
static inline int writen(long fd,void * buf,size_t size){
	size_t left=size;
	int r;
	char * bufptr = (char*)buf;
	while(left>0){
		if((r=write((int)fd,bufptr,left))==-1){
			if(errno==EINTR)continue;
			return -1;
		}
		if (r == 0)return 0;
		left -=r;
		bufptr+=r;
	}
	return 1;
}
#endif /*MYLIB_H_*/
