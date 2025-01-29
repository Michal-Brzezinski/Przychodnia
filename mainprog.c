
/*
PLIK SKŁADA SIĘ Z NIEZBĘDNYCH INICJALIZACJI ZASOBÓW, BY PÓŹNIEJ EJ ZWOLNIĆ, Z DEFINICJI FUNKCJI OBSŁUGI SYGNAŁÓW,
GENEROWANIA PROCESÓW POTOMNYCH - LEKARZY, PACJENTÓW I REJESTRACJI 
*/


#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/errno.h>
#include <signal.h>

#ifndef SA_RESTART
#define SA_RESTART 0x10000000   
#endif
// w razie braku definicji w systemie (np. u mnie się jakieś błędy pojawiały)
#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0x00000001 
#endif

#include <time.h>
#include <unistd.h> // Dodany nagłówek dla fork() i execl() 
#include <sys/wait.h>
#include <signal.h>

#include "MyLib/sem_utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/dekoratory.h"
#include "MyLib/shm_utils.h"

#define S 5     // ilosc semaforow w zbiorze - w razie potrzeby zwiększyć
#define BUILDING_MAX 10     // maksymalna pojemność pacjentów w budynku 
#define MAX_GENERATE 15   // maksymalna liczba procesów pacjentów do wygenerowania
#define PAM_SIZE 7 // Rozmiar tablicy pamieci wspoldzielonej
// struktura pamieci wspoldzielonej
// pamiec_wspoldzielona[0] - wspolny licznik pacjentow
// pamiec_wspoldzielona[1-5] - limity pacjentow dla lekarzy
// pamiec_wspoldzielona[6] - licznik procesów, które zapisały do pamięci dzielonej

volatile sig_atomic_t keep_generating = 1;
pid_t generator_pacjentow_pid = -1;
pid_t generator_lekarzy_pid = -1;
pid_t rejestracja_pid = -1;

int shmID;  // id pamięci współdzielonej
int semID;  // id zbioru semaforów
int msg_id; // id 1. kolejki POZ
int msg_id1; // id 2. kolejki KARDIOLOGA
int msg_id2; // id 3. kolejki OKULISTY
int msg_id3; // id 4. kolejki PEDIATRY
int msg_id4; // id 5. kolejki PEDIATRY
int msg_id5; // id 6. kolejki LEKARZA MEDYCYNY PRACY
key_t klucz_wejscia; // klucz do semafora panującego nad ilością pacjentów w budynku
key_t shm_key; // klucz do pamięci współdzielonej
key_t msg_key; // klucz do kolejki komunikatów do rejestracji
key_t msg_key1; // klucz do kolejki do POZ
key_t msg_key2; // klucz do kolejki do KARDIOLOGA
key_t msg_key3; // klucz do kolejki do OKULISTY
key_t msg_key4; // klucz do kolejki do PEDIATRY
key_t msg_key5; // klucz do kolejki do LEKARZA MEDYCYNY PRACY



/*  --------------   FUNKCJE OBSLUGI SYGNAŁÓW   -------------    */
void handle_sigchld(int sig) {
    // Obsługa zakończenia procesów potomnych
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigint(int sig) {
    keep_generating = 0;

    // Zakończ procesy potomne
    if (generator_pacjentow_pid > 0) {
        kill(generator_pacjentow_pid, SIGTERM);
    }

    if (generator_lekarzy_pid > 0) {
      kill(generator_lekarzy_pid, SIGTERM);
    }

    if (rejestracja_pid > 0) {
        kill(rejestracja_pid, SIGTERM);
    }

    // Zwolnij zasoby IPC
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(msg_key);
    zwolnijPamiecWspoldzielona(shm_key);
    zwolnijKolejkeKomunikatow(msg_key1);
    zwolnijKolejkeKomunikatow(msg_key2);
    zwolnijKolejkeKomunikatow(msg_key3);
    zwolnijKolejkeKomunikatow(msg_key4);
    zwolnijKolejkeKomunikatow(msg_key5);
    system("bash czystka.sh");

    printRed("\n[Main]: Zakończono program po otrzymaniu SIGINT.");
    exit(0);
}


int main(){

    //  --------------------  INICJALIZACJA  -----------------   

    int i;  // zmienna iteracyjna 

    klucz_wejscia =  generuj_klucz_ftok(".", 'A');   // do semafora panującego nad ilością pacjentów w budynku
    semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0600);

    shm_key = generuj_klucz_ftok(".",'X');
    shmID = alokujPamiecWspoldzielona(shm_key, PAM_SIZE * sizeof(int), IPC_CREAT | IPC_EXCL | 0600);

    msg_key = generuj_klucz_ftok(".",'B');
    msg_id = alokujKolejkeKomunikatow(msg_key,IPC_CREAT | IPC_EXCL | 0600);

    msg_key1 = generuj_klucz_ftok(".",1);
    msg_id1 = alokujKolejkeKomunikatow(msg_key1,IPC_CREAT | IPC_EXCL | 0600);

    msg_key2 = generuj_klucz_ftok(".",2);
    msg_id2 = alokujKolejkeKomunikatow(msg_key2,IPC_CREAT | IPC_EXCL | 0600);
    
    msg_key3 = generuj_klucz_ftok(".",3);
    msg_id3 = alokujKolejkeKomunikatow(msg_key3,IPC_CREAT | IPC_EXCL | 0600);
    
    msg_key4 = generuj_klucz_ftok(".",4);
    msg_id4 = alokujKolejkeKomunikatow(msg_key4,IPC_CREAT | IPC_EXCL | 0600);
    
    msg_key5 = generuj_klucz_ftok(".",5);
    msg_id5 = alokujKolejkeKomunikatow(msg_key5,IPC_CREAT | IPC_EXCL | 0600);

    inicjalizujSemafor(semID,0,BUILDING_MAX); // semafor zainicjalizowany na maksymalną liczbe pacjentów w budynku
    inicjalizujSemafor(semID,1,0);  //potrzebny, aby proces czekał na potwierdzenie przyjęcia
    inicjalizujSemafor(semID,2,1);  // semafor mówiący, że rejestracja jest zamknięta
    inicjalizujSemafor(semID,3,1);  // semafor zapisu do pamieci wspoldzielonej
    inicjalizujSemafor(semID,4,0);  // semafor do odczytu z pamieci wspoldzielonej
    int limit_pacjentow = losuj_int(MAX_GENERATE/2)+MAX_GENERATE/2; // Losowy limit pacjentów dla wszystkich lekarzy
    char arg2[10];    // arg2 to limit pacjentów dla wszystkich lekarzy - używany jako argument w execl
    sprintf(arg2, "%d", limit_pacjentow);   // Konwersja liczby na ciąg znaków


    //  -------------------   OBSŁUGA SYGNAŁÓW    --------------------------

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
        perror("[Main]: sigaction");
        exit(1);
    }


    //---------------------------   PACJENCI     -------------------------------

    // Utwórz proces potomny do generowania pacjentów
    // Aby proces rodzic mógł wykonywać inne zadania

    generator_pacjentow_pid = fork();
    if (generator_pacjentow_pid == -1) {
        perror("[Main]: Błąd fork dla generatora pacjentów\n");
        exit(2);
    } else if (generator_pacjentow_pid == 0) {
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
            //sleep(2); // Losowe opóźnienie 0-6 sekund 
        }
        exit(0); // Zakończ proces potomny po wygenerowaniu pacjentów
    }

    //-----------------------   REJESTRACJA  -------------------------------

    switch (fork()) {
        case -1:
            perror("[Main]: Błąd fork dla rejestracji\n");
            // obsługa błędu - powiadomienie innych procesów (które utworzyły się poprawnie) o błędzie (np. sygnały)
            exit(2);
        case 0:
            print_fflush("[Main]: Uruchamianie rejestracji...");
            execl("./rejestracja", "rejestracja", arg2, NULL);
            // usuwanie wszystkich procesów i zasobów ipc - zaimplementować
            perror("[Main]: Błąd execl dla rejestracji\n");
            exit(3);
        default:
            break;  // Kontynuuj generowanie pacjentów
    }


    /*--------------------------    LEKARZE   ---------------------------------------*/

        generator_lekarzy_pid = fork();
    if (generator_lekarzy_pid == -1) {
        perror("[Main]: Błąd fork dla generatora lekarzy\n");
        exit(2);
    } else if (generator_lekarzy_pid == 0) {
        // Proces potomny: generowanie lekarzy
        // Teoretycznie ma ten sam handler zakończenia procesów dzieci
        printf("[Main]: Maksymalna liczba pacjentów do przyjęcia to %d\n", limit_pacjentow);
        char arg1[2];   // arg1 to id lekarza

        for (i = 1; i < 6; i++) {
            sprintf(arg1, "%d", i); // Konwersja liczby na ciąg znaków
            
            pid_t pid = fork();
            if (pid == -1) {
                perror("[Main]: Błąd fork (generator lekarzy) - próbuj generować lekarzy dalej\n");
                break;
            } else if (pid == 0) {
                execl("./lekarz", "lekarz", arg1, arg2, NULL);
                // 1. argument to id lekarza, 2. argument to limit pacjentów dla wszystkich lekarzy losowo wygenerowany
                perror("[Main]: Błąd execl dla lekarza\n");
                exit(2);
            } 
            // Proces rodzic: sprawdź zakończenie procesu potomnego bez blokowania
            while (waitpid(-1, NULL, WNOHANG) > 0);
            // sprawdza, czy jakikolwiek proces potomny zakończył się, ale nie blokuje, 
            // jeśli żaden proces nie jest gotowy do zakończenia - flaga WNOHANG
            //sleep(2); // Losowe opóźnienie 
        }
        
        exit(0); // Zakończ proces potomny po wygenerowaniu pacjentów
    }
    

    /*  -----------   OBSŁUGA POMYŚLNEGO ZAKOŃCZENIA PROGRAMU    ----------- */
    /*   ---------- (ZWOLNIENIE ZASOBÓW I CZEKANIE NA PROCESY) -----------   */

    // Czekaj na zakończenie procesu generowania lekarzy, jeśli nie został zakończony wcześniej
    int status;
    waitpid(generator_lekarzy_pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[Main]: Proces generowania lekarzy zakończony z kodem %d.\n", WEXITSTATUS(status));
        fflush(stdout);
    } else {
        printf("[Main]: Proces generowania lekarzy  zakończony niepowodzeniem.\n");
        fflush(stdout);
    }

    // Czekaj na zakończenie procesu generowania pacjentów, jeśli nie został zakończony wcześniej
    status =0;
    waitpid(generator_pacjentow_pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[Main]: Proces generowania pacjentów zakończony z kodem %d.\n", WEXITSTATUS(status));
        fflush(stdout);
    } else {
        printf("[Main]: Proces generowania pacjentów  zakończony niepowodzeniem.\n");
        fflush(stdout);
    }

    // Czekaj na zakończenie procesu rejestracja, jeśli nie został zakończony wcześniej
    status=0;
    waitpid(rejestracja_pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[Main]: Proces rejestracji zakończony z kodem %d.\n", WEXITSTATUS(status));
        fflush(stdout);
    } else {
        printf("[Main]: Proces rejestracji zakończony niepowodzeniem.\n");
        fflush(stdout);
    }

    system("killall pacjent"); 
    // Zakoncz wszystkie procesy pacjentów, które nie zdążyły się zakończyć, po zakończeniu generatora pacjentów
    // i zakończeniu rejestracji, aby nie prosiły o nieistniejące już zasoby
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(msg_key);
    zwolnijPamiecWspoldzielona(shm_key);
    zwolnijKolejkeKomunikatow(msg_key1);
    zwolnijKolejkeKomunikatow(msg_key2);
    zwolnijKolejkeKomunikatow(msg_key3);
    zwolnijKolejkeKomunikatow(msg_key4);
    zwolnijKolejkeKomunikatow(msg_key5);

    system("bash czystka.sh");

    return 0;
}