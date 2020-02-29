/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */

#if !defined(MEMBOX_STATS_)
#define MEMBOX_STATS_

/**
 * @file  stats.h
 * @brief Contiene librerie delle funzioni implementate in stats.c
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stdio.h>
#include <time.h>
#include <pthread.h>

/**
 * @struct statistics
 * @brief contiene contatori statistiche
 *
 * @var nusers n. di utenti registrati
 * @var nonline n. di utenti connessi
 * @var ndelivered n. di messaggi testuali consegnati
 * @var nnotdelivered n. di messaggi testuali non ancora consegnati
 * @var nfiledelivered n. di file consegnati
 * @var nfilenotdelivered n. di file non ancora consegnati
 * @var nerrors n. di messaggi di Errore
 */
struct statistics {
    unsigned long nusers;                       // n. di utenti registrati
    unsigned long nonline;                      // n. di utenti connessi
    unsigned long ndelivered;                   // n. di messaggi testuali consegnati
    unsigned long nnotdelivered;                // n. di messaggi testuali non ancora consegnati
    unsigned long nfiledelivered;               // n. di file consegnati
    unsigned long nfilenotdelivered;            // n. di file non ancora consegnati
    unsigned long nerrors;                      // n. di messaggi di errore
};

/* aggiungere qui altre funzioni di utilita' per le statistiche */

/**
 * @function readNonline
 * @brief legge in mutua esclusione nonline
 *
 * @returns nonline
 */
int readNonline();

/**
 * @function readNusers
 * @brief legge in mutua esclusione nusers
 *
 * @returns nusers
 */
int readNusers();

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
void aggiornaStats(int users, int online, int mdelivered, int mnotdelivered, int fdelivered, int fnotdelivered, int errors);

/**
 * @function printStats
 * @brief Stampa le statistiche nel file passato come argomento
 *
 * @param fout descrittore del file aperto in append.
 *
 * @return 0 in caso di successo, -1 in caso di fallimento
 */
static inline int printStats(FILE *fout) {
    extern struct statistics chattyStats;

    if (fprintf(fout, "%ld - %ld %ld %ld %ld %ld %ld %ld\n",
		(unsigned long)time(NULL),
		chattyStats.nusers,
		chattyStats.nonline,
		chattyStats.ndelivered,
		chattyStats.nnotdelivered,
		chattyStats.nfiledelivered,
		chattyStats.nfilenotdelivered,
		chattyStats.nerrors
		) < 0) return -1;
    fflush(fout);
    return 0;
}

#endif /* MEMBOX_STATS_ */
