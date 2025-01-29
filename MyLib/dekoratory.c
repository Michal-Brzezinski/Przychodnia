#include "dekoratory.h"


int losuj_int(int N) {
    /*Sposób losowania niezależne od czasu i w miarę możliwości niedeterministyczny*/
    
    int losowy;
    unsigned int seed;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1; // Obsłuż błąd otwarcia pliku
    }
    read(fd, &seed, sizeof(seed)); // Pobierz losowe dane z /dev/urandom
    close(fd);

    srand(seed);
    losowy = rand() % (N + 1);
    return losowy;
}

int procentNaNaturalna(int n, int x) {
    /*Funckcja oblicza x% liczby n, zwracając wynik jako liczbę całkowitą*/
    double procent = (double)x / 100.0; // Konwersja procentów na ułamek
    double s = floor(n * procent); 
    // Obliczenie x% liczby n i zaokrąglenie do najniższej liczby całkowitej
    // To gwarantuje nie wyjść poza zakres podany w argumencie funkcji
    return (int)s; // Zwrócenie jako liczba całkowita
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