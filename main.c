#include "pacjent.h"


sem_t building_sem;

int main(int argc, char **argv){

    // Inicjalizacja generatora liczb losowych
    srand(time(NULL));

    // Inicjalizacja semafora
    if (sem_init(&building_sem, 0, BUILDING_CAPACITY) != 0) {
        perror("\nBlad inicjalizacji semafora\n");
        return 1;
    }


    printf("Wstepna proba wygenerowania pacjentow\n");

    generate_patients(6);

    
    // Zniszczenie semafora po zakończeniu
    if (sem_destroy(&building_sem) != 0) {
        perror("\nBłąd niszczenia semafora\n");
        return 1;
    }

    printf("\nWszystkie procesy zakończone.\n");

    return 0;
}