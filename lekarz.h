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

#include "MyLib/dekoratory.h"
#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"

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

volatile int zakoncz_program = 0;

int sem_id, msg_id_lekarz, msg_id_rejestracja;
key_t klucz_sem, klucz_kolejki_lekarza, klucz_kolejki_rejestracji;

key_t klucz_wyjscia;
int msg_id_wyjscie;

int limit_POZ2; // globalne do ulatwienia dzialania programu
int limity_lekarzy[5];

//------------------------  FUNKCJE DEKLAROWANE W HEADERZE  --------------------------------

void *lekarzPOZ2(void* _arg);
void obsluga_SIGINT(int sig);
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

int *wypiszPacjentowWKolejce(int msg_id, int *rozmiar_kolejki, Lekarz *lekarz) {
    Wiadomosc msg;
    int *pacjenci_po_zamknieciu_pid;
    int rozmiar = policzProcesy(msg_id);
    
    if (rozmiar == 0) {
        // W RAZIE PUSTEJ KOLEJKI NA KONIEC NIE ROB NIC
        *rozmiar_kolejki = 0;
        return NULL;
    }

    *rozmiar_kolejki = rozmiar; // dzieki wskaznikowi mozna przeniesc rozmiar kolejki poza funkcje 
    pacjenci_po_zamknieciu_pid = (int *)(malloc(rozmiar * sizeof(int)));
    if(pacjenci_po_zamknieciu_pid == NULL)
    {
        perror_red("[wypiszPacjentowWKolejceLekarza]: malloc error\n");
        exit(1);
    }
    
    printMagenta("[%s]: Po zamknieciu przychodni obsluzono pacjentow:\n", lekarz->nazwa);
    
    int i=0; // zmienna do iteracji po tablicy zaalokowanej
    while (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) != -1) {
        
        print("[%s]: Pacjent nr %d, wiek: %d, vip: %d\n",lekarz->nazwa, msg.id_pacjent, msg.wiek, msg.vip);
        pacjenci_po_zamknieciu_pid[i]=msg.id_pacjent;
        i++;
    }
    if (errno != ENOMSG) {
        perror_red("[wypiszPacjentowWKolejceLekarza]: Blad msgrcv\n");
    }

    return pacjenci_po_zamknieciu_pid;
}


void wypiszIOdeslijPacjentow(Lekarz *lekarz, int msg_id){

    int rozmiar_pozostalych = 0;
    int *pidy_pozostalych = wypiszPacjentowWKolejce(msg_id_lekarz, &rozmiar_pozostalych, lekarz);
    int i; // zmienna iteracyjna

    // wysylanie pozostalym pacjentom komunikatu o wyjsciu
    for(i=0;i<rozmiar_pozostalych;i++){    
        Wiadomosc msg1;
        msg1.mtype = pidy_pozostalych[i];
        msg1.id_pacjent = pidy_pozostalych[i];
        // Wyslij pacjenta do domu
        if (msgsnd(msg_id_wyjscie, &msg1, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
            perror_red("[Lekarz]: Blad msgsnd - pacjent do domu\n");
            continue;
        }
    }
    if (pidy_pozostalych != NULL) {
        free(pidy_pozostalych); 
        // zwalniam, poniewaz wypisz pacjentow dynamicznie alokuje tablice pidow, ktora trzeba zwolnic
    }

}