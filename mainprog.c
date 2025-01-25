#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/errno.h>
#include <time.h>
#include <unistd.h> // Dodany nagłówek dla fork() i execl() 
#include <sys/wait.h>
#include <signal.h>

#include "MyLib/sem_utils.h"
#include "MyLib/dekoratory.h"

#define S 3     // ilosc semaforow w zbiorze - w razie potrzeby zwiększyć
#define BUILDING_MAX 10     // maksymalna pojemność pacjentów w budynku 
#define MAX_GENERATE 100    // maksymalna liczba procesów pacjentów do wygenerowania

volatile sig_atomic_t keep_generating = 1;
pid_t generator_pid = -1;
pid_t rejestracja_pid = -1;

void handle_sigchld(int sig) {
    // Obsługa zakończenia procesów potomnych
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigint(int sig) {
    keep_generating = 0;

    // Zakończ procesy potomne
    if (generator_pid > 0) {
        kill(generator_pid, SIGTERM);
    }
    if (rejestracja_pid > 0) {
        kill(rejestracja_pid, SIGTERM);
    }

    // Zwolnij zasoby IPC
    system("fish czystka.sh");

    printRed("\n[Main]: Zakończono generowanie pacjentów po otrzymaniu SIGINT.");
    exit(0);
}


int main(){

    int i;  // zmienna iteracyjna 
    key_t klucz_wejscia =  generuj_klucz_ftok(".", 'A');   // do semafora panującego nad ilością pacjentów w budynku
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0666);

    inicjalizujSemafor(semID,0,BUILDING_MAX); // semafor zainicjalizowany na maksymalną liczbe pacjentów w budynku
    inicjalizujSemafor(semID,1,0);  //potrzebny, aby proces czekał na potwierdzenie przyjęcia
    inicjalizujSemafor(semID,2,1);  // semafor mówiący, że rejestracja jest zamknięta
    

    printGreen("[Main]: Semafory gotowe!");

    // Seed the random number generator
    srand(time(NULL));

    // Ustawienie obsługi sygnału SIGINT
    signal(SIGINT, handle_sigint);

    // Ustawienie obsługi sygnału SIGCHLD, by zapobiec zombiakom
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    // SA_NONLDSTOP - nie wysyłać sygnału SIGCHLD, gdy dziecko zatrzyma się
    // SA_RESTART - automatycznie wznowić przerwane wywołania systemowe
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }


    // GENEROWANIE PACJENTÓW

    // Utwórz proces potomny do generowania pacjentów
    // Aby proces rodzic mógł wykonywać inne zadania

    generator_pid = fork();
    if (generator_pid == -1) {
        perror("[Main]: Błąd fork dla generatora pacjentów\n");
        exit(2);
    } else if (generator_pid == 0) {
        // Proces potomny: generowanie pacjentów
        //Teoretycznie ma ten sam handler zakończenia procesów dzieci
        for (i = 0; i < MAX_GENERATE && keep_generating; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("[Main]: Błąd fork (generator pacjentów) - próbuj generować pacjentów dalej\n");
                break;
            } else if (pid == 0) {
                execl("./pacjent", "pacjent", NULL);
                perror("[Main]: Błąd execl dla pacjenta\n");
                exit(2);
            } 
            // Proces rodzic: sprawdź zakończenie procesu potomnego bez blokowania
            while (waitpid(-1, NULL, WNOHANG) > 0);
            //prawdza, czy jakikolwiek proces potomny zakończył się, ale nie blokuje, 
            //jeśli żaden proces nie jest gotowy do zakończenia - flaga WNOHANG
            sleep(2); // Losowe opóźnienie 0-6 sekund 
        }
        exit(0); // Zakończ proces potomny po wygenerowaniu pacjentów
    }

    switch (fork()) {
        case -1:
            perror("[Main]: Błąd fork dla rejestracji\n");
            // obsługa błędu - powiadomienie innych procesów (które utworzyły się poprawnie) o błędzie (np. sygnały)
            exit(2);
        case 0:
            print_fflush("[Main]: Uruchamianie rejestracji...");
            execl("./rejestracja", "rejestracja", NULL);
            // usuwanie wszystkich procesów i zasobów ipc - zaimplementować
            perror("[Main]: Błąd execl dla rejestracji\n");
            exit(3);
        default:
            break;  // Kontynuuj generowanie pacjentów
    }


    printGreen("[Main]: Wszystko się udało :)\n");

    // Czekaj na zakończenie generowania pacjentów
    while (keep_generating) {
        pause(); // Czekaj na sygnał
    }

    // Czekaj na zakończenie procesu rejestracja
    int status;
    waitpid(rejestracja_pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[Main]: Proces rejestracji zakończony z kodem %d.\n", WEXITSTATUS(status));
        fflush(stdout);
    } else {
        printf("[Main]: Proces rejestracji zakończony niepowodzeniem.\n");
        fflush(stdout);
    }
    //nie czekam na zakończenie generatora pacjentów, bo on zakończył się sam po SIGINT

    printRed("\n[Main]: Zakończono generowanie pacjentów po otrzymaniu SIGINT.");
    return 0;
}