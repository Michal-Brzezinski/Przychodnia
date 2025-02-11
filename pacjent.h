#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#ifndef SA_RESTART
#define SA_RESTART 0x10000000   
#endif
// w razie braku definicji w systemie (np. u mnie sie jakies bledy pojawialy)
#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0x00000001 
#endif
#include <semaphore.h> //semafory POSIX dla watkow
#include <pthread.h>

#define S 7 // ilosc semaforow w zbiorze

typedef struct {
    int id_pacjent; // numer pacjenta - pid
    int vip; // 1 jesli VIP, 0 jesli nie
    int wiek;    // Wiek pacjenta
    int id_lekarz;  // numer lekarza, do ktorego pacjent chce sie udac
    int czy_wszedl; // zmienna, ktora pomaga w kontroli do wyjscia pacjenta z budynku
    // przydaje sie gdy np. pacjent zdazyl wejsc do budynku, ale gdy chce wyjsc przychodnia juz zamknieta
} Pacjent;
//  struktura pacjenta 

typedef struct {
    long mtype;       // Typ wiadomosci
    int id_pacjent;   // Numer pacjenta
    int vip;          // Status VIP
    int wiek;         // Wiek pacjenta
    int id_lekarz;    // Numer preferowanego lekarza 
    char kto_skierowal[128]; // nazwa lekarza, ktory skierowal
} Wiadomosc;
//  struktura wiadomosci w rejestracji

Pacjent pacjent; //globalna zmienna do ulatiwenia obslugi SIGUSR2
int sem_id;

void inicjalizujPacjenta(Pacjent *pacjent){

    pacjent->id_pacjent = getpid(); // za ID pacjenta bedzie sluzyc PID procesu, ktory go wywoluje
    pacjent->czy_wszedl = 0;    // generujac pacjenta jest on poza budynkiem przychodni

    int pomocnicza_vip = losuj_int(100); // 0-99
    if (pomocnicza_vip < 20)  pacjent->vip = 1; // 20% szans
    else pacjent->vip  = 2; // 80% szans
    /*WARTO ZWROCIC UWAGE, ZE WARTOSC VIP BEDZIE 0 DLA PACJENTOW
    KTORZY WRACAJA Z BADAN AMBULATORYJNYCH*/

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

    switch(pacjent->id_lekarz){
        case 1:
            strcpy(msg->kto_skierowal, "Lekarz POZ");
            break;
        case 2:
            strcpy(msg->kto_skierowal, "Kardiolog");
            break;
        case 3:
            strcpy(msg->kto_skierowal, "Okulista");
            break;
        case 4:
            strcpy(msg->kto_skierowal, "Pediatra");
            break;
        case 5:
            strcpy(msg->kto_skierowal, "Lekarz Medycyny Pracy");
            break;
    }
}