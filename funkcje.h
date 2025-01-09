#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>

#define MAX_PATIENTS 100 // Maksymalna liczba pacjentow do wygenerowania w ciagu dnia
#define BUILDING_CAPACITY 50 // Maksymalna liczba pacjentow w przychodni

sem_t building_sem; // Semafor kontrolujacy liczbe pacjentow w budynku


// Struktura reprezentujaca pacjenta
typedef struct {
    int id;
    int is_vip; // 1 jesli VIP, 0 jesli nie
    int age;    // Wiek pacjenta
} Patient;



// =============== FUNKCJA OPISUJACA ZACHOWANIE PACJENTA ===============


void patient_process(Patient patient) {
    printf("Pacjent %d (VIP: %d, Wiek: %d) probuje wejsc do budynku.\n", 
           patient.id, patient.is_vip, patient.age);

    // Proba wejscia do budynku
    if (sem_trywait(&building_sem) == 0) {
        printf("Pacjent %d wszedl do budynku.\n", patient.id);
        sleep(rand() % 3 + 1); // Symulacja czasu spedzonego w budynku
        printf("Pacjent %d opuszcza budynek.\n", patient.id);
        sem_post(&building_sem); // Zwalnianie miejsca w budynku
    } else {
        printf("Pacjent %d nie mogl wejsc do budynku - brak miejsca.\n", patient.id);
    }

    exit(0); // Proces pacjenta konczy dzialanie
}



// ================= FUNKCJA GENERUJACA PACJENTOW =================


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
            perror("BLAD FORK");
            exit(1);
        }

        // Mala przerwa przed generowaniem kolejnego pacjenta
        usleep(50000); // 50 ms
    }

    // Oczekiwanie na zakonczenie wszystkich procesow pacjentow
    while (wait(NULL) > 0);
}

