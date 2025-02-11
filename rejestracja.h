#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <time.h>

#include "MyLib/sem_utils.h"
#include "MyLib/shm_utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/utils.h"

#define S 7 // Ilosc semaforow w zbiorze
#define PAM_SIZE 7      // Rozmiar tablicy pamieci wspoldzielonej

typedef struct {
    long mtype;       // Typ wiadomosci
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza  
    char kto_skierowal[128]; // nazwa lekarza, ktory skierowal
} Wiadomosc;
//  struktura wiadomosci w rejestracji

// Definicja typu wyliczeniowego dla lekarzy
enum lekarze{ 
    POZ = 1, 
    KARDIOLOG, 
    OKULISTA, 
    PEDIATRA, 
    MEDYCYNA_PRACY
};

volatile int running = 1; // Flaga do sygnalizowania zakonczenia

// Zmienna globalna do przechowywania pid procesu okienka nr 2
pid_t pid_okienka2 = -1;

int msg_id_rej; // id i klucz kolejki do rejestracji
key_t msg_key_rej;

int sem_id; // id  i klucz do zbioru semaforow
key_t sem_key;

key_t klucz_wyjscia;
int msg_id_wyjscie;

int shm_id; // id pamieci wspoldzielonej
int *pamiec_wspoldzielona;
key_t shm_key;        // klucz do pamieci wspoldzielonej

int msg_id_1; // id 1. kolejki POZ
int msg_id_2; // id 2. kolejki KARDIOLOGA
int msg_id_3; // id 3. kolejki OKULISTY
int msg_id_4; // id 4. kolejki PEDIATRY
int msg_id_5; // id 5. kolejki LEKARZA MEDYCYNY PRACY

key_t msg_key_1; // klucz do kolejki do POZ
key_t msg_key_2; // klucz do kolejki do KARDIOLOGA
key_t msg_key_3; // klucz do kolejki do OKULISTY
key_t msg_key_4; // klucz do kolejki do PEDIATRY
key_t msg_key_5; // klucz do kolejki do LEKARZA MEDYCYNY PRACY

int limity_lekarzy[5] = {0}; // Tablica przechowujaca limity pacjentow dla lekarzy
int limit_pacjentow;

// zmienne do operacji na czasie
int Tp, Tk; // czas poczatkowy i czas koncowy (czas dzialania rejestracji)

int building_max;

// funkcje potrzebne do obslugi rejestracji

void uruchomOkienkoNr2();
void zatrzymajOkienkoNr2();

//  Wersja funkcji dla rejestracji - rozni sie nieznacznie od tej w lekarz.h
Wiadomosc *wypiszPacjentowWKolejce(int msg_id, int semID, int *rozmiar_kolejki) {
// Funkcja w
    
    Wiadomosc msg;
    Wiadomosc *pacjenci_po_zamknieciu;
    int rozmiar = policzProcesy(msg_id);
    *rozmiar_kolejki = rozmiar; // dzieki wskaznikowi mozna przeniesc rozmiar kolejki poza funkcje 
    
    pacjenci_po_zamknieciu = (Wiadomosc *)(malloc(rozmiar * sizeof(Wiadomosc)));
    if(pacjenci_po_zamknieciu == NULL)
    {
        perror_red("[wypiszPacjentowWKolejceRejestracji]: malloc error\n");
        exit(1);
    }
    
    print("Pacjenci w kolejce do rejestracji w momencie zamykania rejestracji:\n");
    
    int i=0; // zmienna do iteracji
    while (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) != -1) {
        print("Pacjent nr %d, wiek: %d, vip: %d\n", msg.id_pacjent, msg.wiek, msg.vip);
        pacjenci_po_zamknieciu[i] = msg;
        i++;

    }
    if (errno != ENOMSG) {
        perror_red("[wypiszPacjentowWKolejceRejestracji]: Blad msgrcv\n");
    }
    return pacjenci_po_zamknieciu;
}