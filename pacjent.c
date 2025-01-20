#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include "operacje.h"

#define S 1 // ilosc semaforow w zbiorze

typedef struct {
    int id_pacjent; // numer pacjenta - pid
    int vip; // 1 jeśli VIP, 0 jeśli nie
    int wiek;    // Wiek pacjenta
    int id_lekarz;  // numer lekarza, do którego pacjent chce się udać
} Pacjent;

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
    // -----------------------------------------------------

    key_t klucz_wejscia;    // do semafora panującego nad ilością pacjentów w budynku
    if ( (klucz_wejscia = ftok(".", 'A')) == -1 )
    {
      printf("Blad ftok (main)\n");
      exit(1);
    }

    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0666);

    printf("Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);
    waitSemafor(semID, 0, 0);
    printf("Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);
    int sleeptime = rand() % 5;
    sleep(sleeptime);
    signalSemafor(semID, 0);

    return 0;
}