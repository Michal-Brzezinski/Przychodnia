#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#ifndef SA_RESTART
#define SA_RESTART 0x10000000   
#endif
// w razie braku definicji w systemie (np. u mnie się jakieś błędy pojawiały)
#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0x00000001 
#endif
#include <semaphore.h> //semafory POSIX dla wątków
#include <pthread.h>

#define S 5 // ilosc semaforow w zbiorze

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
//  struktura wiadomości w rejestracji

void inicjalizujPacjenta(Pacjent *pacjent){

    pacjent->id_pacjent = getpid();

    int pomocnicza_vip = losuj_int(100); // 0-99
    if (pomocnicza_vip < 20)  pacjent->vip = 1; // 20% szans
    else pacjent->vip  = 2; // 80% szans
    /*WARTO ZWRÓCIĆ UWAGĘ, ŻE WARTOŚĆ VIP BĘDZIE 0 DLA PACJENTÓW
    KTÓRZY WRACAJĄ Z BADAŃ AMBULATORYJNYCH*/

    pacjent->wiek = losuj_int(100); // Wiek 0-100 lat

    int pomocnicza_lekarz = losuj_int(100); // 0-99
    if (pomocnicza_lekarz < 60) pacjent->id_lekarz = 1; // 60% szans
    else if (pomocnicza_lekarz < 70) pacjent->id_lekarz = 2; // 10% szans
    else if (pomocnicza_lekarz < 80) pacjent->id_lekarz = 3; // 10% szans
    else if (pomocnicza_lekarz < 90) pacjent->id_lekarz = 4; // 10% szans
    else pacjent->id_lekarz = 5; // 10% szans

}

void inicjalizujWiadomosc(Wiadomosc *msg, Pacjent *pacjent){

    msg->mtype = (pacjent->vip == 1)+1; // przypisanie typu komunikatu na podstawie bycia vipem
    msg->id_pacjent = getpid();
    msg->vip = pacjent->vip;
    msg->wiek = pacjent->wiek;
    msg->id_lekarz = pacjent->id_lekarz;

}