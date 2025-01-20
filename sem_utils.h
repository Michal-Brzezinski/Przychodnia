#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>


typedef struct {
    int id_pacjent; // numer pacjenta - pid
    int vip; // 1 jeśli VIP, 0 jeśli nie
    int wiek;    // Wiek pacjenta
    int id_lekarz;  // numer lekarza, do którego pacjent chce się udać
} Pacjent;
//  struktura pacjenta 

typedef struct {
    long mtype;       // Typ wiadomości
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza  
} Wiadomosc;

int alokujSemafor(key_t klucz, int number, int flagi);
void inicjalizujSemafor(int semID, int number, int val);
int zwolnijSemafor(int semID, int number);
int waitSemafor(int semID, int number, int flags);
void signalSemafor(int semID, int number);
int valueSemafor(int semID, int number);

