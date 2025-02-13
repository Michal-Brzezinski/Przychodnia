#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#ifndef SA_RESTART
#define SA_RESTART 0x10000000   
#endif
// w razie braku definicji w systemie (np. u mnie sie jakies bledy pojawialy)
#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0x00000001 
#endif

#include "MyLib/utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"
#include "MyLib/shm_utils.h"

// Definicja typu wyliczeniowego dla lekarzy
enum lekarze{ 
    POZ = 1, 
    KARDIOLOG, 
    OKULISTA, 
    PEDIATRA, 
    MEDYCYNA_PRACY
};

typedef struct{

    int id_lekarz;
    char nazwa[128];
    int licznik_pacjentow;
    int indywidualny_limit;

}Lekarz;

typedef struct {
    long mtype;       // Typ wiadomosci
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza  
    char kto_skierowal[128]; // nazwa lekarza, ktory skierowal
} Wiadomosc;
//  struktura wiadomosci w rejestracji


//  --------------- ZMIENNE GLOBALNE UZYWANE W lekarz.c ---------------------------

pthread_t POZ2;

volatile sig_atomic_t zakoncz_program = 0;  // obsluga SIGINT

volatile sig_atomic_t koniec_pracy = 0; // potrzebne do obslugi sygnalu dyrektora - zakonczenie pracy

int sem_id, msg_id_lekarz, msg_id_rejestracja;
key_t klucz_sem, klucz_kolejki_lekarza, klucz_kolejki_rejestracji;

key_t klucz_wyjscia;
int msg_id_wyjscie;

int limit_POZ2; // globalne do ulatwienia dzialania programu
int limity_lekarzy[5];
int id_lekarz;  // przydaje sie w momencie osblugi sygnalu dyrektora

int Tp, Tk; // godziny otwarcia przychodni

//------------------------  FUNKCJE DEKLAROWANE W HEADERZE  --------------------------------

void *lekarzPOZ2(void* _arg);
void obsluga_SIGINT(int sig);
void obsluga_SIGUSR1(int sig);
void czynnosci_lekarskie(Lekarz *lekarz);
void wyslij_do_specjalisty(Wiadomosc *msg, Lekarz *lekarz);
void badania_ambulatoryjne(Wiadomosc *msg, Lekarz *lekarz);


//------------------------  FUNKCJE DEFINIOWANE W HEADERZE  --------------------------------

void inicjalizuj_lekarza(Lekarz* lekarz, int id_lekarz, int limit_pacjentow){
    /*Funkcja inicjalizuje strukture lekarza*/

    lekarz->id_lekarz = id_lekarz;
    lekarz->licznik_pacjentow = 0;
    lekarz->indywidualny_limit = limit_pacjentow;

    switch (id_lekarz)
    {
    case POZ:
        sprintf(lekarz->nazwa, "Lekarz POZ");
        break;
    case KARDIOLOG:
        sprintf(lekarz->nazwa, "Kardiolog");
        break;
    case OKULISTA:
        sprintf(lekarz->nazwa, "Okulista");
        break;
    case PEDIATRA:
        sprintf(lekarz->nazwa, "Pediatra");
        break;
    case MEDYCYNA_PRACY:
        sprintf(lekarz->nazwa, "Lekarz Medycyny Pracy");
        break;
    default:
        break;
    }

}

Wiadomosc* wypiszPacjentowWKolejce(int msg_id, int *rozmiar_kolejki, Lekarz *lekarz) {
    int capacity = 500; // Poczatkowa pojemnosc tablicy
    int size = 0;
    Wiadomosc *pacjenci = malloc(capacity * sizeof(Wiadomosc));
    if (pacjenci == NULL) {
        perror_red("[wypiszPacjentowWKolejce]: malloc error\n");
        exit(1);
    }

    Wiadomosc msg;
    while (1) {
        errno = 0;
        int ret = msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT);
        if (ret != -1) {
            // Otrzymano wiadomosc
            print("[%s]: Pacjent nr %d, wiek: %d, vip: %d\n",
                  lekarz->nazwa, msg.id_pacjent, msg.wiek, msg.vip);

            // Sprawdzenie, czy potrzebna jest realokacja tablicy
            if (size >= capacity) {
                capacity *= 2;
                Wiadomosc *temp = realloc(pacjenci, capacity * sizeof(Wiadomosc));
                if (temp == NULL) {
                    perror_red("[wypiszPacjentowWKolejce]: realloc error\n");
                    free(pacjenci);
                    exit(1);
                }
                pacjenci = temp;
            }

            pacjenci[size++] = msg;
        } else {
            if (errno == ENOMSG) {
                // Brak wiadomosci w kolejce
                break;
            } else {
                perror_red("[wypiszPacjentowWKolejce]: Blad msgrcv\n");
                free(pacjenci);
                exit(1);
            }
        }
    }

    *rozmiar_kolejki = size;

    // Dopasowanie rozmiaru tablicy do faktycznej liczby pacjentow
    if (size == 0) {
        free(pacjenci);
        pacjenci = NULL;
    } else {
        Wiadomosc *temp = realloc(pacjenci, size * sizeof(Wiadomosc));
        if (temp == NULL) {
            perror_red("[wypiszPacjentowWKolejce]: realloc error\n");
            free(pacjenci);
            exit(1);
        }
        pacjenci = temp;
    }

    return pacjenci;
}


void wypiszIOdeslijPacjentow(Lekarz *lekarz, int msg_id) {
    int rozmiar_pozostalych = 0;
    Wiadomosc *pozostali = wypiszPacjentowWKolejce(msg_id, &rozmiar_pozostalych, lekarz);

    if (pozostali != NULL) {
        // Wysylanie pozostalym pacjentom komunikatu o wyjsciu
        for (int i = 0; i < rozmiar_pozostalych; i++) {
            pozostali[i].mtype = pozostali[i].id_pacjent;
            if (msgsnd(msg_id_wyjscie, &pozostali[i], sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Lekarz]: Blad msgsnd - pacjent do domu\n");
                continue;
            }
        }
        free(pozostali); // Zwalniam pamiec
    }
}

