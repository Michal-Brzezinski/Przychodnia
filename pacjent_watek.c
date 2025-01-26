#include <pthread.h>
#include "pacjent.h"

// Struktura do przechowywania danych dla dziecka i opiekuna
typedef struct {
    Pacjent *pacjent; // Wskaźnik na pacjenta (dziecko)
    int semID;        // ID semafora
    int msg_id;       // ID kolejki komunikatów
} DaneWatek;

void *dziecko(void *arg) {
    DaneWatek *dane = (DaneWatek *)arg;

    printf("\033[1;34m[Dziecko]: Pacjent nr %d (wiek: %d, VIP: %d) czeka na opiekuna.\033[0m\n",
           dane->pacjent->id_pacjent, dane->pacjent->wiek, dane->pacjent->vip);
    fflush(stdout);

    // Dziecko czeka na sygnał od opiekuna
    waitSemafor(dane->semID, 1, 0);
    printf("\033[1;34m[Dziecko]: Pacjent nr %d wszedł do rejestracji z opiekunem.\033[0m\n",
           dane->pacjent->id_pacjent);
    fflush(stdout);

    // Wysyłanie wiadomości
    Wiadomosc msg;
    inicjalizujWiadomosc(&msg, dane->pacjent);
    if (msgsnd(dane->msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("\033[1;31m[Dziecko]: Błąd msgsnd\033[0m\n");
        exit(1);
    }

    return NULL;
}

void *opiekun(void *arg) {
    DaneWatek *dane = (DaneWatek *)arg;

    printf("\033[1;34m[Opiekun]: Opiekun dla pacjenta nr %d wprowadza dziecko do budynku.\033[0m\n",
           dane->pacjent->id_pacjent);
    fflush(stdout);

    // Opiekun wprowadza dziecko do budynku
    signalSemafor(dane->semID, 1);

    // Opiekun opuszcza rejestrację
    printf("\033[1;34m[Opiekun]: Opiekun dla pacjenta nr %d opuścił rejestrację.\033[0m\n",
           dane->pacjent->id_pacjent);
    fflush(stdout);

    return NULL;
}

int main() {
    Pacjent pacjent;
    inicjalizujPacjenta(&pacjent);

    key_t klucz_wejscia = generuj_klucz_ftok('.', 'A');
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    key_t msg_key = generuj_klucz_ftok('.', 'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key, IPC_CREAT | 0600);

    if (pacjent.wiek < 18) {
        pthread_t watek_dziecko, watek_opiekun;
        DaneWatek dane = {&pacjent, semID, msg_id};

        pthread_create(&watek_opiekun, NULL, opiekun, &dane);
        pthread_create(&watek_dziecko, NULL, dziecko, &dane);

        pthread_join(watek_opiekun, NULL);
        pthread_join(watek_dziecko, NULL);
    } else {
        // Standardowy przepływ dla pełnoletnich pacjentów
        printf("\033[1;34m[Pacjent]: Pacjent nr %d (wiek: %d, VIP: %d) próbuje wejść do budynku.\033[0m\n",
               pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
        waitSemafor(semID, 0, 0);

        Wiadomosc msg;
        inicjalizujWiadomosc(&msg, &pacjent);
        if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
            perror("\033[1;31m[Pacjent]: Błąd msgsnd\033[0m\n");
            exit(1);
        }
    }

    return 0;
}
