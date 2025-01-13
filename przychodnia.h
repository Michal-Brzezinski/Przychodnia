#pragma once
#include "pacjent.h"


// Struktura reprezentująca przychodnię
typedef struct {
    int max_patients;  // Maksymalna liczba pacjentów w przychodni
    int current_patients; // Liczba pacjentów w danej chwili
    int sem_id;  // Identyfikator zbioru semaforów
    int queue_id;  // Identyfikator kolejki komunikatów

    // Inne zasoby, jak liczba pacjentów w kolejce do rejestracji, otwarte okienka rejestracji itp.
} Clinic;

void open_clinic();
void close_clinic();
void run_clinic(Clinic *clinic);
void manage_patients(Clinic *clinic);