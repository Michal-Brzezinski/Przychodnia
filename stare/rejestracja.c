#include "rejestracja.h"

// identyfikator kolejki komunikatów
// extern int queue_id jest w pacjent.h

// Funkcja obsługująca rejestrację
void registration_process() {
    Message msg;
    Confirmation conf;

    printf("\nRejestracja: Oczekiwanie na pacjentów...\n\n");

    while (1) {
        if (msgrcv(queue_id, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            // bo msg_sz – rozmiar komunikatu na który wskazuje msg_ptr bez wartości long int;
            printf("Rejestracja: Pacjent %d (VIP: %d, Wiek: %d) zgłosił się.\n", msg.id, msg.is_vip, msg.age);

            // Wysłanie potwierdzenia rejestracji
            conf.type = msg.id;
            conf.id = msg.id;
        sleep(6);
            if (msgsnd(queue_id, &conf, sizeof(Confirmation) - sizeof(long), 0) == -1) {
                perror(" Błąd wysyłania potwierdzenia\n");
            }
        } 
        else if (errno != EINTR) {
            perror(" Błąd odbierania wiadomości\n");
        }
    }
}
