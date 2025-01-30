#include "pacjent.h"
#include "rejestracja.h"
#include "funkcje_porzadkowe.h"

int main(int argc, char **argv) {
    // Inicjalizacja generatora liczb pseudolosowych
    srand(time(NULL));

    //Funkcja signal() rejestruje funkcje signal_handler 
    //jako obslugujaca sygnal SIGINT, ktory jest wysylany, gdy uzytkownik nacisnie Ctrl+C.
    initialize_semaphores();
    initialize_message_queue();

    // Rejestracja obslugi sygnalu SIGINT (Ctrl+C)
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Blad przy rejestracji obslugi sygnalu SIGINT");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Proces rejestracji
        registration_process();
    } else if (pid > 0) {
        
        // Petla w nieskonczonosc generujaca pacjentow
        while (keep_generating){
            generate_patients();    // Proces generujacy pacjentow
        }

    } else {
        perror(" Blad fork\n");
        exit(1);
    }
    while (wait(NULL) > 0);
    printf("System zakonczyl prace.\n");
    // W przypadku zakonczenia generowania pacjentow lub wyjscia z petli

    return 0;
}