#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "operacje.h"
#include <pthread.h>    // do obsługi dynamicznego 2. okna


#define BUILDING_MAX 3 // Maksymalna liczba pacjentów w budynku
#define K (BUILDING_MAX / 2)
#define ZAMKNIECIE_OKIENKA2 (BUILDING_MAX / 3)

int msg_id;
int aktywne_okienka = 1; // Liczba otwartych okiencek
int rozmiar_kolejki_rejestracji = 0;     // Liczba pacjentów w kolejce

pthread_t okno2;         // Wątek dla drugiego okienka
int okno2_aktywne = 0;    // Czy drugi wątek jest aktywny
pthread_mutex_t lock;      // Mutex do synchronizacji


void *process_patients(void *arg) {
    int window_id = *(int *)arg;
    free(arg);

    while (1) {
        pthread_mutex_lock(&lock);

        if (rozmiar_kolejki_rejestracji == 0 && window_id == 2 && okno2_aktywne == 1) {
            // Jeśli drugi wątek, brak pacjentów i okienko ma być zamknięte
            pthread_mutex_unlock(&lock);
            break;
        }

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
}

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

    /*while (1) {
            Wiadomosc msg;
            // Odbierz wiadomość od pacjenta
            if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1) {
                perror("Błąd msgrcv");
                continue;
            }

            printf("Pacjent %d (wiek: %d, VIP: %d) zgłosił się do rejestracji.\n",
                msg.id_pacjent, msg.wiek, msg.vip);
            fflush(stdout);

            rozmiar_kolejki_rejestracji++;

            // Obsługa otwierania/zamykania drugiego okienka
            if (rozmiar_kolejki_rejestracji > K && aktywne_okienka == 1) {
                printf("Otwiera się drugie okienko rejestracji.\n");
                aktywne_okienka = 2;
                fflush(stdout);
            } else if (rozmiar_kolejki_rejestracji < ZAMKNIECIE_OKIENKA2 && aktywne_okienka == 2) {
                printf("Zamyka się drugie okienko rejestracji.\n");
                aktywne_okienka = 1;
                fflush(stdout);
            }

            sleep(2); // Symulacja czasu rejestracji

            printf("Pacjent %d został zarejestrowany w okienku %d.\n",
                msg.id_pacjent, (rozmiar_kolejki_rejestracji > K ? 2 : 1));
            fflush(stdout);

            rozmiar_kolejki_rejestracji--;

            // Jeśli sygnał zamknięcia przychodni, zakończ pracę
            if (msg.id_pacjent == -1) {
                printf("Rejestracja zamyka się.\n");
                break;
            }
        }
*/


    
    // Uruchom pierwszy wątek
    int *arg1 = malloc(sizeof(int));
    *arg1 = 1;
    pthread_create(&okno2, NULL, process_patients, arg1);



        while (1) {
        Wiadomosc msg;
        // Odbierz wiadomość od pacjenta
        if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1) {
            perror("Błąd msgrcv");
            continue;
        }

        pthread_mutex_lock(&lock);

        printf("Pacjent %d (wiek: %d, VIP: %d) w kolejce do rejestracji.\n",
               msg.id_pacjent, msg.wiek, msg.vip);
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
            pthread_cancel(okno2);
        }

        pthread_mutex_unlock(&lock);

        if (msg.id_pacjent == -1) { // Sygnał zamknięcia
            break;
        }
    }

    pthread_mutex_destroy(&lock);
    
    // Usuń kolejkę komunikatów
    if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        perror("Błąd usuwania kolejki komunikatów");
    }
    return 0;
}
