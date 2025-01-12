#include "pacjent.h"

// Klucz i identyfikator semafora
key_t building_key;
int building_sem_id;

void initialize_semaphores() {
    building_key = ftok(".", 'B');  // utworzenie klucza
    if (building_key == -1) {
        perror("Błąd ftok");
        exit(1);
    }

    // utworzenie zbioru semaforow i pobranie identyfikator
    building_sem_id = semget(building_key, 1, IPC_CREAT | 0666);    
    if (building_sem_id == -1) {
        perror("Błąd semget");
        exit(1);
    }

    if (semctl(building_sem_id, 0, SETVAL, BUILDING_CAPACITY) == -1) {  // inicjalizacja semafora
        perror("Błąd semctl (SETVAL)");
        exit(1);
    }
}

void cleanup_semaphores() {     // usuniecie zbioru semaforow
    if (semctl(building_sem_id, 0, IPC_RMID) == -1) {
        perror("Błąd semctl (IPC_RMID)");
    }
}

void patient_process(Patient patient) {
    struct sembuf sem_op;

    printf("Pacjent %d (VIP: %d, Wiek: %d) próbuje wejść do budynku.\n",
           patient.id, patient.is_vip, patient.age);

    // Próba wejścia do budynku
    sem_op.sem_num = 0;
    sem_op.sem_op = -1; // Opuszczenie semafora
    sem_op.sem_flg = IPC_NOWAIT;

    if (semop(building_sem_id, &sem_op, 1) == 0) {
        printf("Pacjent %d wszedł do budynku.\n", patient.id);
        sleep(rand() % 3 + 1); // Symulacja czasu spędzonego w budynku
        printf("Pacjent %d opuszcza budynek.\n", patient.id);

        // Zwalnianie miejsca w budynku
        sem_op.sem_op = 1; // Podniesienie semafora
        if (semop(building_sem_id, &sem_op, 1) == -1) {
            perror("Błąd semop (podniesienie)");
        }
    } else {
        printf("Pacjent %d nie mógł wejść do budynku - brak miejsca.\n", patient.id);
    }

    exit(0); // Proces pacjenta kończy działanie
}

void generate_patients(int num_patients) {
    for (int i = 0; i < num_patients; i++) {
        Patient patient;
        patient.id = i + 1;
        patient.is_vip = rand() % 2; // 50% szans na bycie VIP
        patient.age = rand() % 100 + 1; // Wiek od 1 do 100 lat

        pid_t pid = fork();
        if (pid == 0) {
            // Proces potomny (pacjent)
            patient_process(patient);
        } else if (pid < 0) {
            perror("Błąd fork");
            exit(1);
        }

        // Mała przerwa przed generowaniem kolejnego pacjenta
        usleep(50000); // 50 ms
    }

    // Oczekiwanie na zakończenie wszystkich procesów pacjentów
    while (wait(NULL) > 0);
}
