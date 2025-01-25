#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>

#define S 3 // ilosc semaforow w zbiorze

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

    int pomocnicza_vip = losuj_int(100);
    if (pomocnicza_vip < 20)  pacjent->vip = 1; // 20% szans
    else pacjent->vip  = 0; // 80% szans

    pacjent->wiek = losuj_int(100); // Wiek 0-100 lat

    pacjent->id_lekarz = losuj_int(4)+1; // id od 1-5

}

void inicjalizujWiadomosc(Wiadomosc *msg, Pacjent *pacjent){

    msg->mtype = (pacjent->vip == 1)+1; // przypisanie typu komunikatu na podstawie bycia vipem
    msg->id_pacjent = getpid();
    msg->vip = pacjent->vip;
    msg->wiek = pacjent->wiek;
    msg->id_lekarz = pacjent->id_lekarz;

}