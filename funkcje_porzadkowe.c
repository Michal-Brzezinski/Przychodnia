#include "funkcje_porzadkowe.h"
#include "pacjent.h"

volatile int keep_generating;   // volatile oznacza, żeby kompilator nie optymalizował
                                // używane w sytuacjach gdy wartość zmiennej jest zmieniana np przez sygnał

void cleanup_message_queue() {
    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        perror("Błąd msgctl (usuwanie kolejki)");}
} 


void cleanup_semaphores() {     // usuniecie zbioru semaforow
    if (semctl(building_sem_id, 0, IPC_RMID) == -1) {
        perror("Błąd semctl (IPC_RMID)");}
}


void signal_handler(int sig) {
    
    // Jeśli otrzymamy sygnał SIGINT (Ctrl+C), zatrzymujemy generowanie pacjentów i zwalniamy zasoby

    // Zatrzymanie generowania pacjentów
    keep_generating = 0;

    // Czekanie na zakończenie wszystkich procesów potomnych
    while (wait(NULL) > 0);  // Oczekiwanie na zakończenie procesów potomnych

    // Zwalnianie zasobów
    cleanup_semaphores();
    cleanup_message_queue();

    exit(0);  // Zakończenie programu
}