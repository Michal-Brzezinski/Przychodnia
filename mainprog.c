#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/errno.h>
#include <time.h>
#include <unistd.h> // Dodany nagłówek dla fork() i execl() 
#include <sys/wait.h> // Dodany nagłówek dla wait() #include "operacje.h" 
#include <signal.h>

#include "MyLib/sem_utils.h"
#define S 2     // ilosc semaforow w zbiorze - w razie potrzeby zwiększyć
#define BUILDING_MAX 10     // maksymalna pojemność pacjentów w budynku 
#define MAX_GENERATE 100    // maksymalna liczba procesów pacjentów do wygenerowania

volatile sig_atomic_t keep_generating = 1;

void handle_sigint(int sig) {
    
    system("fish czystka.sh");
    keep_generating = 0;
}


int main(){

    int i;  // zmienna iteracyjna 
    key_t klucz_wejscia;    // do semafora panującego nad ilością pacjentów w budynku
    if ( (klucz_wejscia = ftok(".", 'A')) == -1 )
    {
      printf("Blad ftok (main)\n");
      exit(1);
    }
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0666);

    inicjalizujSemafor(semID,0,BUILDING_MAX);
    inicjalizujSemafor(semID,1,0);  //potrzebny, aby proces czekał na potwierdzenie przyjęcia
    
    // semafor zainicjalizowany na maksymalną liczbe pacjentów w budynku

    printf("Semafory gotowe!\n");
    fflush(stdout);

    // Seed the random number generator
    srand(time(NULL));

    // Ustawienie obsługi sygnału SIGINT
    signal(SIGINT, handle_sigint);


    // GENEROWANIE PACJENTÓW
    for (i = 0; i < MAX_GENERATE && keep_generating; i++) {
        // ostateczna wersja to generowanie aż do otrzymania sygnału zakończenia np. sigterm
        switch (fork()) {
            case -1:
                perror("Błąd fork (mainprog) - próbuj generować pacjentów dalej");
                // ignoruj błąd i kontynuuj generowanie pacjentów
                break;
            case 0:
                execl("./pacjent", "pacjent", NULL);
                perror("Błąd execl dla pacjenta");
                exit(3);
        }

        sleep(rand() % 3); // Losowe opóźnienie 0-3 sekund 
    }
/*
    pid_t rejestracja_pid = fork();
    switch (rejestracja_pid) {
        case -1:
            perror("Błąd fork dla rejestracji");
            // obsługa błędu - powiadomienie innych procesów (które utworzyły się poprawnie) o błędzie (np. sygnały)
            exit(2);
        case 0:
            printf("Uruchamianie rejestracji...\n");
            fflush(stdout);
            execl("./rejestracja", "rejestracja", NULL);
            // usuwanie wszystkich procesów i zasobów ipc - zaimplementować
            perror("Błąd execl dla rejestracji");
            exit(3);
        default:
            break;  // Kontynuuj generowanie pacjentów
    }
*/

    // Czekaj na zakończenie generowania pacjentów
    while (keep_generating) {
        pause(); // Czekaj na sygnał
    }

    printf("\nZakończono generowanie pacjentów po otrzymaniu SIGINT.\n");
    return 0;
}