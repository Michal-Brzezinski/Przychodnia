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

void print_fflush(const char *tekst) {
    /* Wyświetla tekst z funkcjonalnością fflush */
    printf("%s\n", tekst);
    fflush(stdout);
}


void printRed(const char *tekst) {
    /* Wyświetla tekst w kolorze czerwonym */
    printf("%s%s%s\n", czerwony, tekst, reset);
    fflush(stdout);
}

void printBlue(const char *tekst) {
    /* Wyświetla tekst w kolorze niebieskim */
    printf("%s%s%s\n", niebieski, tekst, reset);
    fflush(stdout);
}

void printGreen(const char *tekst) {
    /* Wyświetla tekst w kolorze zielonym */
    printf("%s%s%s\n", zielony, tekst, reset);
    fflush(stdout);
}

void printYellow(const char *tekst) {
    /* Wyświetla tekst w kolorze żółtym */
    printf("%s%s%s\n", zolty, tekst, reset);
    fflush(stdout);
}