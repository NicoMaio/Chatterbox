/**
 * @file  stats.c
 * @brief Contiene implementazione funzioni dichiarate in stats.h
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stats.h>
#include <pthread.h>

/* -- variabile di mutua esclusione per accedere alle stats -- */
static pthread_mutex_t stats = PTHREAD_MUTEX_INITIALIZER;

/**
 * @function readNonline
 * @brief legge in mutua esclusione nonline
 *
 * @returns nonline
 */
int readNonline(){
	 extern struct statistics chattyStats;
	 long val;
	 /* -- prendo mutua esclusione per leggere stats -- */
	 pthread_mutex_lock(&stats);
	 val = chattyStats.nonline;
	 pthread_mutex_unlock(&stats);
	 return (int)val;
}

/**
 * @function readNusers
 * @brief legge in mutua esclusione nusers
 *
 * @returns nusers
 */
int readNusers(){
	extern struct statistics chattyStats;
	long val;
	/* -- prendo mutua esclusione per leggere stats -- */
	pthread_mutex_lock(&stats);
	val = chattyStats.nusers;
	pthread_mutex_unlock(&stats);
	return (int)val;
}

/**
 * @function aggiornaStats
 * @brief aggiorna in mutua esclusione tutti i puntatori delle stats
 *
 * @param users nuovo n. di utenti registrati
 * @param online nuovo n. di utenti connessi
 * @param mdelivered nuovo n. di messaggi testuali consegnati
 * @param mnotdelivered nuovo n. di messaggi testuali non ancora consegnati
 * @param fdelivered nuovo n. di file consegnati
 * @param fnotdelivered nuovo n. di file non ancora consegnati
 * @param errors nuovo n. di messaggi di Errore
 */
void aggiornaStats(int users, int online, int mdelivered, int mnotdelivered, int fdelivered, int fnotdelivered, int errors){
  extern struct statistics chattyStats;
  /* -- aggiorno in mutua esclusione tutti i contatori -- */
  pthread_mutex_lock(&stats);
  chattyStats.nusers += users;
  chattyStats.nonline += online;
  chattyStats.ndelivered += mdelivered;
  chattyStats.nnotdelivered += mnotdelivered;
  chattyStats.nfiledelivered += fdelivered;
  chattyStats.nfilenotdelivered += fnotdelivered;
  chattyStats.nerrors += errors;
  pthread_mutex_unlock(&stats);
}
