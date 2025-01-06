#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>

#define MAX_PATIENTS 100 // Maksymalna liczba pacjentow do wygenerowania w ciagu dnia
#define BUILDING_CAPACITY 50 // Maksymalna liczba pacjentow w przychodni

sem_t building_sem; // Semafor kontrolujacy liczbe pacjentow w budynku


// Struktura reprezentujaca pacjenta
typedef struct {
    int id;
    int is_vip; // 1 jesli VIP, 0 jesli nie
    int age;    // Wiek pacjenta
} Patient;