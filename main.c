#include "pacjent.h"
#include "rejestracja.h"

int main(int argc, char **argv) {
    // Inicjalizacja generatora liczb pseudolosowych
    srand(time(NULL)); 

    // Inicjalizacja semaforów i kolejki komunikatów
    initialize_semaphores();
    initialize_message_queue();

    printf("System rejestracji i obsługi pacjentów został uruchomiony.\n");

    // Generowanie procesów pacjentów
    int num_patients = 3;  // Liczba pacjentów do wygenerowania
    generate_patients(num_patients);
    registration_process();
    // Czyszczenie zasobów IPC po zakończeniu działania systemu
    cleanup_semaphores();
    cleanup_message_queue();

    printf("System zakończył pracę.\n");

    printf("\nWszystkie procesy zakończone.\n");

    return 0;
}
