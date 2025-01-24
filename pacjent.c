#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>

#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"
#include "MyLib/dekoratory.h"

#define S 2 // ilosc semaforow w zbiorze

void inicjalizujWiadomosc(Wiadomosc *msg, Pacjent *pacjent);
void inicjalizujPacjenta(Pacjent *pacjent);

int main(){

    // ----------- inicjalizacja wartości struktury pacjenta
    Pacjent pacjent;
    inicjalizujPacjenta(&pacjent);

    // _______________________________  DOSTANIE SIĘ DO BUDYNKU     _______________________________________


    key_t klucz_wejscia = generuj_klucz_ftok(".",'A');

    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    printf("Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    sleep(losuj_int(3));    /* SYMULACJA CZASU POTRZEBNEGO NA WEJŚCIE DO BUDYNKU */

    waitSemafor(semID, 0, 0);   /* SPRAWDZA CZY MOŻE WEJŚĆ DO BUDYNKU BAZUJĄC NA SEMAFORZE*/
    printf("Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    sleep(5); // opóźnienie 5 sekund w budynku


    /*  ________________________________    KOMUNIKACJA Z REJESTRACJĄ   __________________________________________*/

    key_t msg_key = generuj_klucz_ftok(".",'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key,IPC_CREAT | 0600);

    Wiadomosc msg;
    inicjalizujWiadomosc(&msg, &pacjent);

    // Pacjent oczekuje na rejestrację
    printf("Pacjent %d czeka na rejestrację w kolejce.\n", msg.id_pacjent);

        // Wyślij wiadomość do rejestracji
    if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("Błąd msgsnd - pacjent");
        exit(1);
    }

    waitSemafor(semID, 1, 0);   // czeka aż przyjdzie komunikat

    /*  =======================================================================================   */


    signalSemafor(semID, 0);    // zwolnienie semafora wejścia do budynku 
    printf("Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);


    return 0;
}


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