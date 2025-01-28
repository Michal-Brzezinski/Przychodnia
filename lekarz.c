#include "lekarz.h"


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("\033[1;31m[Lekarz]: Nieprawidłowa liczba argumentów\033[0m\n");
        exit(1);
    }
    Lekarz lekarz;
    int id_lekarz = atoi(argv[1]);
    int limit_pacjentow = atoi(argv[2]);

    int limit_indywidualny = 0;
    switch(id_lekarz){
        case POZ:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 30);
            break;
        case KARDIOLOG:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        case OKULISTA:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        case PEDIATRA:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        case MEDYCYNA_PRACY:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        default:
            break;
    }
    inicjalizuj_lekarza(&lekarz, id_lekarz, limit_indywidualny);

    printf("\033[1;35m[Lekarz]: Wygenerowano lekarza: %s o id: %d, limit pacjentów: %d\033[0m\n", lekarz.nazwa, lekarz.id_lekarz, lekarz.indywidualny_limit);

    return 0;
}


void czynnosci_lekarskie(Lekarz *lekarz){



}