/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file  user.c
 * @brief Contiene implementazione funzioni dichiarate in user.h
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <pthread.h>
#include <user.h>
#include <message.h>
#include <connections.h>
#include <mylib.h>

/* -- variabile di muta esclusione per gestire user_list degli utenti online -- */
static pthread_mutex_t user_mtx = PTHREAD_MUTEX_INITIALIZER;

/**
 * @function create
 * @brief Crea un array di user* inizializzati a NULL
 *
 * @param dim dimensione della user_list
 *
 * @return puntatore ad array di user** se dim>0
 *         NULL in caso di dim<=0
 */
user **create(int dim)
{
	if(dim >0)
	{
		user **user_list = malloc(sizeof(user*)*dim);
		for(int i = 0;i < dim ; i++)
		{
			user_list[i]=NULL;
		}
		return user_list;
	}
	else
	{
		return NULL;
	}
}

/**
 * @function find_user
 * @brief cerca utente in user_list tramite nome utente
 *
 * @param user_list : array di struct in cui cercare
 * @param name : stringa da cercare in user_list ovvero in array di struct
 * @param dim : dimensione array di struct user_list
 *
 * @return utente richiseto se utente name presente in user_list
 *         NULL in caso contrario
 */
user* find_user(user **user_list,char*name,int dim)
{
	int trovato = 0;
	user* curr;
	/* -- prendo mutua esclusione per accedere a user_list -- */
	pthread_mutex_lock(&user_mtx);
	for(int i =0; i<dim && trovato ==0;i++)
	{
		if(user_list[i]!=NULL)
		{
			if(strcmp(name,user_list[i]->nome)==0)
			{
				curr = user_list[i];
				trovato = 1;
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	pthread_mutex_unlock(&user_mtx);
	if(trovato)
	{
		return curr;
	}
	else
	{
		return NULL;
	}
}

/**
 * @function find_user_byfd
 * @brief cerca utente in user_list tramite descrittore connessione utente
 *
 * @param user_list : array di struct in cui cercare
 * @param fd : long da cercare in user_list ovvero in array di struct
 * @param dim : dimensione array di struct user_list
 *
 * @return utente richiseto se utente name presente in user_list
 *         NULL in caso contrario
 */
int find_user_byfd(user **user_list,long fd,int dim)
{
	int trovato = 0;
	/* -- prendo mutua esclusione per accedere a user_list -- */
	pthread_mutex_lock(&user_mtx);
	for(int i =0; i<dim && trovato ==0;i++)
	{
		if(user_list[i]!=NULL)
		{
			if(user_list[i]->fd == fd)
			{
				trovato = 1;
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	pthread_mutex_unlock(&user_mtx);
	return trovato;
}

/**
 * @function add_user
 * @brief Aggiunge name a user_list
 *
 * @param dim : dimensione della user_list
 * @param name : nome utente da aggiungere a user_list
 * @param user_list : array di stringhe in cui aggiungere name
 *
 * @return 1 inserimento avvenuto con successo
 *         0 se name gia presente, -1 se user_list piena
 */
int add_user(user **user_list,char *name,int dim,long fd)
{
	if(find_user(user_list,name,dim)!=NULL)
	{
		return 0;
	}
	int trovato = 0;
	user*new = malloc(sizeof(user));
	memset(new,0,sizeof(user));
	memset(&(new->fd_on),0,sizeof(pthread_mutex_t));
	new -> fd_on = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	memset(&(new->fd),0,sizeof(long));
	new->fd = fd;
	new->nome = malloc(sizeof(char)*(MAX_NAME_LENGTH+1));
	strncpy(new->nome,name,(MAX_NAME_LENGTH+1));
	/* -- prendo mutua esclusione per accedere a user_list -- */
	pthread_mutex_lock(&user_mtx);
	for(int i =0; i<dim && trovato==0; i++)
	{
		if(user_list[i]== NULL)
		{
			trovato = 1;
			user_list[i] = new;
		}
		else
		{
			continue;
		}
	}
	pthread_mutex_unlock(&user_mtx);
	if(trovato == 0) return -1;
	else return 1;
}

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
int rem_user_byfd(user** user_list,long fd,int dim)
{
	if(find_user_byfd(user_list,fd,dim) ==0 ){
		return 0;
	}else{
		/* -- prendo mutua esclusione per accedere a user_list -- */

		pthread_mutex_lock(&user_mtx);
		for(int i = 0;i<dim; i++)
		{
			if(user_list[i]!=NULL)
			{
				if(user_list[i]->fd == fd)
				{
					free(user_list[i]->nome);
					free(user_list[i]);
					user_list[i]=NULL;
				}
				else
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}
		pthread_mutex_unlock(&user_mtx);
	}
	return 1;
}

/**
 * @function destroy_user
 * @brief Svuota user_list
 *
 * @param dim : dimensione della user_list
 * @param user_list : puntatore all'array di stringhe da eliminare
 *
 */
void destroy_user(user **user_list,int dim)
{
	/* -- prendo mutua esclusione per accedere a user_list -- */
	pthread_mutex_lock(&user_mtx);
	for(int i =0;i<dim;i++)
	{
		if(user_list[i]!=NULL)
		{
			free(user_list[i]->nome);
			free(user_list[i]);
		}
		else
		{
			continue;
		}
	}
	free(user_list);
	pthread_mutex_unlock(&user_mtx);
}

/**
 * @function InvioToUserOnline
 * @brief invia header e data a utente online prendendo mutex su user_list e fd_on
 *
 * @param utente_o : puntatore ad utente online a cui inviare header e data
 * @param fd : indirizzo a cui inviare risposta
 * @param risp : contiene info per invio header e data
 */
void InvioToUserOnline(user *utente_o,long fd,message_t risp)
{
	/* -- prendo mutua esclusione per su user_list per leggere da un user online -- */
	pthread_mutex_lock(&user_mtx);
	/* -- prendo mutua esclusione su fd_on per inviare rispsota a utente online -- */
	//pthread_mutex_lock(&utente_o->fd_on);
	sendHeader(fd,&risp.hdr);
	sendData(fd,&risp.data);
	//pthread_mutex_unlock(&utente_o->fd_on);
	pthread_mutex_unlock(&user_mtx);
}

/**
 * @function InvioToUserOnline_2
 * @brief invia header e data a utente online prendendo mutex su user_list
 *
 * @param utente_o : puntatore ad utente online a cui inviare header e data
 * @param fd : indirizzo a cui inviare risposta
 * @param risp : contiene info per invio header e data
 */
void InvioToUserOnline_2(user *utente_o,long fd,message_t risp)
{
	/* -- prendo mutua esclusione per su user_list per inviare messaggio ad un user online -- */
	pthread_mutex_lock(&user_mtx);
	sendHeader(fd,&risp.hdr);
	sendData(fd,&risp.data);
	pthread_mutex_unlock(&user_mtx);
}

/**
 * @function InvioHeaderToUserNOnline
 * @brief invia header a utente online prendendo mutex su user_list e fd_on
 *
 * @param utente_invia : puntatore ad utente online a cui inviare header
 * @param fd : indirizzo a cui inviare risposta
 * @param risp : contiene info per invio header
 */
void InvioHeaderToUserNOnline(user*utente_invia,long fd,message_t risp)
{
	/* -- prendo mutua esclusione per su user_list per leggere da un user online -- */
	pthread_mutex_lock(&user_mtx);
	/* -- prendo mutua esclusione su fd_on per inviare rispsota a utente online -- */
 	//pthread_mutex_lock(&utente_invia->fd_on);
	sendHeader(fd,&risp.hdr);
	//pthread_mutex_unlock(&utente_invia->fd_on);
	pthread_mutex_unlock(&user_mtx);
}

/**
 * @function InvioHeaderToUserNOnline_2
 * @brief invia header a utente online prendendo mutex su user_list
 *
 * @param fd : indirizzo a cui inviare risposta
 * @param risp : contiene info per invio header
 */
void InvioHeaderToUserNOnline_2(long fd,message_t risp)
{
	/* -- prendo mutua esclusione per su user_list per inviare messaggio ad un user online -- */
	pthread_mutex_lock(&user_mtx);
	sendHeader(fd,&risp.hdr);
	pthread_mutex_unlock(&user_mtx);
}

/**
 * @function Invio_Lista_Utenti_Online
 * @brief invia lista utenti online in mutua esclusione sia su user_list che su fd_on
 *
 * @param user_list : puntatore a lista utenti online
 * @param utente_o : puntatore ad utente online a cui inviare la user_list
 * @param fd : fd dell'utente a cui si invia la user_list
 * @param nonline : numero utenti nonline
 * @param maxcon : numero di connessioni contemporanee massimo
 * @param risp : messaggio da inviare all'utente
 * @param msg : contiene info per inviare messaggio all'utente
 */
void Invio_Lista_Utenti_Online(user** user_list,user* utente_o,long fd,int nonline,int maxcon,message_t risp,message_t msg)
{
		/* -- prendo mutua esclusione per accedere a user_list -- */
		pthread_mutex_lock(&user_mtx);
		char *ok = malloc(sizeof(char)*(MAX_NAME_LENGTH+1)*nonline);
		memset(ok,0,(MAX_NAME_LENGTH+1)*nonline);
		int dim = nonline * (MAX_NAME_LENGTH+1);
		int indok = 0;
		int conta_u=0;
		for(int i=0;i<maxcon && conta_u<nonline;i++)
		{
			if(user_list[i]!=NULL && user_list[i]->nome!= NULL && strcmp(user_list[i]->nome,"")!=0 && strcmp(user_list[i]->nome," ")!=0)
			{
				strncpy(&ok[indok],user_list[i]->nome,strlen(user_list[i]->nome) + 1);
				indok+=(MAX_NAME_LENGTH+1);
				conta_u++;
			}
		}
		/* -- prendo mutua esclusione su fd_on per inviare rispsota a utente online -- */
		//pthread_mutex_lock(&utente_o->fd_on);
		if(fd>0){
			sendHeader(fd,&risp.hdr);
			writen(fd, msg.data.hdr.receiver, sizeof(char)*(MAX_NAME_LENGTH +1));
			write (fd, &dim, sizeof(int));
			for(int i =0;i<nonline*(MAX_NAME_LENGTH+1);i+=(MAX_NAME_LENGTH+1))
			{
				writen(fd,&ok[i],(MAX_NAME_LENGTH+1));
			}
		}
		//pthread_mutex_unlock(&utente_o->fd_on);
		free(ok);
		pthread_mutex_unlock(&user_mtx);
}

/**
 * @function Invio_Lista_Utenti_Online_2
 * @brief invia lista utenti online in mutua esclusione su user_list
 *
 * @param user_list : puntatore a lista utenti online
 * @param utente_o : puntatore ad utente online a cui inviare la user_list
 * @param fd : fd dell'utente a cui si invia la user_list
 * @param nonline : numero utenti nonline
 * @param maxcon : numero di connessioni contemporanee massimo
 * @param risp : messaggio da inviare all'utente
 * @param msg : contiene info per inviare messaggio all'utente
 */
void Invio_Lista_Utenti_Online_2(user** user_list,user* utente_o,long fd,int nonline,int maxcon,message_t risp,message_t msg)
{
		/* -- prendo mutua esclusione per accedere a user_list -- */
		pthread_mutex_lock(&user_mtx);
		char *ok = malloc(sizeof(char)*(MAX_NAME_LENGTH+1)*nonline);
		memset(ok,0,(MAX_NAME_LENGTH+1)*nonline);
		int dim = nonline * (MAX_NAME_LENGTH+1);
		int indok = 0;
		int conta_u=0;
		for(int i=0;i<maxcon && conta_u<nonline;i++)
		{
			if(user_list[i]!=NULL && user_list[i]->nome!= NULL && strcmp(user_list[i]->nome,"")!=0 && strcmp(user_list[i]->nome," ")!=0)
			{
				strncpy(&ok[indok],user_list[i]->nome,strlen(user_list[i]->nome) + 1);
				indok+=(MAX_NAME_LENGTH+1);
				conta_u++;
			}
		}
		if(fd>0){
			sendHeader(fd,&risp.hdr);
			writen(fd, msg.data.hdr.receiver, sizeof(char)*(MAX_NAME_LENGTH +1));
			write (fd, &dim, sizeof(int));
			for(int i =0;i<nonline*(MAX_NAME_LENGTH+1);i+=(MAX_NAME_LENGTH+1))
			{
				writen(fd,&ok[i],(MAX_NAME_LENGTH+1));
			}
		}
		free(ok);
		pthread_mutex_unlock(&user_mtx);
}
