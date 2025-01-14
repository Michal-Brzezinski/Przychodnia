#include "pacjent.h"
#include "rejestracja.h"

int main(int argc, char **argv) {
    // Inicjalizacja generatora liczb pseudolosowych
    srand(time(NULL));

    initialize_semaphores();
    initialize_message_queue();

    pid_t pid = fork();
    if (pid == 0) {
        // Proces rejestracji
        registration_process();

    } else if (pid > 0) {
        // Proces generujący pacjentów
        generate_patients(HOW_MUCH_PATIENTS);
        // Sprzątanie po zakończeniu
        cleanup_semaphores();
        cleanup_message_queue();
    } else {
        perror(" Błąd fork\n");
        exit(1);
    }
    while (wait(NULL) > 0);
    printf("System zakończył pracę.\n");

    ///printf("\nWszystkie procesy zakończone.\n");

    return 0;
}
