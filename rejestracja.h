#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>  

#define S 2     // ilosc semaforow w zbiorze - w razie potrzeby zwiększyć
#define BUILDING_MAX 3 // Maksymalna liczba pacjentów w budynku
#define K (BUILDING_MAX / 2)
#define ZAMKNIECIE_OKIENKA2 (BUILDING_MAX / 3)

typedef struct {
    long mtype;       // Typ wiadomości
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza  
} Wiadomosc;
//  struktura wiadomości w rejestracji

void *obsluz_pacjentow(void *arg);