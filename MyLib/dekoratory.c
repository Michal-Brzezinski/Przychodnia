#include "dekoratory.h"


int losuj_int(int N) { 
    /* Zwraca losową wartość z zakresu od 0 do N */

    srand(time(0) ^ getpid()); // Inicjalizacja generatora liczb losowych
    return rand() % (N + 1); 
}


key_t generuj_klucz_ftok(const char *sciezka, char projek_id) { 
    /* Sprawdza czy zaistniał błąd, jeżeli tak to kończy proces z kodem 1 */
    /* Jeżeli nie, to zwraca wartość wygenerowanego klucza */

    key_t klucz; 

    if ((klucz = ftok(sciezka, projek_id)) == -1){ 
        printf("Błąd ftok\n"); 
        exit(1); 
    }

    return klucz; 
}