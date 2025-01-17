#include "pacjent.h"
#include "rejestracja.h"
#include "funkcje_porzadkowe.h"

int main(int argc, char **argv) {
    // Inicjalizacja generatora liczb pseudolosowych
    srand(time(NULL));

    // Rejestracja obsługi sygnału SIGINT (Ctrl+C)
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Błąd przy rejestracji obsługi sygnału SIGINT");
        exit(1);
    }

    //Funkcja signal() rejestruje funkcję signal_handler 
    //jako obsługującą sygnał SIGINT, który jest wysyłany, gdy użytkownik naciśnie Ctrl+C.
    initialize_semaphores();
    initialize_message_queue();

    pid_t pid = fork();
    if (pid == 0) {
        // Proces rejestracji
        registration_process();
    } else if (pid > 0) {
        // Proces generujący pacjentów
        generate_patients();
        
        // Pętla w nieskończoność generująca pacjentów
        while (keep_generating) {
            sleep(1);  // Opcjonalnie można dodać mały sen w tej pętli, żeby nie obciążać CPU
        }

    } else {
        perror(" Błąd fork\n");
        exit(1);
    }
    while (wait(NULL) > 0);
    printf("System zakończył pracę.\n");
    // W przypadku zakończenia generowania pacjentów lub wyjścia z pętli

    return 0;
}