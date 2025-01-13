#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>

#define MAX_PATIENTS 100 // Maksymalna liczba pacjentów do wygenerowania w ciągu dnia
#define BUILDING_CAPACITY 50 // Maksymalna liczba pacjentów w przychodni
#define MAX_ADMISSION 3 // Maksymalna liczba osób do zarejestrowania
#define HOW_MUCH_PATIENTS 3 // Ilu pacjentów wygenerować

// Klucz i identyfikator semafora
extern key_t building_key;
extern int building_sem_id;

// Klucz i identyfikator kolejki komunikatów
extern key_t queue_key;
extern int queue_id;

// Struktura wiadomości w kolejce 
typedef struct {
    long type;    // Typ wiadomości (priorytet: 1 dla VIP, 2 dla zwykłych pacjentów)
    int id;       // ID pacjenta
    int age;      // Wiek pacjenta
    int is_vip;   // 1 jeśli VIP, 0 jeśli nie
} Message;

// Struktura reprezentująca pacjenta
typedef struct {
    int id;
    int is_vip; // 1 jeśli VIP, 0 jeśli nie
    int age;    // Wiek pacjenta
} Patient;

// Wiadomość potwierdzająca dla pacjenta, czy został przyjęty
typedef struct {
    long type;  // ID pacjenta (odpowiadający `patient.id`)
    int id;     // Identyfikator pacjenta
} Confirmation;



// Funkcje
void patient_process(Patient patient);
void generate_patients(int num_patients);

void initialize_semaphores();
void cleanup_semaphores();

void initialize_message_queue();
void cleanup_message_queue();