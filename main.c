#include "pacjent.h"

int main(int argc, char **argv){

    srand(time(NULL));
    sem_init(&building_sem, 0, BUILDING_CAPACITY);


    printf("Wstepna proba wygenerowania pacjentow");

    generate_patients(6);

    sem_destroy(&building_sem);

    return 0;
}