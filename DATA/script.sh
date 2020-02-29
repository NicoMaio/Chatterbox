#!/bin/bash

# verifico che siano stati passati due argomenti
if [[ $# -lt 2 ]]; then
	echo "use ./script.sh <chatty.conf1 or chatty.conf2> <minutes>" 1>&2
	exit 1
fi

if [ $1 == "-help" -o $2 == "-help" ]; then
	echo "use ./script.sh <chatty.conf1 or chatty.conf2> <minutes>" 1>&2
	exit 1
fi


# verifico che il file passato esista
if [[ ! -e "$1" ]]; then
	echo "$1 not exist" 1>&2
	exit 1
fi

if [ ! 'isnum "$2"' ]; then
	echo "$2 is not a valid number" 1>&2
	exit 1
fi

DN=""

# scorro il file in cerca di DirName
while read -r conf_name conf_value; do
	if [[ $conf_name == "DirName" ]]; then
		DN=$conf_value
	fi
done < $1

# rimuobo =
DN="$(echo -e "${DN}" | tr -d '=')"
# rimuovo spazio bianco
DN="$(echo -e "${DN}" | tr -d '[:space:]')"
# rimuovo ""
DN="$(echo -e "${DN}" | tr -d '\"')"

cd ..$DN
tarball="archive.tar.gz"

# se il secondo parametro è 0 stampo il contenuto di DirName
if [ $2 -eq 0 ]; then
	cd ..
	ls ..$DN
	exit 1
fi

elenco="elencofile.txt"

# cerco file modificati più di $2 secondi fa e li inserisco in $elenco
# con -type f certifico che sto selezionando solo file per le ricerche
find -type f -mmin +$2 > $elenco

# verifico che l'elenco sia diversamente vuoto
if [ -s $elenco ];

	then
		# creo tarball
		tar -czf $tarball -T $elenco
		echo "$tarball created with success"
		# elimino tutti i file diversi dal tarball
		find . ! -name $tarball -delete
		exit 1
	else
		# elenco vuoto
		echo "No file was modified more in $2 minutes" 1>&2
fi
rm $elenco
