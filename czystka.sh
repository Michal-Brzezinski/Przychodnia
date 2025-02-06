#!/bin/bash

DIRECTORY1="./*.o" 
DIRECTORY2="MyLib/*.o"

rm -f mainprog pacjent rejestracja lekarz dyrektor fifo_dyrektor

# Usuwanie plików z katalogu MyLib
for file in $DIRECTORY1; do
    if [ -e $file ]; then
        rm -f $file
    fi
done

# Usuwanie plików z katalogu MyLib
for file in $DIRECTORY2; do
    if [ -e $file ]; then
        rm -f $file
    fi
done
