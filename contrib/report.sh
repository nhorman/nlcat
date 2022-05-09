#!/bin/sh

#simple example script to show how nlcat can be used in a script to read in data
#to a script for handling and dispatch

while read LINE
do
	echo $LINE | jq .
done < <(../nlcat)

