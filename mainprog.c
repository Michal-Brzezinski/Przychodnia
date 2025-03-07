
/*
PLIK SKLADA SIE Z NIEZBEDNYCH INICJALIZACJI ZASOBOW, BY POZNIEJ EJ ZWOLNIC, Z DEFINICJI FUNKCJI OBSLUGI SYGNALOW,
GENEROWANIA PROCESOW POTOMNYCH - LEKARZY, PACJENTOW I REJESTRACJI
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
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

#include "MyLib/sem_utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/utils.h"
#include "MyLib/shm_utils.h"

#define FIFO_DYREKTOR "fifo_dyrektor"   // nazwa kolejki fifo do przekazywania pidu lekarza dyrektorowi

// Dane do moderowania pracy programu
const static int max_generate = 6000; // maksymalna liczba procesow pacjentow do wygenerowania
int limit_pacjentow = 4000; // maksymalna liczba pacjentow przyjetych przez wszystkich lekarzy
const static char *building_max = "3000";  //maksymalna liczba pacjentow w budynku (230 - JESZCZE DZIALA POTEM SIE BLOKUJE)
const static char *Tp = "06:39";
const static char *Tk = "22:40";

// ________________________________________________________________________________
// #define SLEEP // zakomentowac, jesli nie chcemy sleepow w generowaniu pacjentow  <--- DO TESTOWANIA
// W PLIKU UTILS.H -> DLA WSZYSTKICH PLIKOW 
// ________________________________________________________________________________

// struktura pamieci wspoldzielonej

// pamiec_wspoldzielona_rej[0] - wspolny licznik pacjentow DLA REJESTRACJI
// pamiec_wspoldzielona_rej[1-5] - limity pacjentow dla lekarzy DLA REJESTRACJI

volatile sig_atomic_t keep_generating = 1;  // zmienne sig_atomic_t do obslugi sygnalow
volatile sig_atomic_t rejestracja_dziala = 1;
volatile sig_atomic_t zakoncz_gc = 0;
pid_t generator_lekarzy_pid = -1;
pid_t rejestracja_pid = -1;
pid_t dyrektor_pid = -1;
pthread_t tid;
pid_t lekarze_pid[5];             // Tablica do przechowywania PID-ow procesow lekarzy

int shm_id_rejestracja; // id pamieci wspoldzielonej
int *pamiec_wspoldzielona_rej;
key_t shm_key_rejestracja;        // klucz do pamieci wspoldzielonej rejestracji

int shm_dostepnosc;//
sig_atomic_t *dostepnosc_lekarza;//
key_t shm_key_dostepnosc;//

int sem_id;            // id zbioru semaforow

int msg_id_rej;       // id kolejki do rejestracji
int msg_id_wyjscie;    // kolejka sluzaca do odpowiedniego sygnalizowania 
                      //  pacjentom kiedy powinni wyjsc z przychodni
int msg_id_POZ;       // id 1. kolejki POZ
int msg_id_KARDIO;    // id 2. kolejki KARDIOLOGA
int msg_id_OKUL;      // id 3. kolejki OKULISTY2
int msg_id_PED;       // id 4. kolejki PEDIATRY
int msg_id_MP;        // id 5. kolejki LEKARZA MEDYCYNY PRACY
key_t klucz_wejscia;  // klucz do semafora panujacego nad iloscia pacjentow w budynku
key_t klucz_wyjscia;  // klucz odpowiedni dla msg_id_wyjscie
key_t msg_key_rej;    // klucz do kolejki do rejestracji
key_t msg_key_POZ;    // klucz do kolejki do POZ
key_t msg_key_KARDIO; // klucz do kolejki do KARDIOLOGA
key_t msg_key_OKUL;   // klucz do kolejki do OKULISTY
key_t msg_key_PED;    // klucz do kolejki do PEDIATRY
key_t msg_key_MP;     // klucz do kolejki do LEKARZA MEDYCYNY PRACY

void sleep_with_interrupts(int seconds) {
// funkcja sleep z obsluga przerwania sygnalami
    struct timespec req, rem;
    req.tv_sec = seconds;
    req.tv_nsec = 0;

    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem; // Kontynuuj sen z pozostalym czasem
    }
}

//  ______________________________   FUNKCJE OBSLUGI SYGNALOW   ______________________________  
void handle_sigchld(int sig)
{
    // Obsluga zakonczenia procesow potomnych
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void obsluga_USR1(int sig){
    rejestracja_dziala = 0;
}

void handle_sigint(int sig);
int process_exists(const char *process_name);
void *cleanup_thread(void *arg);


int main()
{

    //  ______________________________  INICJALIZACJA  ______________________________

    int i; // zmienna iteracyjna

    klucz_wejscia = generuj_klucz_ftok(".", 'A'); // do zbioru semaforow
    sem_id = alokujSemafor(klucz_wejscia, S, IPC_CREAT | IPC_EXCL | 0600);

    inicjalizujSemafor(sem_id, 0, atoi(building_max));                // semafor zainicjalizowany na maksymalna liczbe pacjentow w budynku
    inicjalizujSemafor(sem_id, 1, 0);                            // potrzebny, aby proces pacjenta czekal na potwierdzenie przyjecia
    inicjalizujSemafor(sem_id, 2, 0);                            // semafor mowiacy, ze rejestracja jest zamknieta
    inicjalizujSemafor(sem_id, 3, 1);                            // semafor dostepu do tablicy przyjec w rejestracji
    inicjalizujSemafor(sem_id, 4, 1);                            // semafor do pracy z plikiem 
    inicjalizujSemafor(sem_id, 5, 0);                            // pomocniczy semafor - czy budynek jest otwarty - moze byc signalowany przez dyrektora
    inicjalizujSemafor(sem_id, 6, 1);                            // semafor do kontroli pracy nad semaforem 5
    inicjalizujSemafor(sem_id, 14, 0);                           // semafor, ktory kontroluje czy wszystkie procesy lekarzy rozpoczely dzialanie
    
    shm_key_rejestracja = generuj_klucz_ftok(".", 'X');
    shm_id_rejestracja = alokujPamiecWspoldzielona(shm_key_rejestracja, PAM_SIZE * sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    pamiec_wspoldzielona_rej = dolaczPamiecWspoldzielona(shm_id_rejestracja, 0);
    memset(pamiec_wspoldzielona_rej, 0, PAM_SIZE * sizeof(int)); // Inicjalizacja wszystkich elementow pamieci na 0

    //
    shm_key_dostepnosc = generuj_klucz_ftok(".", 'D');
    shm_dostepnosc = alokujPamiecWspoldzielona(shm_key_dostepnosc, DOSTEPNOSC * sizeof(sig_atomic_t), IPC_CREAT | IPC_EXCL | 0600);
    dostepnosc_lekarza = dolaczPamiecWspoldzielona(shm_dostepnosc, 0);
    memset(dostepnosc_lekarza, 1, DOSTEPNOSC * sizeof(sig_atomic_t)); // Inicjalizacja wszystkich elementow pamieci na 0
//

    msg_key_rej = generuj_klucz_ftok(".", 'B');
    msg_id_rej = alokujKolejkeKomunikatow(msg_key_rej, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI REJESTRACJI
    inicjalizujSemafor(sem_id, 7, MAX_KOMUNIKATOW); 

    klucz_wyjscia = generuj_klucz_ftok(".", 'W');
    msg_id_wyjscie = alokujKolejkeKomunikatow(klucz_wyjscia, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI WYJSCIA PACJENTOW
    inicjalizujSemafor(sem_id, 8, MAX_KOMUNIKATOW);

    msg_key_POZ = generuj_klucz_ftok(".", 1);
    msg_id_POZ = alokujKolejkeKomunikatow(msg_key_POZ, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI LEKARZA POZ
    inicjalizujSemafor(sem_id, 9, MAX_KOMUNIKATOW);

    msg_key_KARDIO = generuj_klucz_ftok(".", 2);
    msg_id_KARDIO = alokujKolejkeKomunikatow(msg_key_KARDIO, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI LEKARZA KARDIOLOGA
    inicjalizujSemafor(sem_id, 10, MAX_KOMUNIKATOW);

    msg_key_OKUL = generuj_klucz_ftok(".", 3);
    msg_id_OKUL = alokujKolejkeKomunikatow(msg_key_OKUL, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI LEKARZA OKULISTY
    inicjalizujSemafor(sem_id, 11, MAX_KOMUNIKATOW);

    msg_key_PED = generuj_klucz_ftok(".", 4);
    msg_id_PED = alokujKolejkeKomunikatow(msg_key_PED, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI LEKARZA PEDIATRY
    inicjalizujSemafor(sem_id, 12, MAX_KOMUNIKATOW);

    msg_key_MP = generuj_klucz_ftok(".", 5);
    msg_id_MP = alokujKolejkeKomunikatow(msg_key_MP, IPC_CREAT | IPC_EXCL | 0600);
    // SEMAFOR DO RADZENIA SOBIE Z PRZEPELNIENIEM KOLEJKI LEKARZA MEDYCYNY PRACY
    inicjalizujSemafor(sem_id, 13, MAX_KOMUNIKATOW);

    char arg2[10];                                                        // arg2 to limit pacjentow dla wszystkich lekarzy - uzywany jako argument w execl
    sprintf(arg2, "%d", limit_pacjentow);                                 // Konwersja liczby na ciag znakow


    // ___________________________ TWORZENIE PLIKOW ______________________________

    // tworze potok nazwany do przekazania pidu losowego lekarza
    if(mkfifo(FIFO_DYREKTOR, 0666) == -1) {
        if(errno != EEXIST) {
            perror_red("[Main]: Blad mkfifo dla potoku dyrektora\n");
            exit(1);
        }
    }  

    //  ______________________________   OBSLUGA SYGNALOW    ______________________________

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

    // Obsluga sygnalu SIGUSR1 - sygnal po zakonczeniu rejestracji
    struct sigaction usr1;
    usr1.sa_handler = obsluga_USR1;
    usr1.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&usr1.sa_mask);
    if (sigaction(SIGUSR1, &usr1, NULL) == -1)
    {
        perror_red("[Pacjent]: sigaction\n");
        exit(1);
    }

    //______________________________   DYREKTOR  ______________________________

    dyrektor_pid = fork();
    switch (dyrektor_pid)
    {
    case -1:
        perror_red("[Main]: Blad fork dla dyrektora\n");
        // obsluga bledu - powiadomienie innych procesow (ktore utworzyly sie poprawnie) o bledzie (np. sygnaly)
        exit(1);
    case 0:
        execl("./dyrektor", "dyrektor", Tp, Tk, NULL);
        // usuwanie wszystkich procesow i zasobow ipc - zaimplementowac
        perror_red("[Main]: Blad execl dla rejestracji\n");
        exit(1);
    default:
        break; // Kontynuuj generowanie pozostalych procesow
    }

    //______________________________  REJESTRACJA  ______________________________

    rejestracja_pid = fork();
    switch (rejestracja_pid)
    {
    case -1:
        perror_red("[Main]: Blad fork dla rejestracji\n");
        // obsluga bledu - powiadomienie innych procesow (ktore utworzyly sie poprawnie) o bledzie (np. sygnaly)
        exit(1);
    case 0:
        print("[Main]: Uruchamianie rejestracji...\n");
        execl("./rejestracja", "rejestracja", arg2, Tp, Tk, building_max, NULL);
        // usuwanie wszystkich procesow i zasobow ipc - zaimplementowac
        perror_red("[Main]: Blad execl dla rejestracji\n");
        exit(1);
    default:
        break; // Kontynuuj generowanie pozostalych procesow
    }

    //______________________________    LEKARZE   ______________________________

    generator_lekarzy_pid = fork();
    if (generator_lekarzy_pid == -1)
    {
        perror_red("[Main]: Blad fork dla generatora lekarzy\n");
        exit(1);
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
                printRed("Lekarz o id %d ma pid %d\n", i, getpid());
                execl("./lekarz", "lekarz", arg1, arg2, Tp, Tk, NULL);
                // 1. argument to id lekarza, 2. argument to limit pacjentow dla wszystkich lekarzy losowo wygenerowany
                perror_red("[Main]: Blad execl dla lekarza\n");
                exit(1);
            }
            else
            {
                lekarze_pid[i - 1] = pid; // Zapisz PID procesu lekarza
            }
            // Proces rodzic: sprawdz zakonczenie procesu potomnego bez blokowania
            while (waitpid(-1, NULL, WNOHANG) > 0);
            // sprawdza, czy jakikolwiek proces potomny zakonczyl sie, ale nie blokuje,
            // jesli zaden proces nie jest gotowy do zakonczenia - flaga WNOHANG
        }

        print("Zakonczono generowanie lekarzy\n");


        //  ______________  PRZEKAZANIE PIDU LOSOWEGO LEKARZA DO DYREKTORA PRZEZ FIFO   _____________________
    

        int losowy_indeks_tablicy_PIDow_lekarzy = losuj_int(4); //losuje indeksy od 0 do 4 czyli tak jak chce
        int random_lekarz_pid = lekarze_pid[losowy_indeks_tablicy_PIDow_lekarzy];
        
        // Otwieramy potok do zapisu i wysylamy PID (jako ciag znakow)
        int fifo_fd = open(FIFO_DYREKTOR, O_WRONLY);
        if(fifo_fd == -1) {
            perror_red("[Generator lekarzy]: Blad otwarcia potoku do dyrektora\n");
            exit(1);
        }
        char pid_string[20];
        sprintf(pid_string, "%d", random_lekarz_pid);
        if(write(fifo_fd, pid_string, strlen(pid_string)+1) == -1) {
            perror_red("[Generator lekarzy]: Blad zapisu do potoku\n");
            exit(1);
        }
        close(fifo_fd);

        //  ____________________________________________________________________________________________

        while (waitpid(-1, NULL, WNOHANG) > 0); // czeka na zakonczenie pracy wszystkich lekarzy
        exit(0); // Zakoncz proces potomny po wygenerowaniu pacjentow
    }


    if (pthread_create(&tid, NULL, cleanup_thread, &msg_id_wyjscie) != 0) {
        perror("pthread_create");
        exit(1);
    }


    //______________________________    PACJENCI     ______________________________

    // Utworz proces potomny do generowania pacjentow
    // Aby proces rodzic mogl wykonywac inne zadania

        // Proces potomny: generowanie pacjentow
        // Teoretycznie ma ten sam handler zakonczenia procesow dzieci

    for (i = 0; i < max_generate && keep_generating; i++)
    {
        #ifdef SLEEP
        sleep_with_interrupts(1); // opzoznienie w generowaniu nowych pacjentow
        #endif

        if(rejestracja_dziala == 0) {
            printYellow("[Main]: Proces generowania pacjentow zakonczyl sie po zamknieciu przychodni\n");
            break; // nie ma sensu generowac weicej pacjentow po zakonczeniu dzialania rejestracji
        }
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
            exit(1);
        }

    }
    while (waitpid(-1, NULL, WNOHANG) > 0);
    keep_generating = 0;


    //  __________________   OBSLUGA POMYSLNEGO ZAKONCZENIA PROGRAMU    __________________
    //   __________________ (ZWOLNIENIE ZASOBOW I CZEKANIE NA PROCESY)  __________________

    while (keep_generating);
    // dziala tak dlugo jak nie konczy sie generowanie pacjentow

    // Czekanie na zakonczenie procesow potomnych
    waitpid(dyrektor_pid, NULL, 0);
    waitpid(generator_lekarzy_pid, NULL, 0);
    waitpid(rejestracja_pid, NULL, 0);

    // KONCZENIE PRACY PROGRAMU: KONCZENIE ZBEDNYCH PROCESOW I ZWALNIANIE ZASOBOW
    wyczyscProcesyPacjentow();
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
    // Po zamknieciu ewentualnych pozostalych pacjentow, oczekuje na ich zakonczenie by uniknac zombie

    // Zakoncz wszystkie procesy pacjentow, ktore nie zdazyly sie zakonczyc, po zakonczeniu generatora pacjentow
    // i zakonczeniu rejestracji, aby nie prosily o nieistniejace juz zasoby

    pthread_join(tid, NULL);
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(klucz_wyjscia);
    zwolnijKolejkeKomunikatow(msg_key_rej);
    zwolnijPamiecWspoldzielona(shm_key_rejestracja);
    zwolnijPamiecWspoldzielona(shm_key_dostepnosc);//
    zwolnijKolejkeKomunikatow(msg_key_POZ);
    zwolnijKolejkeKomunikatow(msg_key_KARDIO);
    zwolnijKolejkeKomunikatow(msg_key_OKUL);
    zwolnijKolejkeKomunikatow(msg_key_PED);
    zwolnijKolejkeKomunikatow(msg_key_MP);

    usunNiepotrzebnePliki();

    print("[Main]: Glowny proces zakonczyl sie po zakonczeniu wygenerowanych procesow\n");

    
    return 0;
}


int process_exists(const char *process_name) {
    // funkcja sprawdzająca czy istnieja procesy o danej nazwie
    
    struct dirent *entry;
    DIR *dp = opendir("/proc");

    if (!dp) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {
        // Sprawdzenie, czy nazwa składa się tylko z cyfr (PID)
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
            continue;

        // Konstrukcja ściezki do pliku "/proc/PID/comm"
        char path[256];
        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

        // Otwieramy plik, który zawiera nazwe procesu
        FILE *fp = fopen(path, "r");
        if (fp) {
            char name[256];
            if (fgets(name, sizeof(name), fp)) {
                name[strcspn(name, "\n")] = 0;  // Usunięcie znaku nowej linii

                if (strcmp(name, process_name) == 0) {
                    fclose(fp);
                    closedir(dp);
                    return 1;  // Proces istnieje
                }
            }
            fclose(fp);
        }
    }

    closedir(dp);
    return 0;  // Brak procesu o tej nazwie
}

void *cleanup_thread(void *arg) {
    // Funkcja Watku czyszczacego
    int msg_id_wyjscie = *((int *)arg);
    waitSemafor(sem_id, 6, 0);
    while (valueSemafor(sem_id, 5) == 1 && zakoncz_gc == 0) {
        signalSemafor(sem_id, 6);
        int exists = process_exists("pacjent");
        if (exists == 0) {
            
            Wiadomosc message;
            int ret;
            // Odebranie wszystkich komunikatów w trybie nieblokującym (IPC_NOWAIT)
            while ((ret = msgrcv(msg_id_wyjscie, &message, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT)) != -1) {
                
            }
            // Jeśli błąd wynika, że kolejka jest pusta, errno będzie ustawione na ENOMSG
            if (errno != ENOMSG) {
                perror("msgrcv");
            }
        }
        if(zakoncz_gc == 1) break;
        waitSemafor(sem_id, 6, 0);
    }
    signalSemafor(sem_id, 6);
    return NULL;
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
    zakoncz_gc = 1;
    // Zwolnij zasoby IPC
    zwolnijSemafor(klucz_wejscia);
    zwolnijKolejkeKomunikatow(msg_key_rej);
    zwolnijKolejkeKomunikatow(klucz_wyjscia);
    zwolnijPamiecWspoldzielona(shm_key_rejestracja);
    zwolnijPamiecWspoldzielona(shm_key_dostepnosc);//
    zwolnijKolejkeKomunikatow(msg_key_POZ);
    zwolnijKolejkeKomunikatow(msg_key_KARDIO);
    zwolnijKolejkeKomunikatow(msg_key_OKUL);
    zwolnijKolejkeKomunikatow(msg_key_PED);
    zwolnijKolejkeKomunikatow(msg_key_MP);
    usunNiepotrzebnePliki();

    waitpid(dyrektor_pid, NULL, 0);
    waitpid(generator_lekarzy_pid, NULL, 0);
    waitpid(rejestracja_pid, NULL, 0);

    wyczyscProcesyPacjentow();
    pthread_join(tid, NULL);
    raise(SIGTERM);
    //printRed("\n[Main]: Zakonczono program po otrzymaniu SIGINT.\n");
    exit(0);
}