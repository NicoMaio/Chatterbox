/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file chatty.c
 * @brief File principale del server chatterbox
 *
 * @author Nicolo` Maio 544935
 *
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libgen.h>
/* inserire gli altri include che servono */
#include <stats.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <mylib.h>
#include <icl_hash.h>
#include <sup_icl.h>
#include <ops.h>
#include <message.h>
#include <connections.h>
#include <sys/signalfd.h>
#include <user.h>
#include <history.h>
#include <config.h>
#include <parset.h>
#include <queue.h>
#include <limits.h>


/**
 * struttura che memorizza le statistiche del server, struct statistics
 * e' definita in stats.h.
 *
 */
struct statistics chattyStats={0,0,0,0,0,0,0};

/* --------------- varibili globali --------------- */
static Config elenco;   // struttura delle info di configurazione vedi parset.c
static Queue* queue;    // coda delle richieste dal client in sospeso vedi queue.c
static icl_hash_t *h;   // tabella hash degli utenti registrati vedi icl_hash.c
static int pipe_t[2];   // pipe usata per comunicazione worker -> listener
static sigset_t mask;   // maschera per segnali di terminazione
static user**user_list; // user_list degli utenti online vedi user.c
/*-------------------------------------------------- */



/**
 * @struct user_in_hash
 * @brief utente registrato in hashed
 *
 * @var name : nome utente registrato
 * @var s : puntatore a history
 * @var dimhist : dimensione history
 * @var fd_u : fd utente registrato
 * @var mia : variabile di mutex per history
 */
typedef struct user{
	char *name;
	storia *s;
	int dimhist;
	long fd_u;
	pthread_mutex_t mia;
}u;

/**
 * @struct Signal-Handler
 * @brief contiene gli argomenti del sig-handler
 *
 * @var set : insieme degli argomenti
 */
typedef struct sigHandlerArgs {
    sigset_t *set;
} sigHandlerArgs_t;

/**
 * @function cleanup
 * @brief elimina il socket ovvero chatty_socket in tmp
 */
void cleanup();

/**
 * @function Segnali
 * @brief signal-handler thread gestisce segnale SIGUSR1
 *
 * @param arg : puntatore a argomenti del signal-handler thread
 */
static void * Segnali(void *arg);

/**
 * @function createUser
 * @brief crea utente da inserire in tab hash
 *
 * @param fd : indirizzo dell'utente da registrare
 * @param name : nome dell'utente da registrare
 *
 * @returns puntatore a struttura del nuovo utente
 */
u *createUser(long fd,char* name);

/**
 * @function deleteuser
 * @brief svuota il campo data di utente in hash
 *
 * @param arg : puntatore a componenti del campo data dell'utente in hash
 */
void deleteuser(void *arg);

/**
 * @function deleteuser2
 * @brief svuota il campo data di utente creato ma già presente in hash
 *
 * @param arg : puntatore a componenti del campo data dell'utente
 */
void deleteuser2(void *arg);

/**
 * @function deletekey
 * @brief svuota il campo key di utente in hash
 *
 * @param arg : puntatore a componenti del campo key dell'utente in hash
 */
void deletekey(void*arg);

/**
 * @function update_max
 * @brief funzione usata per aggiornare il massimo fd in set della select
 *
 * @param set : insieme degli fd su cui legge la select
 * @param max : valore di max fd attuale
 *
 * @returns nuovo fd max
 */
long update_max(fd_set *set,long max);

/**
 * @function Listening
 * @brief thread Listener che ascolta da tutti gli fd
 * 			 e intercetta anche segnali di terminazione
 *
 * @param arg : puntatore a argomenti del Listener
 */
static void *Listening(void *arg);

/**
 * @function create_copy_msg
 * @brief crea una copia del messaggio da inserire in history
 *
 * @param : message messaggio da copiare
 *
 * @returns copia del messaggio passato come parametro
 */
message_t create_copy_msg(message_t message);

/**
 * @function Working
 * @brief funzione eseguita dai thread worker in pool per gestire
 * 			 richieste dai vari client
 *
 * @param arg : puntatore agli argomenti del Worker
 */
static void *Working(void *arg);

/**
 * @function main
 * @brief funzione main del server chatty
 */
int main(int argc, char *argv[])
{
	/* -- Controllo input chatty -- */
	if(argc < 2)
	{
		fprintf(stderr, "Il server va lanciato con il seguente comando: chatty -f conffile\n");
		return -1;
	}

	/* -- Riempie la struct di configurazione vedi mylib.h e parset.c --*/
	read_config_file(argv[2],&elenco);

	/* -- Creazione Directory tmp che conterra' il socket e i file i cui path presenti in
		  chatty.conf1 e chatty.conf2 -- */
	struct stat st1;
	if(stat("./tmp",&st1) == -1)
	{
		mkdir("./tmp",0777);
	}
	if(stat(elenco.DirName,&st1)== -1)
	{
		mkdir(elenco.DirName,0777);
	}

	/* -- Dichiarazione varibili utili in main -- */
	int notused;

	/* -- Creo la pipe richiesta per comunicazione workers -> listener -- */
	SYSCALL(notused,pipe(pipe_t),"funzione pipe");

	/* -- Imposto gestione segnali -- */
	sigset_t maskT;
	sigemptyset(&maskT);
	sigaddset(&maskT,SIGPIPE);
	sigaddset(&maskT,SIGUSR1);
  	sigemptyset(&mask);
  	sigaddset(&mask, SIGINT);
 	sigaddset(&mask, SIGQUIT);
  	sigaddset(&mask, SIGTERM);
 	SYSCALL(notused,pthread_sigmask(SIG_BLOCK,&maskT,NULL),"Funzione Sigmask - 1");
 	SYSCALL(notused,pthread_sigmask(SIG_BLOCK, &mask,NULL),"Funzione Sigmask - 2");

	/* -- Allocazione e dichiarazione varie struct  -- */
	queue = initQueue();
	user_list = create(elenco.MaxConnections);
	h = icl_hash_create( hash_pjw, string_compare);

	/* -- Dichiarazione ed esecuzione thread -- */
	sigHandlerArgs_t sigArgs = {&maskT};
	pthread_t  *workers,signalthread, listener;
	workers = malloc(sizeof(pthread_t)*elenco.ThreadsInPool);
	SYSCALL(notused,pthread_create(&listener,NULL,Listening,NULL),"Creazione listener");
	SYSCALL(notused,pthread_create(&signalthread,NULL,Segnali,&sigArgs),"Creazione SignalThread");
	for(int i=0;i<elenco.ThreadsInPool;i++){
		SYSCALL(notused,pthread_create(&workers[i],NULL,Working,NULL),"Creazione Pool workers");
	}

	/*-- Protocollo di terminazione --*/
	SYSCALL(notused,pthread_join(listener,NULL),"Join listener");
	for(int i =0;i<elenco.ThreadsInPool;i++){
		SYSCALL(notused,pthread_join(workers[i],NULL),"Join workers");
	}
	SYSCALL(notused,pthread_cancel(signalthread),"pthread_cancel Thread dei segnali");
	SYSCALL(notused,pthread_join(signalthread,NULL),"pthread_join Thread dei segnali");
	free(workers);
	DeleteQueue(queue);
	cleanup();
	free(queue);
	icl_hash_destroy(h, deletekey , deleteuser);
	destroy_user(user_list,elenco.MaxConnections);
	/*-- Fine protocollo di terminazione --*/

  return 0;
}

/**
 * @function cleanup
 * @brief elimina il socket ovvero chatty_socket in tmp
 */
void cleanup(){
	int notused;
	SYSCALL(notused,unlink(elenco.UnixPath),"unlink socket");
}

/**
 * @function Segnali
 * @brief signal-handler thread gestisce segnale SIGUSR1
 *
 * @param arg : puntatore a argomenti del signal-handler thread
 */
static void * Segnali(void *arg){
	sigset_t *maskT= ((sigHandlerArgs_t*)arg)->set;
	int sig;
	for(;;){
		sigwait(maskT,&sig);
			if(sig ==SIGUSR1){
				/* -- apro file delle statistiche -- */
				FILE *fout = fopen(elenco.StatFileName,"w");
				/* -- scrivo statistiche -- */
				if (printStats(fout) == 0){
				}else{
					printf("\n -- Errore scrittura statistic -- \n");
				}
				/* -- chiudo il file -- */
				fclose(fout);
			}
	}
pthread_exit(NULL);
}

/**
 * @function createUser
 * @brief crea utente da inserire in tab hash
 *
 * @param fd : indirizzo dell'utente da registrare
 * @param name : nome dell'utente da registrare
 *
 * @returns puntatore a struttura del nuovo utente
 */
u *createUser(long fd,char* name){
	u *use=malloc(sizeof(u));
	memset(use,0,sizeof(u));
	use->dimhist = elenco.MaxHistMsgs;
	memset(&(use->fd_u),0,sizeof(long));
	use->fd_u = fd;
	use->mia =(pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	use->name = malloc(sizeof(char)*MAX_NAME_LENGTH);
	strcpy(use->name,name);
	return use;
}

/**
 * @function deleteuser
 * @brief svuota il campo data di utente in hash
 *
 * @param arg : puntatore a componenti del campo data dell'utente in hash
 */
void deleteuser(void *arg){
	u *use = (u*)arg;
	if(use->s)destroyhist(use->s,elenco.MaxHistMsgs);
	free(use);
}

/**
 * @function deleteuser2
 * @brief svuota il campo data di utente creato ma già presente in hash
 *
 * @param arg : puntatore a componenti del campo data dell'utente
 */
void deleteuser2(void *arg){
	u*use = (u*)arg;
	free(use->name);
	free(use);
}

/**
 * @function deletekey
 * @brief svuota il campo key di utente in hash
 *
 * @param arg : puntatore a componenti del campo key dell'utente in hash
 */
void deletekey(void*arg){
	if(arg != NULL){
		char *key =(char*)arg;
		free(key);
	}
}

/**
 * @function update_max
 * @brief funzione usata per aggiornare il massimo fd in set della select
 *
 * @param set : insieme degli fd su cui legge la select
 * @param max : valore di max fd attuale
 *
 * @returns nuovo fd max
 */
long update_max(fd_set *set,long max){
	long fd_max=0;
	for(long i =0;i<=max;i++){
		if(FD_ISSET(i,set)){
			if(i>fd_max)fd_max=i;
		}
	}
	return fd_max;
}

/**
 * @function Listening
 * @brief thread Listener che ascolta da tutti gli fd
 * 			 e intercetta anche segnali di terminazione
 *
 * @param arg : puntatore a argomenti del Listener
 */
static void *Listening(void *arg){
	/* -- imposto comunicazione con socket -- */
	int notused;
	char * ecco = malloc(sizeof(char)*(strlen(elenco.UnixPath)+1));
	strcpy(ecco,elenco.UnixPath);
	struct sockaddr_un sa;
	memset(&sa,0,sizeof(sa));
	strncpy(sa.sun_path,ecco,strlen(ecco)+1);
	sa.sun_family=AF_UNIX;
	int fd_sk;
	free(ecco);
	int max = elenco.MaxConnections;
	SYSCALL(fd_sk,socket(AF_UNIX,SOCK_STREAM,0),"Funzione socket");
	SYSCALL(notused,bind(fd_sk,(struct sockaddr *)&sa,sizeof(sa)),"Funzione bind");
	SYSCALL(notused,listen(fd_sk,max),"Funzione listen");
	int fdmax=0;
	fd_set set,tmpset;

	/* -- imposto signalfd per segnali di interruzione -- */
	int signal;
	SYSCALL(signal,signalfd(-1,&mask,0),"signalfd");

	FD_ZERO(&set);
	/* -- inserisco fd nel set da cui legge la select -- */
	FD_SET(fd_sk,&set);
	FD_SET(signal,&set);
	FD_SET(pipe_t[0],&set);

	if (fd_sk > fdmax) fdmax = fd_sk;


	struct timeval timer;
	while(1){
		/* -- imposto timer -- */

		timer.tv_sec=0;
		timer.tv_usec=1000;
		FD_ZERO(&tmpset);
		tmpset = set;
		SYSCALL(notused,select(fdmax+1,&tmpset,NULL,NULL,&timer),"Funzione Select");

		for(int i=0;i<=fdmax;i++){
			if(FD_ISSET(i,&tmpset))
			{
				if(i == signal && i != 0 && i>0)
				{
					/* -- ricevuto segnale di terminazione comincio protocollo -- */
					close(fd_sk);
					Push(queue,-1);
					pthread_exit(NULL);
				}
				else
				{
					if(i == fd_sk && i!=0 && i>0)
					{
						/* -- ricevuta richiesta da socket -- */
						int fd_c;
						fd_c=accept(fd_sk,NULL,0);
						if(fd_c >0){
							FD_SET(fd_c,&set);
							if(fd_c>fdmax)fdmax=fd_c;
						}
					}
					else
					{
						if(i== pipe_t[0] && i!=0 && i>0)
						{
							/* -- leggo dalla pipe per reinserire fd in set -- */
							int fd_p;
							SYSCALL(notused,read(pipe_t[0],&fd_p,sizeof(int)),"Aggiorna con pipe");
							if(fd_p != 0 && fd_p>0){
								FD_SET(fd_p,&set);
								if (fd_p > fdmax) fdmax=fd_p;
							}
						}
						else
						{
						  /* -- inserisco richiesta nel pool delle richieste -- */
							if(i>0){
								Push(queue,i);
								FD_CLR(i,&set);
								fdmax= update_max(&set,fdmax);
							}

						}
					}
				}
			}
		}
	}
}

/**
 * @function create_copy_msg
 * @brief crea una copia del messaggio da inserire in history
 *
 * @param : message messaggio da copiare
 *
 * @returns copia del messaggio passato come parametro
 */
message_t create_copy_msg(message_t message){
	/* -- creo la copia del messaggio da inserire in history -- */
	message_t msg;
	memset(&msg,0,sizeof(message_t));
	msg.hdr.op = message.hdr.op;
	strncpy(msg.hdr.sender,message.hdr.sender,strlen(message.hdr.sender)+1);
	strncpy(msg.data.hdr.receiver,message.data.hdr.receiver,strlen(message.data.hdr.receiver)+1);
	msg.data.hdr.len = message.data.hdr.len;
	msg.data.buf = malloc(sizeof(char)*(strlen(message.data.buf)+1));
	strncpy(msg.data.buf,message.data.buf,strlen(message.data.buf)+1);
	return msg;
}

/**
 * @function Working
 * @brief funzione eseguita dai thread worker in pool per gestire
 * 			 richieste dai vari client
 *
 * @param arg : puntatore agli argomenti del Worker
 */
static void *Working(void *arg){

	while(1){
		/* -- leggo richiesta in sospeso da queue -- */
		long fd;
		fd= Pop(queue);
		int rimosso = 0;
		int mancatacon =0;
		int maxcon = elenco.MaxConnections;
		if(fd == -1)
		{
			return (void*) NULL;
		}else{
			message_t msg;
			memset(&msg,0,sizeof(message_t));
			/* -- leggo il messaggio della richiesta -- */
			int r = readMsg(fd,&msg);
			if ( r <= 0)
			{
				/* -- rimuovo user online -- */
				r = rem_user_byfd(user_list,fd,maxcon);
				close(fd);
				rimosso = 1;
				if(r != 0)
				{
					aggiornaStats(0,-1,0,0,0,0,0);
				}
			}
			else
			{
				/* -- Eseguo richiesta ricevuta -- */
				switch(msg.hdr.op){
					case(REGISTER_OP):{
						Registra(msg,fd);
						break;
					}case(CONNECT_OP):{
						mancatacon = Connetti(msg,fd);
						break;
					}case (POSTTXT_OP):{
						InviaTxtToOne(msg,fd);
						break;
					}case(POSTTXTALL_OP):{
						InviaTxtToAll(msg,fd);
						break;
					}case(POSTFILE_OP):{
						InviaFileToOne(msg,fd);
						break;
					}case(GETFILE_OP):{
						DownloadFile(msg,fd);
						break;
					}case(USRLIST_OP):{
						InvioUserList(msg,fd);
						break;
					}case(GETPREVMSGS_OP):{
						DownloadHistoryMSG(msg,fd);
						break;
					}case(UNREGISTER_OP):{
						Deregistra(msg,fd);
						break;
					}
					default: break;
				}
				if(rimosso == 0 && mancatacon != -1)
				{
					/* -- riaggiungo fd in set scrivendolo in pipe  -- */
					int notused;
					long agg;
					memset(&agg,0,sizeof(long));
					agg = fd;
					if (agg != 0 && agg!=-1 && agg >0)
					SYSCALL(notused,write(pipe_t[1],&agg,sizeof(long)),"write in pipe");
				}
			}
		}
	}
	return NULL;
}

/**
 * @function Registra
 * @brief svolge operazione di registrazione utente
 *
 * @param msg : messaggio inviato dall'utente che si vuole registrare
 * @param fd : indirizzo dell'utente che si vuole registrare
 */
void Registra(message_t msg,long fd){
	int maxcon = elenco.MaxConnections;
	u* newUser = createUser(fd,msg.hdr.sender);
	message_t risp;
	memset(&risp,0,sizeof(risp));
	op_t resp;
	/* --- Cerco utente in tabella hash degli utenti registrati se non lo trovo lo inserisco --- */
	if(icl_hash_insert_sup(h,newUser->name,newUser)!=NULL)
	{
		aggiornaStats(1,0,0,0,0,0,0);
		resp = OP_OK;
		/* -- inserisco utente registrato in user_list degli utenti online -- */
		add_user(user_list,newUser->name,maxcon,fd);
		aggiornaStats(0,1,0,0,0,0,0);
		int nonline =readNonline();
		int dim =nonline*(MAX_NAME_LENGTH+1);
		/* -- inizializzo history del nuovo utente -- */
		newUser->s = initHist(elenco.MaxHistMsgs,newUser->mia);
		/* -- invio risposta al client --*/
		memset(&risp,0,sizeof(message_t));
		user *utente_o=find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
		setHeader(&risp.hdr,resp,"server");
		setData(&risp.data,newUser->name,"",dim);
		Invio_Lista_Utenti_Online_2(user_list,utente_o,fd,nonline,maxcon,risp,msg);
	}
	else
	{
		resp = OP_NICK_ALREADY;
		/* -- invio risposta di errore al client -- */
		add_user(user_list,newUser->name,maxcon,fd);
		aggiornaStats(0,1,0,0,0,0,0);
		user *utente_o=find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
		setHeader(&risp.hdr,resp,msg.hdr.sender);
		InvioHeaderToUserNOnline(utente_o,fd,risp);
		aggiornaStats(0,0,0,0,0,0,1);
		deleteuser2(newUser);
	}
}

/**
 * @function Connetti
 * @brief svolge operazione di connessione utente
 *
 * @param msg : messaggio inviato dall'utente che si vuole connettere
 * @param fd : indirizzo dell'utente che si vuole connettere
 *
 * @returns 1 se utente connesso con successo -1 altrimenti
 */
int Connetti(message_t msg,long fd){
	op_t resp;
	message_t risp;
	memset(&risp,0,sizeof(risp));
	int maxcon = elenco.MaxConnections;
	/* -- cerco utente da connettere in tab hash -- */
	if(icl_hash_find_sup(h,msg.hdr.sender)==NULL){
		/* -- invio risposta di errore al client -- */
		resp = OP_NICK_UNKNOWN;
		setHeader(&risp.hdr,resp,"server");
		sendHeader(fd,&risp.hdr);
		return -1;
	}
	resp = OP_OK;
	/* -- inserisco utente in user_list degli utenti online -- */
	add_user(user_list,msg.hdr.sender,elenco.MaxConnections,fd);
	aggiornaStats(0,1,0,0,0,0,0);
	int nonline = readNonline();
	int dim =nonline*(MAX_NAME_LENGTH+1);
	user *utente_con=find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	memset(&risp,0,sizeof(message_t));
	setHeader(&risp.hdr,resp,"server");
	setData(&risp.data,msg.hdr.sender,"",dim);
	/* -- invio risposta al client -- */
	Invio_Lista_Utenti_Online_2(user_list,utente_con,fd,nonline,maxcon,risp,msg);
	return 1;
}

/**
 * @function InviaTxtToOne
 * @brief invia messaggio ad un altro utente registrato
 *
 * @param msg : contiene info messaggio da inviare
 * @param fd : indirizzo dell'utente che invia
 */
void InviaTxtToOne(message_t msg,long fd){
	op_t resp;
	message_t risp;
	memset(&risp,0,sizeof(risp));
	u *use;
	user * utente_o=NULL;
	/* -- cerco utente receiver in tabella hash -- */
	use =(u*) icl_hash_find_sup(h,msg.data.hdr.receiver);
	/* -- cerco utente sender in user_list degli utenti online -- */
	user *utente_invia = find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	if ( use == NULL )
	{
		resp = OP_NICK_UNKNOWN;
	}else{
		if( strlen(msg.data.buf) > elenco.MaxMsgSize)
		{
			resp = OP_MSG_TOOLONG;
			/* -- invio messaggio di errore se messaggio troppo lungo -- */
			setHeader(&risp.hdr,resp,"server");
			InvioHeaderToUserNOnline(utente_invia,fd,risp);
			free(msg.data.buf);
			aggiornaStats(0,0,0,0,0,0,1);
			return;
		}
		else
		{
			/* -- aggiungo messaggi ad history del receiver -- */
			message_t copy = create_copy_msg(msg);
			utente_o=find_user(user_list,msg.data.hdr.receiver,elenco.MaxConnections);
			add( use->s,&copy,use->mia,elenco.MaxHistMsgs);
		}
	}
	if(utente_o != NULL)
	{
			resp = TXT_MESSAGE;
	}
	else
	{
			resp = OP_OK;
	}
	/* -- invio risposta al client -- */
	if(resp == TXT_MESSAGE)
	{
		setHeader(&risp.hdr,resp,msg.hdr.sender);
		setData(&risp.data,msg.data.hdr.receiver,msg.data.buf,strlen(msg.data.buf)+1);
		long fd_on = utente_o->fd;
		InvioToUserOnline(utente_o,fd_on,risp);
		setHeader(&risp.hdr,OP_OK,msg.hdr.sender);
		InvioHeaderToUserNOnline_2(fd,risp);
		free(msg.data.buf);
		aggiornaStats(0,0,1,0,0,0,0);
	}
	else
	{
		if(resp == OP_OK)
		{
			setHeader(&risp.hdr,resp,"server");
			InvioHeaderToUserNOnline_2(fd,risp);
			free(msg.data.buf);
			aggiornaStats(0,0,0,1,0,0,0);
		}
		else
		{
			/* -- casi di errore -- */
			setHeader(&risp.hdr,resp,msg.hdr.sender);
			InvioHeaderToUserNOnline(utente_invia,fd,risp);
			free(msg.data.buf);
			aggiornaStats(0,0,0,0,0,0,1);
		}
	}
}

/**
 * @function InviaTxtToAll
 * @brief invia messaggio a tutti gli utenti registrati
 *
 * @param msg : contiene info messaggio da inviare
 * @param fd : indirizzo dell'utente che invia
 */
void InviaTxtToAll(message_t msg, long fd){
	int nusers;
	message_t risp;
	memset(&risp,0,sizeof(risp));
	op_t resp;
	/* -- cerco utente sender in user_list degli utenti online -- */
	user *utente_invia = find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	/* -- verifico dimensione messaggio da inviare in caso invio messaggio di errore -- */
	if( strlen(msg.data.buf) > elenco.MaxMsgSize )
	{
		resp = OP_MSG_TOOLONG;
		setHeader(&risp.hdr,resp,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
		free(msg.data.buf);
		msg.data.buf = NULL;
		return;
	}
	else
	{
		nusers = readNusers();
		/* -- creo eleneco di utenti registrati -- */
		char **user_register_list =icl_m_user_list(h,MAX_NAME_LENGTH,nusers);
		for( int i =0; i<nusers; i++)
		{
			if(strcmp(user_register_list[i],msg.hdr.sender)!=0)
			{
				u *use;
				user * utente_o;
				/* -- cerco utente receiver[i] in hash -- */
				use =(u*) icl_hash_find_sup(h,user_register_list[i]);
				/* -- cerco utente receiver[i] in user_lsit degli utenti online -- */
				utente_o=find_user(user_list,user_register_list[i],elenco.MaxConnections);
				message_t copy = create_copy_msg(msg);
				/* -- aggiungo msg in history del receiver[i] --*/
				add( use->s,&copy,use->mia,elenco.MaxHistMsgs);
				if(utente_o != NULL)
				{
					resp = TXT_MESSAGE;
				}
				else
				{
					resp = OP_OK;
				}
				/* -- invio msg al client receiver[i] se online -- */
				if(resp == TXT_MESSAGE)
				{
					memset(&risp,0,(sizeof(message_t)));
					setHeader(&risp.hdr,TXT_MESSAGE,msg.hdr.sender);
					setData(&risp.data,user_register_list[i],msg.data.buf,strlen(msg.data.buf)+1);
					long fd_on = utente_o->fd;
					InvioToUserOnline(utente_o,fd_on,risp);
					aggiornaStats(0,0,1,0,0,0,0);
				}
				else
				{
					aggiornaStats(0,0,0,1,0,0,0);
				}
			}
		}
		/* -- distruggo elenco user registrati -- */
		for(int i=0;i<nusers;i++)
		{
			if(user_register_list[i]!= NULL){
				free(user_register_list[i]);
				user_register_list[i] = NULL;
			}
		}
		free(user_register_list);
		/* -- invio risposta a utente sender -- */
		resp = OP_OK;
		memset(&risp,0,(sizeof(message_t)));
		setHeader(&risp.hdr,OP_OK,msg.hdr.sender);
		InvioHeaderToUserNOnline_2(fd,risp);
		if(msg.data.buf!=NULL){
			free(msg.data.buf);
			msg.data.buf = NULL;
		}
	}
}

/**
 * @function InviaFileToOne
 * @brief invia file ad un altro utente registrato
 *
 * @param msg : contiene info del file da inviare
 * @param fd : indirizzo dell'utente che invia
 */
void InviaFileToOne(message_t msg,long fd){
	int notused;
	message_t risp;
	memset(&risp,0,sizeof(risp));
	op_t resp;
	u *use;
	user * utente_o;
	/* -- cerco utente receiver in tab hash -- */
	use =(u*) icl_hash_find_sup(h,msg.data.hdr.receiver);
	/* -- cerco utente sender in user_list degli utenti online -- */
	user* utente_invia =find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	if ( use == NULL )
	{
		resp = OP_NICK_UNKNOWN;
	}
	else
	{
		if( msg.data.hdr.len > elenco.MaxFileSize * 1024 )
		{
				resp = OP_MSG_TOOLONG;
		}
		else
		{
			/* -- aggiungo il messaggio nella histor del receiver -- */
			utente_o=find_user(user_list,msg.data.hdr.receiver,elenco.MaxConnections);
			message_t eco;
			memset(&eco,0,sizeof(message_t));
			setData(&eco.data,msg.data.hdr.receiver,msg.data.buf,msg.data.hdr.len);
			setHeader(&eco.hdr,FILE_MESSAGE,msg.hdr.sender);
			add( use->s,&eco, use->mia,elenco.MaxHistMsgs);
		}
	}
	if( utente_o != NULL)
	{
		resp = FILE_MESSAGE;
	}
	else
	{
		resp = OP_OK;
	}
	if(resp != FILE_MESSAGE && resp != OP_OK){
		/* -- invio messaggio d'errore se msg troppo grande o se ricevente non registrato -- */
		setHeader(&risp.hdr,resp,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
		free(msg.data.buf);
		aggiornaStats(0,0,0,0,0,0,1);
		return;
	}
	/* -- creo path file da inviare -- */
	int dim_file = strlen(elenco.DirName)+ strlen(msg.data.buf)+2;
	char *file = malloc(sizeof(char)*dim_file);
	memset(file,0,dim_file);
	strcat(file,elenco.DirName);
	strcat(file,"/");
	strcat(file,basename(msg.data.buf));
	message_t ms2;
	memset(&ms2,0,sizeof(ms2));
	if(readData(fd,&(ms2.data))<0){
		free(ms2.data.buf);
		return;
	}
	if(ms2.data.hdr.len > elenco.MaxFileSize * 1024){

		resp = OP_MSG_TOOLONG;
	}
	/* -- apro il file da inviare -- */
	FILE* file_id=fopen(file,"w");
	if(file_id == NULL)
	{
		resp = OP_FAIL;
	}
	/* -- scrivo nel file da inviare il conten -- */
	if(resp!= OP_MSG_TOOLONG){
		fwrite(ms2.data.buf,sizeof(char),ms2.data.hdr.len,file_id);

	}
	SYSCALL(notused,fclose(file_id),"fclode file_id in InviaFileToOne");
	/* -- invio risposta -- */
	if(resp == FILE_MESSAGE)
	{
		setHeader(&risp.hdr,FILE_MESSAGE,msg.hdr.sender);
		setData(&risp.data,msg.data.hdr.receiver,msg.data.buf,strlen(msg.data.buf)+1);
		long fd_on = utente_o->fd;
		InvioToUserOnline(utente_o,fd_on,risp);
		setHeader(&risp.hdr,OP_OK,msg.hdr.sender);
		InvioHeaderToUserNOnline_2(fd,risp);
		free(ms2.data.buf);
		free(file);
		aggiornaStats(0,0,0,0,1,0,0);
	}
	else
	{
		if(resp == OP_OK)
		{
			setHeader(&risp.hdr,OP_OK,"server");
			InvioHeaderToUserNOnline_2(fd,risp);
			free(ms2.data.buf);
			free(file);
			aggiornaStats(0,0,0,0,0,1,0);
		}
		else
		{
				setHeader(&risp.hdr,resp,"server");
				InvioHeaderToUserNOnline(utente_invia,fd,risp);
				aggiornaStats(0,0,0,0,0,0,1);
				free(file);
				free(ms2.data.buf);
		}
	}
}

/**
 * @function DownloadFile
 * @brief svolge richiesta download file
 *
 * @param msg : contiene info del file da scaricare
 * @param fd : indirizzo dell'utente che richiede DownloadFile
 */
void DownloadFile(message_t msg, long fd){
	int notused;
	message_t risp;
	memset(&risp,0,sizeof(message_t));
	/* -- cerco utente sender in user_list degli utenti online -- */
	user* utente_invia =find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	int not_procedo = 0;
	int file_open = 0;
	op_t resp;
	/* -- Creo stringa nome del file da mappare -- */
	int dimpath = strlen(msg.data.buf)+strlen(elenco.DirName)+2;
	char *file = malloc(sizeof(char)*dimpath);
	memset(file,0,dimpath);
	strcat(file,elenco.DirName);
	strcat(file,"/");
	strcat(file,msg.data.buf);
	/* -- Apro il file da mappare -- */
	char *filemappato;
	int ok = open(file,O_RDWR,0666);
	struct stat st;
	int dimfile;
	if( ok < 0)
	{
		resp = OP_NO_SUCH_FILE;
		not_procedo = 1;
	}
	else
	{
		file_open = 1;
		/* -- useo struct stat e verifico che sia un file regolare -- */
		if(stat(file,&st) == -1 || !S_ISREG(st.st_mode))
		{
			resp = OP_NO_SUCH_FILE;
			not_procedo = 1;
		}
		else
		{
			/* -- verifico dimensione del file da inviare -- */
			if(st.st_size  > elenco.MaxFileSize * 1024)
			{
				resp = OP_MSG_TOOLONG;
				not_procedo = 1;
			}
		}
	}
	if(not_procedo == 0)
	{
		dimfile = st.st_size;
		/* -- procedo con la mappatura del file in memoria per scaricarlo -- */
		filemappato = mmap(NULL,dimfile,PROT_READ,MAP_PRIVATE,ok,0);
		if(filemappato == MAP_FAILED)
		{
			resp = OP_NO_SUCH_FILE;
			not_procedo = 1;
		}
	}
	if(file_open == 1)
	{
		SYSCALL(notused,close(ok),"close file mappato in DownloadFile");
	}
	if(not_procedo == 1)
	{
		free(file);
		free(msg.data.buf);
	}
	if(not_procedo == 0)
	{
		resp = OP_OK;
		free(file);
		free(msg.data.buf);
	}
	/* -- invio risposta -- */
	if(resp == OP_OK)
	{
		aggiornaStats(0,0,0,0,1,-1,0);
		setHeader(&risp.hdr,resp,"server");
		setData(&risp.data,msg.data.hdr.receiver,filemappato,dimfile);
		InvioToUserOnline(utente_invia,fd,risp);
	}
	else
	{
		aggiornaStats(0,0,0,0,0,0,1);
		setHeader(&risp.hdr,resp,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
	}
}

/**
 * @function InvioUserList
 * @brief invia lista utenti registrati
 *
 * @param msg : contiene info per inviare lista utenti registrati a utente sender
 * @param fd : indirizzo dell'utente che richiede UserList degli utenti registrati
 */
void InvioUserList(message_t msg, long fd){
	message_t risp;
	memset(&risp,0,sizeof(message_t));
	int maxcon = elenco.MaxConnections;
	op_t resp;
	int nonline;
	nonline = readNonline();
	/* -- cerco utente sender in user_list degli utenti online -- */
	user* utente_invia =find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	if(nonline<=0){
		resp = OP_FAIL;
	}else{
		resp = OP_OK;
	}
	/* -- invio risposta -- */
	if(resp == OP_OK)
	{
		int dim =nonline*(MAX_NAME_LENGTH+1);
		setHeader(&risp.hdr,resp,"server");
		setData(&risp.data,"","",dim);
		Invio_Lista_Utenti_Online_2(user_list,utente_invia,fd,nonline,maxcon,risp,msg);
	}
	else
	{
		setHeader(&risp.hdr,resp,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
		aggiornaStats(0,0,0,0,0,0,1);
	}
}

/**
 * @function DownloadHistoryMSG
 * @brief scarica l'history dell'utente sender
 *
 * @param msg : contiene info per scaricare history
 * @param fd : indirizzo dell'utente che richiede download history
 */
void DownloadHistoryMSG(message_t msg,long fd){
	message_t risp;
	u *use;
	/* -- cerco utente sender in tab hash -- */
	use =(u*) icl_hash_find_sup(h,msg.hdr.sender);
	/* -- cerco utente receiver in user_list degli utenti online -- */
	user* utente_invia =find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	if(use == NULL){
		setHeader(&(risp.hdr),OP_NICK_UNKNOWN,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
		aggiornaStats(0,0,0,0,0,0,1);
	}
	/* -- leggo numero di messaggi presenti in history -- */
	int conta = readConta(use->s,use->mia);
	if(conta > 0)
	{
		setHeader(&(risp.hdr),OP_OK,"server");
		setData(&(risp.data),"",(char*)&(conta),sizeof(int));

		/* -- invio history -- */
		InvioToUserOnline(utente_invia,fd,risp);
		size_t i=0;
		int index = 0;

		/* -- prendo mutua esclusione su history con var "mia" -- */
		pthread_mutex_lock(&use->mia);
		index = use->s->testa;
		while(i < conta && index!=use->s->fine)
		{
			if(use->s->messaggi[index]!=NULL)
			{
				InvioToUserOnline(utente_invia,fd,*(use->s->messaggi[index]));
				aggiornaStats(0,0,1,-1,0,0,0);
				index=(index+1)%elenco.MaxHistMsgs;
				i++;
			}
			else
			{
				index=(index+1)%elenco.MaxHistMsgs;
			}
		}
		if(conta >= elenco.MaxHistMsgs)
			InvioToUserOnline(utente_invia,fd,*(use->s->messaggi[index]));

		pthread_mutex_unlock(&use->mia);
	}
	else
	{
		/* -- invio messaggio di errore -- */
		setHeader(&(risp.hdr),OP_FAIL,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
		aggiornaStats(0,0,0,0,0,0,1);
	}
}

/**
 * @function Deregistra
 * @brief elimina da hash utente sender
 *
 * @param msg : contiene indo utente da deregistrare
 * @param fd : indirizzo dell'utente che richiede deregistrazione
 */
void Deregistra(message_t msg,long fd){
	message_t risp;
	memset(&risp,0,sizeof(risp));
	/* -- cerco utente sender in user_list degli utenti online -- */
	user* utente_invia =find_user(user_list,msg.hdr.sender,elenco.MaxConnections);
	op_t resp;
	u *use;
	/* -- cerco utente sender in tab hash -- */
	use =(u*) icl_hash_find_sup(h,msg.hdr.sender);
	if( use == NULL)
	{
		/* -- invio risposta di errore -- */
		resp = OP_NICK_UNKNOWN;
		aggiornaStats(0,0,0,0,0,0,1);
		setHeader(&(risp.hdr),resp,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
	}
	else
	{
		/* -- rimuovo utente da tab hash degli utenti registrati -- */
		int ok = icl_hash_delete_sup( h, msg.hdr.sender, deletekey, deleteuser);
		if(ok >= 0)
		{
			resp = OP_OK;
			aggiornaStats(-1,0,0,0,0,0,0);
		}
		else
		{
			resp = OP_NICK_UNKNOWN;
			aggiornaStats(0,0,0,0,0,0,1);
		}
		/* -- invio rispota -- */
		setHeader(&(risp.hdr),resp,"server");
		InvioHeaderToUserNOnline(utente_invia,fd,risp);
	}
}
