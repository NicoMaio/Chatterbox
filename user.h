/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */


#if!defined(USER_H)
#define USER_H
/**
 * @file  user.h
 * @brief Contiene librerie delle funzioni implementate in user.c
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <message.h>
#include <pthread.h>
/* -- Libreria contenente funzioni per user_list (lista utenti online)-- */

/**
 * @struct utente
 * @brief contiene info utente online
 *
 * @var nome nome dell'utente online
 * @var fd fd dell'utente online
 * @var fd_on variabile di mutua esclusione per fd dell'utente online
 */
typedef struct utente{
	char *nome;
	long fd;
	pthread_mutex_t fd_on;
}user;

/**
 * @function create
 * @brief Crea un array di user* inizializzati a NULL
 *
 * @param dim dimensione della user_list
 *
 * @return puntatore ad array di user** se dim>0
 *         NULL in caso di dim<=0
 */
user **create(int dim);

/**
 * @function InvioHeaderToUserNOnline
 * @brief invia header a utente online prendendo mutex su user_list e fd_on
 *
 * @param utente_invia puntatore ad utente online a cui inviare header
 * @param fd indirizzo a cui inviare risposta
 * @param risp contiene info per invio header
 */
void InvioHeaderToUserNOnline(user*utente_invia,long fd,message_t risp);

/**
 * @function InvioHeaderToUserNOnline_2
 * @brief invia header a utente online prendendo mutex su user_list
 *
 * @param fd indirizzo a cui inviare risposta
 * @param risp contiene info per invio header
 */
void InvioHeaderToUserNOnline_2(long fd,message_t risp);

/**
 * @function InvioToUserOnline
 * @brief invia header e data a utente online prendendo mutex su user_list e fd_on
 *
 * @param utente_o puntatore ad utente online a cui inviare header e data
 * @param fd indiririzzo a cui inviare risposta
 * @param risp contiene info per invio header e data
 */
void InvioToUserOnline(user *utente_o,long fd,message_t risp);

/**
 * @function InvioToUserOnline_2
 * @brief invia header e data a utente online prendendo mutex su user_list
 *
 * @param utente_o puntatore ad utente online a cui inviare header e data
 * @param fd indirizzo a cui inviare risposta
 * @param risp contiene info per invio header e data
 */
void InvioToUserOnline_2(user *utente_o,long fd,message_t risp);

/**
 * @function find_user
 * @brief cerca utente in user_list
 *
 * @param user_list : array di struct in cui cercare
 * @param name : stringa da cercare in user_list ovvero in array di struct
 * @param dim : dimensione array di struct user_list
 *
 * @return utente richiseto se utente name presente in user_list
 *         NULL in caso contrario
 */
user* find_user(user **user_list,char*name,int dim);

/**
 * @function find_user_byfd
 * @brief cerca utente in user_list tramite descrittore connessione utente
 *
 * @param user_list : array di struct in cui cercare
 * @param fd : intero da cercare in user_list ovvero in array di struct
 * @param dim : dimensione array di struct user_list
 *
 * @return utente richiseto se utente name presente in user_list
 *         NULL in caso contrario
 */
int find_user_byfd(user **user_list,long fd,int dim);

/**
 * @function add_user
 * @brief Aggiunge name a user_list
 *
 * @param dim : dimensione della user_list
 * @param name : nome utente da aggiungere a user_list
 * @param user_list : array di stringhe in cui aggiungere name
 * @oaram fd : descrittore connessione di utente da aggiungere a user_list
 *
 * @return 1 inserimento avvenuto con successo
 *         0 se name gia presente, -1 se user_list piena
 */
int add_user(user **user_list,char *name,int dim,long fd);

/**
 * @function rem_user
 * @brief elimina utente name da user_list
 *
 * @param dim : dimensione della user_list
 * @param user_list : array di struct da cui rimuovere name
 * @param fd : indirizzo della connessione dell'utente che si e` appena disconnesso
 *
 * @return 1 se rimozione avvenuta con sucesso
 *		   0 se name non presente in user_list
 *
 */
int rem_user_byfd(user** user_list,long fd,int dim);

/**
 * @function destroy_user
 * @brief Svuota user_list
 *
 * @param dim : dimensione della user_list
 * @param user_list : puntatore all'array di stringhe da eliminare
 *
 *
 */
void destroy_user(user **user_list,int dim);

/**
 * @function Invio_Lista_Utenti_Online
 * @brief invia lista utenti online
 *
 * @param user_list puntatore a lista utenti online
 * @param utente_o puntatore ad utente online a cui inviare la user_list
 * @param fd fd dell'utente a cui si invia la user_list
 * @param nonline numero utenti nonline
 * @param maxcon numero di connessioni contemporanee massimo
 * @param risp messaggio da inviare all'utente
 * @param msg contiene info per inviare messaggio all'utente
 */
void Invio_Lista_Utenti_Online(user** user_list,user*utente_o,long fd,int nonline,int maxcon,message_t risp,message_t msg);

/**
 * @function Invio_Lista_Utenti_Online_2
 * @brief invia lista utenti online in mutua esclusione su user_list
 *
 * @param user_list puntatore a lista utenti online
 * @param utente_o puntatore ad utente online a cui inviare la user_list
 * @param fd fd dell'utente a cui si invia la user_list
 * @param nonline numero utenti nonline
 * @param maxcon numero di connessioni contemporanee massimo
 * @param risp messaggio da inviare all'utente
 * @param msg contiene info per inviare messaggio all'utente
 */
void Invio_Lista_Utenti_Online_2(user** user_list,user*utente_o,long fd,int nonline,int maxcon,message_t risp,message_t msg);

#endif
