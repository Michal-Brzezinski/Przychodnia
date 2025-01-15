#pragma once
#include "pacjent.h"

// Funkcja do operacji na semaforze
void semaphore_operation(int sem_id, int sem_num, int operation) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = operation;
    sops.sem_flg = 0;
    if (semop(sem_id, &sops, 1) == -1) {
        perror("semop");
        exit(1);
    }
}
