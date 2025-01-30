#include "rejestracja.h"

// identyfikator kolejki komunikatow
// extern int queue_id jest w pacjent.h

// Funkcja obslugujaca rejestracje
void registration_process() {
    Message msg;
    Confirmation conf;

    printf("\nRejestracja: Oczekiwanie na pacjentow...\n\n");

    while (1) {
        if (msgrcv(queue_id, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            // bo msg_sz – rozmiar komunikatu na ktory wskazuje msg_ptr bez wartosci long int;
            printf("Rejestracja: Pacjent %d (VIP: %d, Wiek: %d) zglosil sie.\n", msg.id, msg.is_vip, msg.age);

            // Wyslanie potwierdzenia rejestracji
            conf.type = msg.id;
            conf.id = msg.id;
        sleep(6);
            if (msgsnd(queue_id, &conf, sizeof(Confirmation) - sizeof(long), 0) == -1) {
                perror(" Blad wysylania potwierdzenia\n");
            }
        } 
        else if (errno != EINTR) {
            perror(" Blad odbierania wiadomosci\n");
        }
    }
}
