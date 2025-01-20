#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include "operacje.h"

#define S 1 // ilosc semaforow w zbiorze


int main(){

    srand(time(0) ^ getpid()); // Inicjalizacja generatora liczb losowych

    // ----------- inicjalizacja wartości struktury pacjenta
    Pacjent pacjent;
    pacjent.id_pacjent = getpid();
    
    int pomocnicza_vip = rand() % 100;
    if (pomocnicza_vip < 10)  pacjent.vip = 1; // 10% szans  
    else pacjent.vip  = 0; // 90% szans 
    
    pacjent.wiek = rand() % 100 + 1; // Wiek od 1 do 100 lat

    pacjent.id_lekarz = rand() % 5;


    // _______________________________  DOSTANIE SIĘ DO BUDYNKU     _______________________________________


    key_t klucz_wejscia;    // do semafora panującego nad ilością pacjentów w budynku
    if ( (klucz_wejscia = ftok(".", 'A')) == -1 )
    {
      printf("Blad ftok (main)\n");
      exit(1);
    }

    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0666);

    printf("Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    sleep(rand() % 3 + 1); // Losowe opóźnienie 1-3 sekundy
    /* SYMULACJA CZASU POTRZEBNEGO NA WEJŚCIE DO BUDYNKU */
    
    waitSemafor(semID, 0, 0);
    printf("Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    sleep(5); // opóźnienie 10 sekund w budynku


    /*  ________________________________    KOMUNIKACJA Z REJESTRACJĄ   __________________________________________*/

    key_t msg_key;
    int msg_id;

    // Utwórz klucz do kolejki komunikatów
    if ((msg_key = ftok(".", 'B')) == -1) {
        perror("Błąd ftok");
        exit(1);
    }

    // Otwórz kolejkę komunikatów
    if ((msg_id = msgget(msg_key, IPC_CREAT | 0666)) == -1) {
        perror("Błąd msgget - pacjent");
        exit(2);
    }

    // Wypełnij strukturę wiadomości
    Wiadomosc msg;
    msg.mtype = (pacjent.vip == 1)+1; // przypisanie typu komunikatu na podstawie bycia vipem 
    msg.id_pacjent = getpid();
    msg.vip = pacjent.vip;
    msg.wiek = pacjent.wiek;
    msg.id_lekarz = pacjent.id_lekarz;

    // Wyślij wiadomość do rejestracji
    if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("Błąd msgsnd - pacjent");
        exit(1);
    }

    // Pacjent oczekuje na rejestrację
    printf("Pacjent %d czeka na rejestrację.\n", msg.id_pacjent);


    /*  =======================================================================================   */


    signalSemafor(semID, 0);
    printf("Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);


    return 0;
}