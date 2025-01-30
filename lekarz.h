#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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