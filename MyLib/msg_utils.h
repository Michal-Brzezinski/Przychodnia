#pragma once
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

#define MAX_KOMUNIKATOW 113 // maksymalna liczba komunikatow w kolejce dla limitow systemowych
                            // w moim przypadku to 113 komunikatow, komunikaty w tym programie maja rozmiar 144B

typedef struct {
    long mtype;       // Typ wiadomosci
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza 
    char kto_skierowal[128]; // nazwa lekarza, ktory skierowal
} Wiadomosc;
//  struktura wiadomosci w rejestracji

int alokujKolejkeKomunikatow(key_t klucz, int flagi);
int zwolnijKolejkeKomunikatow(key_t  klucz);
int policzProcesy(int msg_id);
