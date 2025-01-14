#include "przychodnia.h"


static int clinic_is_open = 0;  // Flaga otwarcia przychodni

void clinic_open(int Tp, int Tk, int max_patients) {
    clinic_is_open = 1;  // Otwieramy przychodnię
    time_t start_time = time(NULL);  // Czas rozpoczęcia
    time_t end_time = start_time + (Tk - Tp);  // Obliczamy czas zamknięcia

    printf("Przychodnia otwarta od %d do %d sekund działania.\n", Tp, Tk);

    while (clinic_is_open && time(NULL) < end_time) {
        generate_patients(max_patients);  // Generowanie pacjentów
    }

    printf("Przychodnia zamknięta.\n");
}

void close_clinic() {
    clinic_is_open = 0;  // Zamykamy przychodnię
}

void handle_sigint(int sig) {
    printf("\nZamykanie systemu...\n");
    cleanup_semaphores();
    cleanup_message_queue();
    exit(0);
}