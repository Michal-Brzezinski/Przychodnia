#pragma once
#include "pacjent.h"

// Funkcje do zarządzania przychodnią
void open_clinic(int Tp, int Tk, int max_patients);  // Tp - start, Tk - koniec

void close_clinic();

void handle_sigint(int sig);
 