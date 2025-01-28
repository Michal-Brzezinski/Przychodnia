#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "MyLib/dekoratory.h"

// zorientować się od jakiego numeru zaczyna się numerowanie lekarzy w losowaniu w pacjencie 
enum lekarze{ 
    POZ = 1, 
    KARDIOLOG, 
    OKULISTA, 
    PEDIATRA, 
    MEDYCYNA_PRACY
};

typedef struct{

    int id_lekarz;
    char nazwa[128];
    int licznik_pacjentow;
    int indywidualny_limit;

}Lekarz;

void inicjalizuj_lekarza(Lekarz* lekarz, int id_lekarz, int limit_pacjentow){

    lekarz->id_lekarz = id_lekarz;
    lekarz->licznik_pacjentow = 0;
    lekarz->indywidualny_limit = limit_pacjentow;

    switch (id_lekarz)
    {
    case POZ:
        sprintf(lekarz->nazwa, "Lekarz POZ");
        break;
    case KARDIOLOG:
        sprintf(lekarz->nazwa, "Kardiolog");
        break;
    case OKULISTA:
        sprintf(lekarz->nazwa, "Okulista");
        break;
    case PEDIATRA:
        sprintf(lekarz->nazwa, "Pediatra");
        break;
    case MEDYCYNA_PRACY:
        sprintf(lekarz->nazwa, "Lekarz Medycyny Pracy");
        break;
    default:
        break;
    }

}
