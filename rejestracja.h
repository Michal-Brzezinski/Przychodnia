#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>

// Definicja maksymalnych rozmiarów kolejki
#define MAX_QUEUE 1000

// Struktura pacjenta w rejestracji
typedef struct {
    long mtype; // Typ wiadomości (1: zwykły pacjent, 2: VIP)
    int id;     // ID pacjenta
    int is_vip; // 1 jeśli VIP, 0 jeśli nie
} RegistrationMessage;

// Funkcje
void registration_process(int max_capacity);
