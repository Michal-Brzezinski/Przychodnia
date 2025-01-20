#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <unistd.h> // Dodany nagłówek dla fork() i execl() 
#include <sys/wait.h> // Dodany nagłówek dla wait() #include "operacje.h" 
#include "operacje.h"
#define S 1     // ilosc semaforow w zbiorze - w razie potrzeby zwiększyć
#define BUILDING_MAX 4     // maksymalna pojemność pacjentów w budynku 
#define MAX_GENERATE 10    // maksymalna liczba procesów pacjentów do wygenerowania

int main(){

    int i;  // zmienna iteracyjna 
    key_t klucz_wejscia;    // do semafora panującego nad ilością pacjentów w budynku
    if ( (klucz_wejscia = ftok(".", 'A')) == -1 )
    {
      printf("Blad ftok (main)\n");
      exit(1);
    }
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0666);
    
    //for (int i = 0; i < S; i++)
    //inicjalizujSemafor(semID, i, 0); // inicjalizujemy zerami
    // W RAZIE WIĘKSZEJ ILOŚCI SEMAFORÓW

    inicjalizujSemafor(semID,0,BUILDING_MAX);
    // semafor zainicjalizowany na maksymalną liczbe pacjentów w budynku

    printf("Semafory gotowe!\n");
    fflush(stdout);

    // GENEROWANIE PACJENTÓW
    for (i = 0; i < MAX_GENERATE; i++)
      switch (fork())
      {
         case -1:
            perror("Blad fork (mainprog)");
            zwolnijSemafor(semID, S);
            exit(2);
         case 0:
            execl("./pacjent", "pacjent", NULL);
      }

    for (i = 0; i < MAX_GENERATE; i++)
      wait( (int *)NULL );

    zwolnijSemafor(semID, S);
    printf("MAIN: Koniec.\n");


    return 0;
}