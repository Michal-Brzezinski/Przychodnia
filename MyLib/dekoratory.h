#pragma once
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

const  static char *czerwony = "\033[1;31m";    // Czerwony kolor
const static char *zielony = "\033[1;32m";  // Zielony kolor
const static char *niebieski = "\033[1;34m";  // Niebieski kolor
const static char *zolty = "\033[1;33m";  // Żółty kolor
const static char *reset = "\033[0m";  // Resetowanie koloru do domyślnego


int losuj_int(int N);
key_t generuj_klucz_ftok(const char *sciezka, char projek_id);

void print_fflush(const char *tekst);
void printRed(const char *tekst);
void printBlue(const char *tekst);
void printGreen(const char *tekst);
void printYellow(const char *tekst);