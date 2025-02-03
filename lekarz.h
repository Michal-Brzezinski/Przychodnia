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
#include "MyLib/shm_utils.h"

#define PAM_SIZE 7 // Rozmiar tablicy pamieci wspoldzielonej
// struktura pamieci wspoldzielonej
// pamiec_wspoldzielona[0] - wspolny licznik pacjentow
// pamiec_wspoldzielona[1-5] - limity pacjentow dla lekarzy
// pamiec_wspoldzielona[6] - licznik procesow, ktore zapisaly do pamieci dzielonej
#define S 5 // Ilosc semaforow w zbiorze

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


void czynnosci_lekarskie(Lekarz *lekarz);
void wyslij_do_specjalisty(Wiadomosc *msg, Lekarz *lekarz);
void badania_ambulatoryjne(Wiadomosc *msg, Lekarz *lekarz);

// FUNKCJE Z REJESTRACJA.C POTRZEBNE DO OBSLUGI PACJNTOW W KOLEJCE PO ZAMKNIECIU PRZYCHODNI


// Funkcja pomocnicza do obliczania liczby procesow w kolejce
int policzProcesy(int msg_id) {
    /* Zlicz liczbe komunikatow w kolejce (w prostym przypadku nieopoznionym) */
    
    int liczba_procesow = 0;
    struct msqid_ds buf;
    if (msgctl(msg_id, IPC_STAT, &buf) == -1) {
        perror_red("[policzProcesy]: msgctl IPC_STAT\n");
        return -1;
    }
    liczba_procesow = buf.msg_qnum;
    return liczba_procesow;
}


int *wypiszPacjentowWKolejce(int msg_id, int sem_id, int *rozmiar_kolejki, Lekarz *lekarz) {
    Wiadomosc msg;
    int *pacjenci_po_zamknieciu_pid;
    int rozmiar = policzProcesy(msg_id);
    *rozmiar_kolejki = rozmiar; // dzieki wskaznikowi mozna przeniesc rozmiar kolejki poza funkcje 
    pacjenci_po_zamknieciu_pid = (int *)(malloc(rozmiar * sizeof(int)));
    if(pacjenci_po_zamknieciu_pid == NULL)
    {
        perror_red("[wypiszPacjentowWKolejce]: malloc error\n");
        exit(1);
    }
    printf("Pacjenci ktorzy zostali przyjeci przez %s po zamknieciu:\n", lekarz->nazwa);
    int i=0; // zmienna do iteracji
    while (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) != -1) {
        printf("Pacjent nr %d, wiek: %d, vip: %d\n", msg.id_pacjent, msg.wiek, msg.vip);
        pacjenci_po_zamknieciu_pid[i]=msg.id_pacjent;
        i++;
    }
    if (errno != ENOMSG) {
        perror_red("[wypiszPacjentowWKolejce]: Blad msgrcv\n");
    }
    return pacjenci_po_zamknieciu_pid;
}