#include "przychodnia.h"


void open_clinic(Clinic *clinic) {
    // Inicjalizacja semaforów, liczby pacjentów i innych zasobów
    clinic->sem_id = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);  // Przykładowe semafory
    if (clinic->sem_id == -1) {
        perror("Błąd semget");
        exit(1);
    }

    // Semafor 0 - liczba pacjentów w budynku
    if (semctl(clinic->sem_id, 0, SETVAL, 0) == -1) {
        perror("Błąd semctl dla liczby pacjentów");
        exit(1);
    }

    // Semafor 1 - dostęp do kontrolowania liczby pacjentów (np. maksymalna liczba pacjentów w budynku)
    if (semctl(clinic->sem_id, 1, SETVAL, 1) == -1) {
        perror("Błąd semctl dla dostępu do liczby pacjentów");
        exit(1);
    }

    // Semafor 2 - pojemność budynku
    if (semctl(clinic->sem_id, 2, SETVAL, BUILDING_CAPACITY) == -1) {
        perror("Błąd semctl dla pojemności budynku");
        exit(1);
    }

    // Inicjalizacja kolejki komunikatów
    clinic->queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (clinic->queue_id == -1) {
        perror("Błąd msgget");
        exit(1);
    }

    // Możesz także zainicjalizować inne zasoby związane z przychodnią
    clinic->max_patients = MAX_PATIENTS;
    clinic->current_patients = 0;

    // Możesz także rozpocząć symulację działania przychodni
    printf("Przychodnia uruchomiona.\n");
}


void close_clinic(Clinic *clinic) {
    // Usunięcie semaforów
    if (semctl(clinic->sem_id, 0, IPC_RMID) == -1) {
        perror("Błąd semctl (IPC_RMID)");
    }

    // Usunięcie kolejki komunikatów
    if (msgctl(clinic->queue_id, IPC_RMID, NULL) == -1) {
        perror("Błąd msgctl (IPC_RMID)");
    }

    printf("Przychodnia zamknięta.\n");
}

void manage_patients(Clinic *clinic) {
    struct sembuf sem_op;

    // Sprawdzenie dostępności miejsca w przychodni
    sem_op.sem_num = 0;  // Semafor dla liczby pacjentów
    sem_op.sem_op = 1;   // Zwiększenie liczby pacjentów w budynku
    sem_op.sem_flg = 0;

    if (semop(clinic->sem_id, &sem_op, 1) == -1) {
        printf("Brak miejsca w budynku, pacjent nie może wejść.\n");
        return;
    }

    clinic->current_patients++;  // Zwiększamy liczbę pacjentów w budynku

    // Tutaj możesz dodać dalszą logikę: np. rejestracja, wybór lekarza, badania itp.
    // Symulacja wizyty pacjenta, dodanie go do odpowiednich kolejek lekarzy, itp.

    // Na koniec opuszczenie budynku
    sem_op.sem_op = -1;  // Zmniejszenie liczby pacjentów
    semop(clinic->sem_id, &sem_op, 1);

    clinic->current_patients--;  // Zmniejszamy liczbę pacjentów w budynku
}


void run_clinic(Clinic *clinic) {
    // Przykładowa symulacja: pacjent generowany co 1 sekundę
    while (time(NULL) < Tk) {  // Zakładając, że mamy zmienne Tp i Tk
        // Generowanie pacjenta (tu np. wywołanie funkcji generate_patient())
        // Pacjent wchodzi do przychodni, przechodzi rejestrację, wybór lekarza itd.

        manage_patients(clinic);

        sleep(1);  // Odstęp między pacjentami
    }

    close_clinic(clinic);  // Na końcu zamykamy przychodnię
}

