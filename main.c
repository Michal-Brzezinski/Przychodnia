#include "pacjent.h"

int main(int argc, char **argv) {
    // Inicjalizacja generatora liczb losowych
    srand(time(NULL));

    // Inicjalizacja semaforów System V
    initialize_semaphores();

    printf("Wstępna próba wygenerowania pacjentów.\n");

    // Generowanie pacjentów
    generate_patients(6);

    // Usuwanie semaforów po zakończeniu
    cleanup_semaphores();

    printf("\nWszystkie procesy zakończone.\n");

    return 0;
}
