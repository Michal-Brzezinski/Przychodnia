#include "funkcje_porzadkowe.h"
#include "pacjent.h"

volatile int keep_generating;

void cleanup_message_queue() {
    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        perror("Błąd msgctl (usuwanie kolejki)");}
} 


void cleanup_semaphores() {     // usuniecie zbioru semaforow
    if (semctl(building_sem_id, 0, IPC_RMID) == -1) {
        perror("Błąd semctl (IPC_RMID)");}
}

void cleanup() {

    cleanup_message_queue();
    cleanup_semaphores();
}

void signal_handler(int sig) {
    
    // Jeśli otrzymamy sygnał SIGINT (Ctrl+C), zatrzymujemy generowanie pacjentów i zwalniamy zasoby

    // Zatrzymanie generowania pacjentów
    keep_generating = 0;

    // Zwalnianie zasobów
    cleanup();

    // Czekanie na zakończenie wszystkich procesów potomnych
    while (wait(NULL) > 0);  // Oczekiwanie na zakończenie procesów potomnych

    exit(0);  // Zakończenie programu
}