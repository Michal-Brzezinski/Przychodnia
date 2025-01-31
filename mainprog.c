
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
// w razie braku definicji w systemie (np. u mnie sie jakies bledy pojawialy)
#ifndef SA_NOCLDSTOP
#define SA_NOCLDSTOP 0x00000001 
#endif

#include <time.h>
#include <unistd.h> // Dodany naglowek dla fork() i execl() 
#include <sys/wait.h>
#include <signal.h>

#include "MyLib/sem_utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/dekoratory.h"
#include "MyLib/shm_utils.h"

#define S 5     // ilosc semaforow w zbiorze - w razie potrzeby zwiekszyc
#define BUILDING_MAX 4     // maksymalna pojemnosc pacjentow w budynku 
#define MAX_GENERATE 30   // maksymalna liczba procesow pacjentow do wygenerowania
#define PAM_SIZE 7 // Rozmiar tablicy pamieci wspoldzielonej
// struktura pamieci wspoldzielonej
// pamiec_wspoldzielona[0] - wspolny licznik pacjentow
// pamiec_wspoldzielona[1-5] - limity pacjentow dla lekarzy
// pamiec_wspoldzielona[6] - licznik procesow, ktore zapisaly do pamieci dzielonej

volatile sig_atomic_t keep_generating = 1;
pid_t generator_pacjentow_pid = -1;
pid_t generator_lekarzy_pid = -1;
pid_t rejestracja_pid = -1;

int shmID;  // id pamieci wspoldzielonej
int * pamiec_wspoldzielona;
int semID;  // id zbioru semaforow
int msg_id_rej; // id kolejki do rejestracji
int msg_id_POZ; // id 1. kolejki POZ
int msg_id_KARDIO; // id 2. kolejki KARDIOLOGA
int msg_id_OKUL; // id 3. kolejki OKULISTY
int msg_id_PED; // id 4. kolejki PEDIATRY
int msg_id_MP; // id 5. kolejki LEKARZA MEDYCYNY PRACY
key_t klucz_wejscia; // klucz do semafora panujacego nad iloscia pacjentow w budynku
key_t shm_key; // klucz do pamieci wspoldzielonej
key_t msg_key_rej; // klucz do kolejki do rejestracji
key_t msg_key_POZ; // klucz do kolejki do POZ
key_t msg_key_KARDIO; // klucz do kolejki do KARDIOLOGA
key_t msg_key_OKUL; // klucz do kolejki do OKULISTY
key_t msg_key_PED; // klucz do kolejki do PEDIATRY
key_t msg_key_MP; // klucz do kolejki do LEKARZA MEDYCYNY PRACY



/*  --------------   FUNKCJE OBSLUGI SYGNAŁÓW   -------------    */
void handle_sigchld(int sig) {
    // Obsluga zakonczenia procesow potomnych
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_sigint(int sig) {
    keep_generating = 0;

    // Zakoncz procesy potomne
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
    zwolnijKolejkeKomunikatow(msg_key_rej);
    zwolnijPamiecWspoldzielona(shm_key);
    zwolnijKolejkeKomunikatow(msg_key_POZ);
    zwolnijKolejkeKomunikatow(msg_key_KARDIO);
    zwolnijKolejkeKomunikatow(msg_key_OKUL);
    zwolnijKolejkeKomunikatow(msg_key_PED);
    zwolnijKolejkeKomunikatow(msg_key_MP);
    usunNiepotrzebnePliki();

    printRed("\n[Main]: Zakonczono program po otrzymaniu SIGINT.\n");
    exit(0);
}


int main(){

    //  --------------------  INICJALIZACJA  -----------------   

    int i;  // zmienna iteracyjna 

    klucz_wejscia =  generuj_klucz_ftok(".", 'A');   // do semafora panujacego nad iloscia pacjentow w budynku
    semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0600);

    shm_key = generuj_klucz_ftok(".",'X');
    shmID = alokujPamiecWspoldzielona(shm_key, PAM_SIZE * sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shmID,0);
    memset(pamiec_wspoldzielona, 0, PAM_SIZE * sizeof(int)); // Inicjalizacja wszystkich elementów pamięci na 0

    msg_key_rej = generuj_klucz_ftok(".",'B');
    msg_id_rej = alokujKolejkeKomunikatow(msg_key_rej,IPC_CREAT | IPC_EXCL | 0600);

    msg_key_POZ = generuj_klucz_ftok(".",1);
    msg_id_POZ = alokujKolejkeKomunikatow(msg_key_POZ,IPC_CREAT | IPC_EXCL | 0600);

    msg_key_KARDIO = generuj_klucz_ftok(".",2);
    msg_id_KARDIO = alokujKolejkeKomunikatow(msg_key_KARDIO,IPC_CREAT | IPC_EXCL | 0600);
    
    msg_key_OKUL = generuj_klucz_ftok(".",3);
    msg_id_OKUL = alokujKolejkeKomunikatow(msg_key_OKUL,IPC_CREAT | IPC_EXCL | 0600);
    
    msg_key_PED = generuj_klucz_ftok(".",4);
    msg_id_PED = alokujKolejkeKomunikatow(msg_key_PED,IPC_CREAT | IPC_EXCL | 0600);
    
    msg_key_MP = generuj_klucz_ftok(".",5);
    msg_id_MP = alokujKolejkeKomunikatow(msg_key_MP,IPC_CREAT | IPC_EXCL | 0600);

    inicjalizujSemafor(semID,0,BUILDING_MAX); // semafor zainicjalizowany na maksymalna liczbe pacjentow w budynku
    inicjalizujSemafor(semID,1,0);  //potrzebny, aby proces pacjenta czekal na potwierdzenie przyjecia
    inicjalizujSemafor(semID,2,1);  // semafor mowiacy, ze rejestracja jest zamknieta
    inicjalizujSemafor(semID,3,1);  // semafor dostepu do pamieci wspoldzielonej
    inicjalizujSemafor(semID,4,0);  // semafor do pracy z plikiem
    int limit_pacjentow = losuj_int(MAX_GENERATE/2)+MAX_GENERATE/2; // Losowy limit pacjentow dla wszystkich lekarzy
    char arg2[10];    // arg2 to limit pacjentow dla wszystkich lekarzy - uzywany jako argument w execl
    sprintf(arg2, "%d", limit_pacjentow);   // Konwersja liczby na ciag znakow


    //  -------------------   OBSŁUGA SYGNAŁÓW    --------------------------

    // Ustawienie obslugi sygnalu SIGINT
    signal(SIGINT, handle_sigint);

    // Ustawienie obslugi sygnalu SIGCHLD, by zapobiec zombiakom
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    // SA_NONLDSTOP - nie wysylac sygnalu SIGCHLD, gdy dziecko zatrzyma sie
    // SA_RESTART - automatycznie wznowic przerwane wywolania systemowe
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror_red("[Main]: sigaction\n");
        exit(1);
    }


 //-----------------------   REJESTRACJA  -------------------------------

    switch (fork()) {
        case -1:
            perror_red("[Main]: Blad fork dla rejestracji\n");
            // obsluga bledu - powiadomienie innych procesow (ktore utworzyly sie poprawnie) o bledzie (np. sygnaly)
            exit(2);
        case 0:
            print("[Main]: Uruchamianie rejestracji...\n");
            execl("./rejestracja", "rejestracja", arg2, NULL);
            // usuwanie wszystkich procesow i zasobow ipc - zaimplementowac
            perror_red("[Main]: Blad execl dla rejestracji\n");
            exit(3);
        default:
            break;  // Kontynuuj generowanie pacjentow
    }


    /*--------------------------    LEKARZE   ---------------------------------------*/

        generator_lekarzy_pid = fork();
    if (generator_lekarzy_pid == -1) {
        perror_red("[Main]: Blad fork dla generatora lekarzy\n");
        exit(4);
    } else if (generator_lekarzy_pid == 0) {
        // Proces potomny: generowanie lekarzy
        // Teoretycznie ma ten sam handler zakonczenia procesow dzieci
        print("[Main]: Maksymalna liczba pacjentow do przyjecia to %d\n", limit_pacjentow);
        char arg1[2];   // arg1 to id lekarza

        for (i = 1; i < 6; i++) {
            sprintf(arg1, "%d", i); // Konwersja liczby na ciag znakow
            
            pid_t pid = fork();
            if (pid == -1) {
                perror_red("[Main]: Blad fork (generator lekarzy) - probuj generowac lekarzy dalej\n");
                break;
            } else if (pid == 0) {
                execl("./lekarz", "lekarz", arg1, arg2, NULL);
                // 1. argument to id lekarza, 2. argument to limit pacjentow dla wszystkich lekarzy losowo wygenerowany
                perror_red("[Main]: Blad execl dla lekarza\n");
                exit(5);
            } 
            // Proces rodzic: sprawdz zakonczenie procesu potomnego bez blokowania
            while (waitpid(-1, NULL, WNOHANG) > 0);
            // sprawdza, czy jakikolwiek proces potomny zakonczyl sie, ale nie blokuje, 
            // jesli zaden proces nie jest gotowy do zakonczenia - flaga WNOHANG
        }
        
        exit(0); // Zakoncz proces potomny po wygenerowaniu pacjentow
    }

    //---------------------------   PACJENCI     -------------------------------

    // Utworz proces potomny do generowania pacjentow
    // Aby proces rodzic mogl wykonywac inne zadania

    generator_pacjentow_pid = fork();
    if (generator_pacjentow_pid == -1) {
        perror_red("[Main]: Blad fork dla generatora pacjentow\n");
        exit(2);
    } else if (generator_pacjentow_pid == 0) {
        // Proces potomny: generowanie pacjentow
        //Teoretycznie ma ten sam handler zakonczenia procesow dzieci
        for (i = 0; i < MAX_GENERATE && keep_generating; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                perror_red("[Main]: Blad fork (generator pacjentow) - probuj generowac pacjentow dalej\n");
                break;
            } else if (pid == 0) {
                execl("./pacjent", "pacjent", NULL);
                perror_red("[Main]: Blad execl dla pacjenta\n");
                exit(2);
            } 
            // Proces rodzic: sprawdz zakonczenie procesu potomnego bez blokowania
            while (waitpid(-1, NULL, WNOHANG) > 0);
            //prawdza, czy jakikolwiek proces potomny zakonczyl sie, ale nie blokuje, 
            //jesli zaden proces nie jest gotowy do zakonczenia - flaga WNOHANG
            sleep(1); // Opoznienien w generowaniu kolejnych pacjentow
        }
        while (waitpid(-1, NULL, WNOHANG) > 0);
        exit(0); // Zakoncz proces potomny po wygenerowaniu pacjentow
    }
    

    /*  -----------   OBSŁUGA POMYŚLNEGO ZAKOŃCZENIA PROGRAMU    ----------- */
    /*   ---------- (ZWOLNIENIE ZASOBÓW I CZEKANIE NA PROCESY) -----------   */

    // Czekaj na zakonczenie procesu generowania lekarzy, jesli nie zostal zakonczony wczesniej
    int status;
    waitpid(generator_lekarzy_pid, &status, 0);
    if (WIFEXITED(status)) 
        print("[Main]: Proces generowania lekarzy zakonczony z kodem %d.\n", WEXITSTATUS(status));
    else 
        print("[Main]: Proces generowania lekarzy  zakonczony niepowodzeniem.\n");
    

    pid_t child_pid1 = fork();
    if (child_pid1 == 0) { // Proces potomny
        // Czekaj na zakonczenie procesu generowania lekarzy, jesli nie zostal zakonczony wczesniej

        oczekujNaProces(rejestracja_pid, "rejestracja");
        exit(0); // Proces potomny kończy działanie
    }
    else if (child_pid1 < 0) perror_red("[Main]: Blad fork - czekanie na rejestracje\n");

    // Czekaj na zakonczenie procesu generowania pacjentow, jesli nie zostal zakonczony wczesniej
    status =0;
    waitpid(generator_pacjentow_pid, &status, 0);
    if (WIFEXITED(status)) 
        print("[Main]: Proces generowania pacjentow zakonczony z kodem %d.\n", WEXITSTATUS(status));
    else 
        print("[Main]: Proces generowania pacjentow  zakonczony niepowodzeniem.\n");
    

    // KONCZENIE PRACY PROGRAMU: KONCZENIE ZBEDNYCH PROCESOW I ZWALNIANIE ZASOBOW 
    wyczyscProcesyPacjentow(); 
    // Zakoncz wszystkie procesy pacjentow, ktore nie zdazyly sie zakonczyc, po zakonczeniu generatora pacjentow
    // i zakonczeniu rejestracji, aby nie prosily o nieistniejace juz zasoby
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(msg_key_rej);
    zwolnijPamiecWspoldzielona(shm_key);
    zwolnijKolejkeKomunikatow(msg_key_POZ);
    zwolnijKolejkeKomunikatow(msg_key_KARDIO);
    zwolnijKolejkeKomunikatow(msg_key_OKUL);
    zwolnijKolejkeKomunikatow(msg_key_PED);
    zwolnijKolejkeKomunikatow(msg_key_MP);

    usunNiepotrzebnePliki();

    return 0;
}