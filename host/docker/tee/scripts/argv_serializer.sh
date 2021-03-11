#!/bin/bash
#set -ex

TRUSTYED_ARGV_FILE=$PWD/trusted_argv

while [ "$#" -gt 0  ]
do
	echo -en "$1\0" >> $TRUSTYED_ARGV_FILE
	shift
done

echo -en "{{"\""\0"\"".join(cmd)}}\0" >> $TRUSTYED_ARGV_FILE