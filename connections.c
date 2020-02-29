/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 *
 * @file connections.c
 * @brief contiene implementazione funzioni dichiarate in connections.h
 *
 * @author Nicolo` Maio 544935
 */

#include <connections.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <mylib.h>



// writen e readn presenti in mylib.h

/**
 * @function openConnection
 * @brief Apre una connessione AF_UNIX verso il server
 *
 * @param path Path del socket AF_UNIX
 * @param ntimes numero massimo di tentativi di retry
 * @param secs tempo di attesa tra due retry consecutive
 *
 * @returns il descrittore associato alla connessione in caso di successo
 *         -1 in caso di errore
 */

int openConnection(char* path, unsigned int ntimes, unsigned int secs){
	/* -- imposto socket -- */
	struct sockaddr_un sa;
	strncpy(sa.sun_path,path,UNIX_PATH_MAX);
	sa.sun_family=AF_UNIX;
	int cfd_skt= socket(AF_UNIX,SOCK_STREAM,0);
	if(cfd_skt == -1)
	{
		perror("Funzione socket client");
		exit(errno);
	}
	int i = ntimes;
	int ok = -1;
	/* -- stabilisco connessione con socket -- */
	while (i>0 && ok < 0){
		ok = connect(cfd_skt,(struct sockaddr*)&sa,sizeof(sa));
		if(errno == ENOENT){
			sleep(secs);
			i--;
			//ok=-1;
		}
	}
	if ( ok < 0) return -1;
	else return cfd_skt;

}


// -------- server side -----
/**
 * @function readHeader
 * @brief Legge l'header del messaggio
 *
 * @param fd     descrittore della connessione
 * @param hdr    puntatore all'header del messaggio da ricevere
 *
 * @returns <=0 se c'e' stato un errore
 *         (se <0 errno deve essere settato, se == 0 connessione chiusa)
 */
int readHeader(long connfd, message_hdr_t *hdr){
	int tmp =0;
	memset(hdr, 0, sizeof(message_hdr_t));
	/* -- leggo op -- */
	tmp = read (connfd, &(hdr->op), sizeof(op_t));
	if(tmp<0)
	{
		return -1;
	}
	/* -- leggo sender -- */
	tmp = readn(connfd,hdr->sender,sizeof(char)*(MAX_NAME_LENGTH+1));
	if(tmp == 0)
	{
		close(connfd);
		return 0;
	}
	else
	{
		if(tmp < 0)
		{
			return -1;
		}
	}
	return 1;
	// in caso di errore, errno viene settato in readn
}

/**
 * @function readData
 * @brief Legge il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al body del messaggio
 *
 * @returns <=0 se c'e' stato un errore
 *         (se <0 errno deve essere settato, se == 0 connessione chiusa)
 */
int readData(long fd, message_data_t *data){
	int tmp =0;
	memset(data, 0, sizeof(message_data_t));
	/* -- leggo receiver -- */
	tmp = readn (fd,data->hdr.receiver,sizeof(char)*(MAX_NAME_LENGTH+1));
	if(tmp<0)
	{
		return -1;
	}
	if(tmp == 0)
	{
		close(fd);
		return 0;
	}
	/* -- leggo dimensione msg -- */
	tmp = read(fd,&data->hdr.len,sizeof(int));
	if(data->hdr.len>0)
	{
		data->buf= (char*) malloc(sizeof(char)*(data->hdr.len));
		memset(data->buf, 0, sizeof(char)*(data->hdr.len));
		/* -- leggo buffer messaggio -- */
		tmp = readn (fd, data->buf, sizeof(char)*(data->hdr.len));
		if(tmp<0)
		{
			free(data->buf);return -1;
		}
		if(tmp == 0)
		{
			close(fd);
			free(data->buf);
			return 0;
		}

	}
	else free(data->buf);
	return 1;
}

/**
 * @function readMsg
 * @brief Legge l'intero messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al messaggio
 *
 * @returns <=0 se c'e' stato un errore
 *         (se <0 errno deve essere settato, se == 0 connessione chiusa)
 */
int readMsg(long fd, message_t *msg){


	int tmp=0;

	memset(msg,0,sizeof(message_t));
	/* -- leggo header -- */
	tmp = readHeader(fd, &msg->hdr);
	if (tmp == 0)
	{
		close(fd);
		return 0;
	}
	if(tmp<0)
	{
		return -1;
	}
	/* -- leggo data -- */
	tmp = readData(fd, &msg->data);
	if (tmp == 0)
	{
		close(fd);
		return 0;
	}
	if(tmp<0)
	{
		return -1;
	}
	return 1;
	// in caso di errore, errno viene settato in readn
	// eseguita in readData e readHeader
}

/* da completare da parte dello studente con altri metodi di interfaccia */


// ------- client side ------
/**
 * @function sendRequest
 * @brief Invia un messaggio di richiesta al server
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @returns <=0 se c'e' stato un errore
 */
int sendRequest(long fd, message_t *msg){
	int tmp=0;
	/* -- invio header -- */
	tmp = sendHeader(fd, &msg->hdr);
	if(tmp<=0)
	{
	 return tmp;
	}
	/* -- invio data -- */
	tmp = sendData(fd, &msg->data);
	if(tmp<=0)
	{
		return tmp;
	}
	return 1;
}

/**
 * @function sendData
 * @brief Invia il body del messaggio al server
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @returns <=0 se c'e' stato un errore
 */
int sendData(long fd, message_data_t *msg){

	int tmp=0;
	/* -- scrivo su socket stringa receiver -- */
	tmp = writen(fd, msg->hdr.receiver, sizeof(char)*(MAX_NAME_LENGTH +1));
	if(tmp<=0)
	{
		return tmp;
	}
	/* -- scrivo dimensione messaggio -- */
	tmp = write (fd, &msg->hdr.len, sizeof(int));
	if(tmp<=0)
	{
		return tmp;
	}

	if(msg->hdr.len>0)
	{
		/* -- scrivo buffer -- */
		tmp = writen(fd,msg->buf,sizeof(char)*(msg->hdr.len));
		if(tmp<=0)return tmp;
	}
	return 1;

}

/**
 * @function sendHeader
 * @brief Invia header del messaggio al server
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al header da inviare
 *
 * @returns <=0 se c'e' stato un errore
 */
int sendHeader(long fd, message_hdr_t *msg){

	int tmp=0;
	/* -- scrivo op -- */
	tmp = write (fd, &(msg->op), sizeof(op_t));
	if(tmp<=0)
	{
		return tmp;
	}
	/* -- scrivo sender -- */
	tmp = writen(fd,msg->sender,sizeof(char)*(MAX_NAME_LENGTH+1));
	if(tmp<=0)return -1;
	return 1;
}
