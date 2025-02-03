
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

#define S 5             // ilosc semaforow w zbiorze - w razie potrzeby zwiekszyc
#define BUILDING_MAX 8  // maksymalna pojemnosc pacjentow w budynku
#define MAX_GENERATE 100 // maksymalna liczba procesow pacjentow do wygenerowania
#define PAM_SIZE 7      // Rozmiar tablicy pamieci wspoldzielonej
// struktura pamieci wspoldzielonej
// pamiec_wspoldzielona[0] - wspolny licznik pacjentow
// pamiec_wspoldzielona[1-5] - limity pacjentow dla lekarzy
// pamiec_wspoldzielona[6] - licznik procesow, ktore zapisaly do pamieci dzielonej

volatile sig_atomic_t keep_generating = 1;
pid_t generator_lekarzy_pid = -1;
pid_t rejestracja_pid = -1;
pid_t lekarze_pid[5];             // Tablica do przechowywania PID-ów procesów lekarzy
pid_t pacjenci_pid[MAX_GENERATE]; // Tablica do przechowywania PID-ów procesów pacjentów

int shm_id; // id pamieci wspoldzielonej
int *pamiec_wspoldzielona;
int sem_id;            // id zbioru semaforow
int msg_id_rej;       // id kolejki do rejestracji
int msg_id_wyjscie;    // kolejka sluzaca do odpowiedniego sygnalizowania 
                      //  pacjentom kiedy powinni wyjsc z przychodni
int msg_id_POZ;       // id 1. kolejki POZ
int msg_id_KARDIO;    // id 2. kolejki KARDIOLOGA
int msg_id_OKUL;      // id 3. kolejki OKULISTY
int msg_id_PED;       // id 4. kolejki PEDIATRY
int msg_id_MP;        // id 5. kolejki LEKARZA MEDYCYNY PRACY
key_t klucz_wejscia;  // klucz do semafora panujacego nad iloscia pacjentow w budynku
key_t klucz_wyjscia;  // klucz odpowiedni dla msg_id_wyjscie
key_t shm_key;        // klucz do pamieci wspoldzielonej
key_t msg_key_rej;    // klucz do kolejki do rejestracji
key_t msg_key_POZ;    // klucz do kolejki do POZ
key_t msg_key_KARDIO; // klucz do kolejki do KARDIOLOGA
key_t msg_key_OKUL;   // klucz do kolejki do OKULISTY
key_t msg_key_PED;    // klucz do kolejki do PEDIATRY
key_t msg_key_MP;     // klucz do kolejki do LEKARZA MEDYCYNY PRACY

/*  --------------   FUNKCJE OBSLUGI SYGNAŁÓW   -------------    */
void handle_sigchld(int sig)
{
    // Obsluga zakonczenia procesow potomnych
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

void handle_sigint(int sig)
{
    keep_generating = 0;

    if (generator_lekarzy_pid > 0)
    {
        kill(generator_lekarzy_pid, SIGTERM);
    }

    if (rejestracja_pid > 0)
    {
        kill(rejestracja_pid, SIGTERM);
    }

    // Zwolnij zasoby IPC
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(msg_key_rej);
    zwolnijKolejkeKomunikatow(klucz_wyjscia);
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

int main()
{

    //  --------------------  INICJALIZACJA  -----------------

    int i; // zmienna iteracyjna

    klucz_wejscia = generuj_klucz_ftok(".", 'A'); // do semafora panujacego nad iloscia pacjentow w budynku
    sem_id = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0600);

    shm_key = generuj_klucz_ftok(".", 'X');
    shm_id = alokujPamiecWspoldzielona(shm_key, PAM_SIZE * sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shm_id, 0);
    memset(pamiec_wspoldzielona, 0, PAM_SIZE * sizeof(int)); // Inicjalizacja wszystkich elementów pamięci na 0

    msg_key_rej = generuj_klucz_ftok(".", 'B');
    msg_id_rej = alokujKolejkeKomunikatow(msg_key_rej, IPC_CREAT | IPC_EXCL | 0600);

    klucz_wyjscia = generuj_klucz_ftok(".", 'W');
    msg_id_wyjscie = alokujKolejkeKomunikatow(klucz_wyjscia, IPC_CREAT | IPC_EXCL | 0600);

    msg_key_POZ = generuj_klucz_ftok(".", 1);
    msg_id_POZ = alokujKolejkeKomunikatow(msg_key_POZ, IPC_CREAT | IPC_EXCL | 0600);

    msg_key_KARDIO = generuj_klucz_ftok(".", 2);
    msg_id_KARDIO = alokujKolejkeKomunikatow(msg_key_KARDIO, IPC_CREAT | IPC_EXCL | 0600);

    msg_key_OKUL = generuj_klucz_ftok(".", 3);
    msg_id_OKUL = alokujKolejkeKomunikatow(msg_key_OKUL, IPC_CREAT | IPC_EXCL | 0600);

    msg_key_PED = generuj_klucz_ftok(".", 4);
    msg_id_PED = alokujKolejkeKomunikatow(msg_key_PED, IPC_CREAT | IPC_EXCL | 0600);

    msg_key_MP = generuj_klucz_ftok(".", 5);
    msg_id_MP = alokujKolejkeKomunikatow(msg_key_MP, IPC_CREAT | IPC_EXCL | 0600);

    inicjalizujSemafor(sem_id, 0, BUILDING_MAX);                           // semafor zainicjalizowany na maksymalna liczbe pacjentow w budynku
    inicjalizujSemafor(sem_id, 1, 0);                                      // potrzebny, aby proces pacjenta czekal na potwierdzenie przyjecia
    inicjalizujSemafor(sem_id, 2, 1);                                      // semafor mowiacy, ze rejestracja jest zamknieta
    inicjalizujSemafor(sem_id, 3, 1);                                      // semafor dostepu do pamieci wspoldzielonej
    inicjalizujSemafor(sem_id, 4, 0);                                      // semafor do pracy z plikiem
    int limit_pacjentow = losuj_int(MAX_GENERATE / 2) + MAX_GENERATE / 2; // Losowy limit pacjentow dla wszystkich lekarzy
    char arg2[10];                                                        // arg2 to limit pacjentow dla wszystkich lekarzy - uzywany jako argument w execl
    sprintf(arg2, "%d", limit_pacjentow);                                 // Konwersja liczby na ciag znakow

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
    if (sigaction(SIGCHLD, &sa, 0) == -1)
    {
        perror_red("[Main]: sigaction\n");
        exit(1);
    }

    //-----------------------   REJESTRACJA  -------------------------------

    rejestracja_pid = fork();
    switch (rejestracja_pid)
    {
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
        break; // Kontynuuj generowanie pozostalych procesow
    }

    /*--------------------------    LEKARZE   ---------------------------------------*/

    generator_lekarzy_pid = fork();
    if (generator_lekarzy_pid == -1)
    {
        perror_red("[Main]: Blad fork dla generatora lekarzy\n");
        exit(4);
    }
    else if (generator_lekarzy_pid == 0)
    {
        // Proces potomny: generowanie lekarzy
        // Teoretycznie ma ten sam handler zakonczenia procesow dzieci
        print("[Main]: Maksymalna liczba pacjentow do przyjecia to %d\n", limit_pacjentow);
        char arg1[2]; // arg1 to id lekarza

        for (i = 1; i < 6; i++)
        {
            sprintf(arg1, "%d", i); // Konwersja liczby na ciag znakow

            pid_t pid = fork();
            if (pid == -1)
            {
                perror_red("[Main]: Blad fork (generator lekarzy) - probuj generowac lekarzy dalej\n");
                break;
            }
            else if (pid == 0)
            {
                execl("./lekarz", "lekarz", arg1, arg2, NULL);
                // 1. argument to id lekarza, 2. argument to limit pacjentow dla wszystkich lekarzy losowo wygenerowany
                perror_red("[Main]: Blad execl dla lekarza\n");
                exit(5);
            }
            else
            {
                lekarze_pid[i - 1] = pid; // Zapisz PID procesu lekarza
            }
            // Proces rodzic: sprawdz zakonczenie procesu potomnego bez blokowania
            while (waitpid(-1, NULL, WNOHANG) > 0)
                ;
            // sprawdza, czy jakikolwiek proces potomny zakonczyl sie, ale nie blokuje,
            // jesli zaden proces nie jest gotowy do zakonczenia - flaga WNOHANG
        }

        exit(0); // Zakoncz proces potomny po wygenerowaniu pacjentow
    }

    //---------------------------   PACJENCI     -------------------------------

    // Utworz proces potomny do generowania pacjentow
    // Aby proces rodzic mogl wykonywac inne zadania

        // Proces potomny: generowanie pacjentow
        // Teoretycznie ma ten sam handler zakonczenia procesow dzieci
    for (i = 0; i < MAX_GENERATE && keep_generating; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror_red("[Main]: Blad fork (generator pacjentow) - probuj generowac pacjentow dalej\n");
            break;
        }
        else if (pid == 0)
        {
            execl("./pacjent", "pacjent", NULL);
            perror_red("[Main]: Blad execl dla pacjenta\n");
            exit(2);
        }
        else
            pacjenci_pid[i] = pid;
        // Proces rodzic: sprawdz zakonczenie procesu potomnego bez blokowania
        // while (waitpid(-1, NULL, WNOHANG) > 0);
        // prawdza, czy jakikolwiek proces potomny zakonczyl sie, ale nie blokuje,
        // jesli zaden proces nie jest gotowy do zakonczenia - flaga WNOHANG
        usleep(500000); // opzoznienie w generowaniu nowych pacjentow
    }
    while (waitpid(-1, NULL, WNOHANG) > 0);
    keep_generating = 0;
        //exit(0); // Zakoncz proces potomny po wygenerowaniu pacjentow


    /*  -----------   OBSŁUGA POMYŚLNEGO ZAKOŃCZENIA PROGRAMU    ----------- */
    /*   ---------- (ZWOLNIENIE ZASOBÓW I CZEKANIE NA PROCESY) -----------   */

    while (keep_generating)
    { // dziala tak dlugo jak nie konczy sie generowanie pacjentow
        sleep(1);
    }

    // Czekanie na zakończenie procesów potomnych
    // waitpid(generator_pacjentow_pid, NULL, 0);
    waitpid(generator_lekarzy_pid, NULL, 0);
    waitpid(rejestracja_pid, NULL, 0);

    // KONCZENIE PRACY PROGRAMU: KONCZENIE ZBEDNYCH PROCESOW I ZWALNIANIE ZASOBOW
    wyczyscProcesyPacjentow();
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
    // Po zamknieciu ewentualnych pozostalych pacjentow, oczekuje na ich zakonczenie by uniknac zombie

    // Zakoncz wszystkie procesy pacjentow, ktore nie zdazyly sie zakonczyc, po zakonczeniu generatora pacjentow
    // i zakonczeniu rejestracji, aby nie prosily o nieistniejace juz zasoby
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(klucz_wyjscia);
    zwolnijKolejkeKomunikatow(msg_key_rej);
    zwolnijPamiecWspoldzielona(shm_key);
    zwolnijKolejkeKomunikatow(msg_key_POZ);
    zwolnijKolejkeKomunikatow(msg_key_KARDIO);
    zwolnijKolejkeKomunikatow(msg_key_OKUL);
    zwolnijKolejkeKomunikatow(msg_key_PED);
    zwolnijKolejkeKomunikatow(msg_key_MP);

    usunNiepotrzebnePliki();

    print("[Main]: Glowny proces zakonczyl sie po zakonczeniu wygenerowanych procesow\n");
    return 0;
}