#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <time.h>

#include "MyLib/sem_utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/dekoratory.h"
#include <pthread.h>


#define BUILDING_MAX 10 // Maksymalna liczba pacjentów w budynku
#define K (BUILDING_MAX / 2)
#define ZAMKNIECIE_OKIENKA2 (BUILDING_MAX / 3)
#define S 2 // Ilość semaforów w zbiorze

int msg_id;
int aktywne_okienka = 1; // Liczba otwartych okienek
int rozmiar_kolejki_rejestracji = 0; // Liczba pacjentów w kolejce

pthread_t okno2; // Wątek dla drugiego okienka
int okno2_aktywne = 0; // Czy drugi wątek jest aktywny
pthread_mutex_t lock; // Mutex do synchronizacji

volatile int running = 1; // Flaga do sygnalizowania zakończenia




// Zmienna globalna do przechowywania pid procesu okienka nr 2
pid_t pid_okienka2 = -1;

// Funkcja pomocnicza do obliczania liczby procesów w kolejce
int policzProcesy(int msg_id) {
    // Zlicz liczbę komunikatów w kolejce (w prostym przypadku nieopóźnionym)
    int liczba_procesow = 0;
    struct msqid_ds buf;
    if (msgctl(msg_id, IPC_STAT, &buf) == -1) {
        perror("msgctl IPC_STAT");
        exit(1);
    }
    liczba_procesow = buf.msg_qnum;
    return liczba_procesow;
}

void uruchomOkienkoNr2(int msg_id, int semID) {
    // Uruchomienie dodatkowego procesu rejestracji (okienka nr 2)
    pid_okienka2 = fork();
    if (pid_okienka2 == 0) {
        // Dziecko: Okienko nr 2
        printf("Otworzenie okienka nr 2...\n");
        while (1) {
            // Czekaj na komunikaty
            Wiadomosc msg;
            if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1) {
                perror("Błąd msgrcv");
                exit(1);
            }
            printf("Okienko nr 2: Rejestracja pacjenta %d\n", msg.id_pacjent);
            sleep(1);  // symulacja procesu rejestracji
        }
    }
}

void zatrzymajOkienkoNr2() {
    // Jeśli proces okienka nr 2 istnieje, wyślij do niego sygnał zakończenia
    if (pid_okienka2 != -1) {
        printf("Zatrzymywanie okienka nr 2...\n");
        kill(pid_okienka2,SIGTERM);  // Wysłać sygnał SIGTERM do procesu okienka nr 2 (zakończenie)
    }
}

int main() {
    key_t msg_key = generuj_klucz_ftok(".", 'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key, IPC_CREAT | 0600);
    key_t klucz_wejscia = generuj_klucz_ftok(".", 'A');
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    // Aktualny czas
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    // Godziny otwarcia i zamknięcia rejestracji (w sekundach od północy)
    int Tp = current_time;          // Aktualny czas
    int Tk = current_time + 60;     // Aktualny czas + 60 sekund (1 minuta)

        printf("Uruchomiono rejestrację\n");

    while (1) {
        // Sprawdź aktualny czas
        now = time(NULL);
        local = localtime(&now);
        current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

        // Sprawdź, czy aktualny czas jest poza godzinami otwarcia
        if (current_time < Tp || current_time > Tk) {
            printf("Rejestracja jest zamknięta. Kończenie pracy.\n");
            break;  // Wyjście z pętli, gdy czas jest poza godzinami otwarcia
        }

        // Czekaj na komunikat rejestracji
        Wiadomosc msg;
        if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1) {
            perror("Błąd msgrcv");
            exit(1);
        }
        printf("Rejestracja pacjenta %d\n", msg.id_pacjent);

        // Sprawdź liczbę procesów oczekujących na rejestrację ponownie
        int liczba_procesow = policzProcesy(msg_id);
        if (liczba_procesow > K) {
            // Uruchomienie okienka nr 2
            uruchomOkienkoNr2(msg_id, semID);
        } else if (liczba_procesow < K / 3) {
            // Jeśli liczba procesów spadła poniżej K/3, zatrzymaj okienko nr 2
            zatrzymajOkienkoNr2();
        }

        // Proces rejestracji kontynuuje swoją pracę
        sleep(1);
    }
}



/*
void *process_patients(void *arg) {
    int window_id = *(int *)arg;
    free(arg);

    while (running) {
        pthread_mutex_lock(&lock);

        if (rozmiar_kolejki_rejestracji > 0) {
            // Obsłuż pacjenta z kolejki
            Wiadomosc msg;
            if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1) {
                perror("Błąd msgrcv");
                pthread_mutex_unlock(&lock);
                continue;
            }

            printf("Pacjent %d (wiek: %d, VIP: %d) zgłosił się do rejestracji, do lekarza %d.\n",
                   msg.id_pacjent, msg.wiek, msg.vip, msg.id_lekarz);
            fflush(stdout);

            rozmiar_kolejki_rejestracji--;
            pthread_mutex_unlock(&lock);

            sleep(2); // Czas obsługi pacjenta
            printf("Okienko %d: Pacjent %d obsłużony.\n", window_id, msg.id_pacjent);
            
        } else {
            pthread_mutex_unlock(&lock);
            sleep(1); // Oczekiwanie na pacjentów
        }
    }

    printf("Okienko %d: Zamyka się.\n", window_id);
    return NULL;
}*/

/*
int main() {
    key_t msg_key;
    pthread_mutex_init(&lock, NULL);

    // Utwórz unikalny klucz dla kolejki komunikatów
    if ((msg_key = ftok(".", 'B')) == -1) {
        perror("Błąd ftok");
        exit(1);
    }

    // Utwórz kolejkę komunikatów
    if ((msg_id = msgget(msg_key, IPC_CREAT | 0666)) == -1) {
        perror("Błąd msgget");
        exit(2);
    }

    printf("Rejestracja uruchomiona, oczekuje na pacjentów...\n");
    fflush(stdout);

    // Uruchom pierwszy wątek
    int *arg1 = malloc(sizeof(int));
    *arg1 = 1;
    pthread_create(&okno2, NULL, process_patients, arg1);

    while (running) {
        Wiadomosc msg;
        // Odbierz wiadomość od pacjenta
        if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1) {
            perror("Błąd msgrcv");
            continue;
        }

        printf("Pacjent %d (wiek: %d, VIP: %d) w kolejce do rejestracji.\n",
               msg.id_pacjent, msg.wiek, msg.vip);

        pthread_mutex_lock(&lock);
        rozmiar_kolejki_rejestracji++;

        // Dynamiczne otwieranie/zamykanie drugiego okienka
        if (rozmiar_kolejki_rejestracji > K && okno2_aktywne == 0) {
            printf("Otwiera się drugie okienko rejestracji.\n");
            okno2_aktywne = 1;

            int *arg2 = malloc(sizeof(int));
            *arg2 = 2;
            pthread_create(&okno2, NULL, process_patients, arg2);
        } else if (rozmiar_kolejki_rejestracji < ZAMKNIECIE_OKIENKA2 && okno2_aktywne == 1) {
            printf("Zamyka się drugie okienko rejestracji.\n");
            okno2_aktywne = 0;
            running = 0; // Sygnalizowanie wątkowi, aby zakończył działanie
            pthread_join(okno2, NULL); // Czekanie na zakończenie wątku
        }

        pthread_mutex_unlock(&lock);

        if (msg.id_pacjent == -1) { // Sygnał zamknięcia
            running = 0; // Zakończ działanie
            break;
        }
    }

    pthread_mutex_destroy(&lock);

    // Usuń kolejkę komunikatów
    if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        perror("Błąd usuwania kolejki komunikatów");
    }
    return 0;
}*/

