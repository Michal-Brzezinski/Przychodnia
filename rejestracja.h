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
#include "MyLib/msg_utils.h"
#include "MyLib/dekoratory.h"
#include "MyLib/shm_utils.h"

#define BUILDING_MAX 10 // Maksymalna liczba pacjentów w budynku
#define S 5 // Ilość semaforów w zbiorze
#define PAM_SIZE 6 // Rozmiar tablicy pamieci wspoldzielonej

typedef struct {
    long mtype;       // Typ wiadomości
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza  
} Wiadomosc;
//  struktura wiadomości w rejestracji

// Funkcja pomocnicza do obliczania liczby procesów w kolejce
int policzProcesy(int msg_id) {
    /* Zlicz liczbę komunikatów w kolejce (w prostym przypadku nieopóźnionym) */
    
    int liczba_procesow = 0;
    struct msqid_ds buf;
    if (msgctl(msg_id, IPC_STAT, &buf) == -1) {
        perror("\033[1;31m[policzProcesy]: msgctl IPC_STAT\033[0m\n");
        exit(1);
    }
    liczba_procesow = buf.msg_qnum;
    return liczba_procesow;
}


void wypiszPacjentowWKolejce(int msg_id, int semID) {
    Wiadomosc msg;
    printf("Pacjenci w kolejce do rejestracji w momencie zamykania rejestracji:\n");
    while (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) != -1) {
        printf("Pacjent nr %d, wiek: %d, vip: %d\n", msg.id_pacjent, msg.wiek, msg.vip);
        // Informuj pacjenta, że może wyjść z budynku
        signalSemafor(semID, 1);
    }
    if (errno != ENOMSG) {
        perror("\033[1;31m[wypiszPacjentowWKolejce]: Błąd msgrcv\033[0m");
    }
}