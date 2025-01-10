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

#define MAX_PATIENTS 100 // Maksymalna liczba pacjentów do wygenerowania w ciągu dnia
#define BUILDING_CAPACITY 50 // Maksymalna liczba pacjentów w przychodni

// Klucz i identyfikator semafora
extern key_t building_key;
extern int building_sem_id;

// Struktura reprezentująca pacjenta
typedef struct {
    int id;
    int is_vip; // 1 jeśli VIP, 0 jeśli nie
    int age;    // Wiek pacjenta
} Patient;

// Funkcje
void patient_process(Patient patient);
void generate_patients(int num_patients);
void initialize_semaphores();
void cleanup_semaphores();
