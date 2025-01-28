#include "lekarz.h"


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("\033[1;31m[Lekarz]: Nieprawidłowa liczba argumentów\033[0m\n");
        exit(1);
    }
    Lekarz lekarz;
    inicjalizuj_lekarza(&lekarz, *argv[1], 10);
    printf("Przyjęty argument: %d", *argv[1]);
    printf("Lekarz nr %d\n", lekarz.id_lekarz);
    return 0;
}
