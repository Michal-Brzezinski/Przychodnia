#include "rejestracja.h"

// identyfikator kolejki komunikatów
// extern int queue_id jest w pacjent.h

void registration_process() {

    Message message;
    Confirmation confirmation;

    printf("Rejestracja rozpoczęła pracę.\n");

    while (1) {
        // Odbieranie wiadomości o rejestracji
        if (msgrcv(queue_id, &message, sizeof(message) - sizeof(long), 0, 0) == -1) {
            perror("Błąd odbierania wiadomości w rejestracji");
            exit(1);
        }

        printf("Rejestracja: Odebrano zgłoszenie pacjenta %d (VIP: %d, Wiek: %d).\n",
               message.id, message.is_vip, message.age);

        // Symulacja czasu rejestracji
        sleep(1);

        // Przygotowanie i wysłanie potwierdzenia rejestracji
        confirmation.type = message.id;
        confirmation.id = message.id;

        if (msgsnd(queue_id, &confirmation, sizeof(confirmation) - sizeof(long), 0) == -1) {
            perror("Błąd wysyłania potwierdzenia rejestracji");
        } else {
            printf("Rejestracja: Zarejestrowano pacjenta %d.\n", message.id);
        }
    }
}
