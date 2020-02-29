/**
 * chatterbox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 *
 */
/**
 * @file  parset.c
 * @brief Contiene implementazione funzioni dichiarate in parset.h
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

#include "parset.h"

/**
 * @function read_str_from_config_line
 * @brief legge una stringa da una riga
 *
 * @param config_line riga da cui leggo la stringa
 * @param val stringa in cui salvo caratteri letti
 */
void read_str_from_config_line(char* config_line, char* val) {
    char prm_name[MAX_CONFIG_VARIABLE_LEN];
    sscanf(config_line, "%s", prm_name);
    int i=0;
    while(config_line[i]!='=')
    {
    	i++;
    }
    i++;
    int dim = (strlen(config_line)-i)+1;
    char stringa[dim];
    int j=0;
    while(j<dim-1)
    {
    	stringa[j]=config_line[i];
    	j++;
    	i++;
    }
    stringa[dim-1]='\0';
    char ret[dim];
    sscanf(stringa,"%s",ret);
    val[0]='.';
    j =1;
    for(int h=0;h<dim-1;h++)
    {
        val[j]=ret[h];
        j++;
    }
    val[dim-1]='\0';
}

/**
 * @function read_int_from_config_line
 * @brief legge un intero da una riga
 *
 * @param config_line riga da cui leggo intero
 *
 * @return intero letto
 */
int read_int_from_config_line(char* config_line) {
	// Selezionata la stringa utile per separare stringa e intero
	// scorro linearmente dopo aver fatto la prima sscanf
    char prm_name[MAX_CONFIG_VARIABLE_LEN];
   	int i=0;
    sscanf(config_line, "%s", prm_name);
    while(config_line[i]!='=')
    {
    	i++;
    }
    i++;
    int dim = (strlen(config_line)-i)+1;
    char stringa[dim];
    int j=0;
    while(j<dim-1)
    {
    	stringa[j]=config_line[i];
    	j++;
    	i++;
    }
    stringa[dim-1]='\0';
    char val[5];
    sscanf(stringa,"%s",val);
    return atoi(val);
}

/**
 * @function read_config_file
 * @brief legge le righe non commentate nel file di configurazione
 *        e da esse estrae le stringhe e gli interi richiesti
 *
 * @param config_filename nome del file da cui leggere righe
 * @param config struttura da riempire leggendo file di configurazione
 */
void read_config_file(char* config_filename, Config* config) {
    FILE *fp;
    char buf[CONFIG_LINE_BUFFER_SIZE];
    if ((fp=fopen(config_filename, "r")) == NULL)
    {
        fprintf(stderr, "Failed to open config file %s", config_filename);
        exit(EXIT_FAILURE);
    }
    while(!feof(fp))
    {
        fgets(buf, CONFIG_LINE_BUFFER_SIZE, fp);
        if (buf[0] == '#' || strlen(buf) < 8)
        {
            continue;
        }
        if (strstr(buf, "UnixPath"))
        {
            read_str_from_config_line(buf, config->UnixPath);
        }
        if (strstr(buf, "DirName"))
        {
            read_str_from_config_line(buf, config->DirName);
        }
        if (strstr(buf, "StatFileName"))
        {
            read_str_from_config_line(buf, config->StatFileName);
        }
       	if (strstr(buf, "MaxConnections"))
        {
            config->MaxConnections = read_int_from_config_line(buf);
        }
        if (strstr(buf, "ThreadsInPool"))
        {
            config->ThreadsInPool = read_int_from_config_line(buf);
        }
        if (strstr(buf, "MaxMsgSize"))
        {
            config->MaxMsgSize = read_int_from_config_line(buf);
        }
        if (strstr(buf, "MaxFileSize"))
        {
            config->MaxFileSize = read_int_from_config_line(buf);
        }
        if (strstr(buf, "MaxHistMsgs"))
        {
            config->MaxHistMsgs = read_int_from_config_line(buf);
        }
    }
    int notused;
    SYSCALL(notused,fclose(fp),"fclose file chatty.conf");
}
