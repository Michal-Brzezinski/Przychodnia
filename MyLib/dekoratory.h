#pragma once
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <sys/wait.h>

const  static char *czerwony = "\033[1;31m";    // Czerwony kolor
const static char *zielony = "\033[1;32m";  // Zielony kolor
const static char *zolty = "\033[1;33m";  // Żolty kolor
const static char *niebieski = "\033[1;34m";  // Niebieski kolor
const static char *magenta = "\033[1;35m";  // Magenta kolor
const static char *cyjan = "\033[1;36m";  // Cyjanowy kolor
const static char *reset = "\033[0m";  // Resetowanie koloru do domyslnego


int losuj_int(int N);
key_t generuj_klucz_ftok(const char *sciezka, char projek_id);

void print(const char *format, ...);
void printRed(const char *format, ...);
void printBlue(const char *format, ...);
void printGreen(const char *format, ...);
void printYellow(const char *format, ...);
void printCyan(const char *format, ...); 
void printMagenta(const char *format, ...);
void perror_red(const char *s);
//int procentNaNaturalna(int n, int x);
void zwrocTabliceLimitowLekarzy(int limit_pacjentow, int *limity_lekarzy);
void oczekujNaProces(pid_t pid, const char *nazwa_procesu);
void czekaj_na_procesy(pid_t *pid_array, int size);
void wyczyscProcesyPacjentow();
void usunNiepotrzebnePliki();
