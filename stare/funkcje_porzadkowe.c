#include "funkcje_porzadkowe.h"
#include "pacjent.h"

volatile int keep_generating;   // volatile oznacza, zeby kompilator nie optymalizowal
                                // uzywane w sytuacjach gdy wartosc zmiennej jest zmieniana np przez sygnal

void cleanup_message_queue() {
    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        perror("Blad msgctl (usuwanie kolejki)");}
} 


void cleanup_semaphores() {     // usuniecie zbioru semaforow
    if (semctl(building_sem_id, 0, IPC_RMID) == -1) {
        perror("Blad semctl (IPC_RMID)");}
}


void signal_handler(int sig) {
    
    // Jesli otrzymamy sygnal SIGINT (Ctrl+C), zatrzymujemy generowanie pacjentow i zwalniamy zasoby

    // Zatrzymanie generowania pacjentow
    keep_generating = 0;

    // Czekanie na zakonczenie wszystkich procesow potomnych
    while (wait(NULL) > 0);  // Oczekiwanie na zakonczenie procesow potomnych



    exit(0);  // Zakonczenie programu
}