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

#define MAX_PATIENTS 100 // Maksymalna liczba pacjentow w przychodni (N)- dac do funkcji przychodni
#define BUILDING_CAPACITY 10 // Maksymalna liczba pacjentow w przychodni
#define HOW_MUCH_PATIENTS 3 // Ilu pacjentow wygenerowac

extern time_t Tp, Tk; // globalne zmienne godzin pracy przychodni (poki co czas w ktorym generowani sa pacjenci)  

// Klucz i identyfikator semafora
extern key_t building_key;
extern int building_sem_id;

// Klucz i identyfikator kolejki komunikatow
extern key_t queue_key;
extern int queue_id;

// Struktura wiadomosci w kolejce 
typedef struct {
    long type;    // Typ wiadomosci (priorytet: 1 dla VIP, 2 dla zwyklych pacjentow)
    int id;       // ID pacjenta
    int age;      // Wiek pacjenta
    int is_vip;   // 1 jesli VIP, 0 jesli nie
} Message;

// Struktura reprezentujaca pacjenta
typedef struct {
    int id;
    int is_vip; // 1 jesli VIP, 0 jesli nie
    int age;    // Wiek pacjenta
} Patient;

// Wiadomosc potwierdzajaca dla pacjenta, czy zostal przyjety
typedef struct {
    long type;  // ID pacjenta (odpowiadajacy `patient.id`)
    int id;     // Identyfikator pacjenta
} Confirmation;



// Funkcje
void patient_process(Patient patient);
void generate_patients();

void initialize_semaphores();
void initialize_message_queue();