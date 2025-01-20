#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <unistd.h> // Dodany nagłówek dla fork() i execl() 
#include <sys/wait.h> // Dodany nagłówek dla wait() #include "operacje.h" 
#include "operacje.h"
#define S 2     // ilosc semaforow w zbiorze - w razie potrzeby zwiększyć
#define BUILDING_MAX 3     // maksymalna pojemność pacjentów w budynku 
#define MAX_GENERATE 12    // maksymalna liczba procesów pacjentów do wygenerowania

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
    inicjalizujSemafor(semID,1,0);
    // semafor zainicjalizowany na maksymalną liczbe pacjentów w budynku

    printf("Semafory gotowe!\n");
    fflush(stdout);

    // GENEROWANIE PACJENTÓW
    for (i = 0; i < MAX_GENERATE; i++){
      switch (fork())
      {
         case -1:
            perror("Błąd fork (mainprog)");
            zwolnijSemafor(semID, S);
            exit(2);
         case 0:
            execl("./pacjent", "pacjent", NULL);
      }

      sleep(rand() % 3); // Losowe opóźnienie 0-3 sekund 

    }

    switch (fork()) {
    case -1:
        perror("Błąd fork dla rejestracji");
        exit(2);
    case 0:
        execl("./rejestracja", "rejestracja", NULL);
        perror("Błąd execl dla rejestracji");
        exit(3);
}

    for (i = 0; i < MAX_GENERATE + 1 ; i++) // + 1 bo jeszcze proces rejestracji
      wait( (int *)NULL );

    zwolnijSemafor(semID, S);
    printf("MAIN: Koniec.\n");


    return 0;
}