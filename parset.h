/*
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */

#if !defined(PARSET_H)
#define PARSET_H
/**
 * @file parset.h
 * @brief File contiene funzioni usate in parset.c
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */


#if!defined(SYSCALL)

#define SYSCALL(a,b,c) \
    if((a=b)==-1) { perror(c);exit(errno); }
#endif

// ipotizzo che il file di config abbia al più 100 righe
#define CONFIG_LINE_BUFFER_SIZE 100

// ipotizzo che le varaibili di config abbiano al più lughezza 15
#define MAX_CONFIG_VARIABLE_LEN 15


/**
 * @struct Config
 * @brief contiene variabili tratte da file di configurazione
 *
 * @var UnixPath stringa contenente path utilizzato per la creazione del socket AF_UNIX
 * @var DirName stringa contenente nome directory dove memorizzare i files da inviare agli utenti
 * @var StatFileName stringa contenente nome file nel quale verranno scritte le statistiche del server
 * @var MaxConnections intero contenente numero massimo di connessioni pendenti
 * @var ThreadsInPool intero contenente numero di thread nel pool
 * @var MaxMsgSize intero contenente dimensione massima di un messaggio testuale (numero di caratteri)
 * @var MaxFileSize intero contenente dimensione massima di un file accettato dal server (kilobytes)
 * @var MaxHistMsgs intero contenente numero massimo di messaggi che il server 'ricorda' per ogni
 */
typedef struct config_struct{

	char UnixPath[26],DirName[19],StatFileName[29];
	int MaxConnections,ThreadsInPool,MaxMsgSize,MaxFileSize,MaxHistMsgs;

}Config;

/**
 * @function read_str_from_config_line
 * @brief legge una stringa da una riga
 *
 * @param config_line riga da cui leggo la stringa
 * @param val stringa in cui salvo caratteri letti
 */
void read_str_from_config_line(char* config_line, char* val);

/**
 * @function read_int_from_config_lin
 * @brief legge un intero da una riga
 *
 * @param config_line riga da cui leggo intero
 *
 * @return intero letto
 */
int read_int_from_config_line(char* config_line);

/**
 * @function read_config_file
 * @brief legge le righe non commentate nel file di configurazione
 *		 e da esse estrae le stringhe e gli interi richiesti
 *
 * @param config_filename nome del file da cui leggere righe
 * @param config struttura da riempire leggendo file di configurazione
 */
void read_config_file(char* config_filename, Config* config);

#endif /* PARSET_H */
