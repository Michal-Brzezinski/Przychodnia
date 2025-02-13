#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "utils.h"

#define PAM_SIZE 7      // Rozmiar tablicy pamieci wspoldzielonej

int alokujPamiecWspoldzielona(key_t klucz, int rozmiar, int flagi);
int *dolaczPamiecWspoldzielona(int shmID, int flagi);
int odlaczPamiecWspoldzielona(int *pamiec);
void zwolnijPamiecWspoldzielona(int klucz);

