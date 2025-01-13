#include "rejestracja.h"

// identyfikator kolejki komunikatów
// extern int queue_id jest w pacjent.h

// Funkcja obsługująca rejestrację
void registration_process() {
    Message msg;
    Confirmation conf;
    int patients_admissioned=0;

    printf("\nRejestracja: Oczekiwanie na pacjentów...\n\n");

    while (patients_admissioned < MAX_ADMISSION) {
        if (msgrcv(queue_id, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            printf("Rejestracja: Pacjent %d (VIP: %d, Wiek: %d) zgłosił się.\n", msg.id, msg.is_vip, msg.age);

            // Wysłanie potwierdzenia rejestracji
            conf.type = msg.id;
            conf.id = msg.id;

            if (msgsnd(queue_id, &conf, sizeof(Confirmation) - sizeof(long), 0) == -1) {
                perror(" Błąd wysyłania potwierdzenia\n");
            }
            patients_admissioned++;
        } 
        else if (errno != EINTR) {
            perror(" Błąd odbierania wiadomości\n");
        }
    }
}
