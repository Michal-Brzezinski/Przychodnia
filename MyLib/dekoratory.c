#include "dekoratory.h"


int losuj_int(int N) {
    /*Sposob losowania niezalezne od czasu i w miare mozliwosci niedeterministyczny*/
    
    int losowy;
    unsigned int seed;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1; // Obsluz blad otwarcia pliku
    }
    read(fd, &seed, sizeof(seed)); // Pobierz losowe dane z /dev/urandom
    close(fd);

    srand(seed);
    losowy = rand() % (N + 1);
    return losowy;
}

int procentNaNaturalna(int n, int x) {
    /*Funckcja oblicza x% liczby n, zwracajac wynik jako liczbe calkowita*/
    double procent = (double)x / 100.0; // Konwersja procentow na ulamek
    double s = floor(n * procent); 
    // Obliczenie x% liczby n i zaokraglenie do najnizszej liczby calkowitej
    // To gwarantuje nie wyjsc poza zakres podany w argumencie funkcji
    return (int)s; // Zwrocenie jako liczba calkowita
}

key_t generuj_klucz_ftok(const char *sciezka, char projek_id) { 
    /* Sprawdza czy zaistnial blad, jezeli tak to konczy proces z kodem 1 */
    /* Jezeli nie, to zwraca wartosc wygenerowanego klucza */

    key_t klucz; 

    if ((klucz = ftok(sciezka, projek_id)) == -1){ 
        printf("Blad ftok\n"); 
        exit(1); 
    }

    return klucz; 
}

void print(const char *format, ...) {
    // Wyswietla tekst z funkcjonalnoscia fflush
    fflush(stdout);
    // Wyswietla tekst z funkcjonalnoscia fflush 
    // Zmienna lista argumentow do printf
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    fflush(stdout);
    va_end(args);
}

void printRed(const char *format, ...) {
    // funkcja drukujaca w kolorze czerwonym
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywołujemy vprintf z kodami kolorów
    printf("%s", czerwony);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjścia
}

void printBlue(const char *format, ...) {
    // funkcja drukujaca w kolorze niebieskim
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywołujemy vprintf z kodami kolorów
    printf("%s", niebieski);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjścia
}

void printGreen(const char *format, ...) {
    // funkcja drukujaca w kolorze zielonym
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywołujemy vprintf z kodami kolorów
    printf("%s", zielony);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjścia
}

void printYellow(const char *format, ...) {
    // funkcja drukujaca w kolorze zoltym
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywołujemy vprintf z kodami kolorów
    printf("%s", zolty);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjścia
}

void printCyan(const char *format, ...) {
    // funkcja drukujaca w kolorze cyjan 
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywołujemy vprintf z kodami kolorów
    printf("%s", cyjan);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjścia
}

void printMagenta(const char *format, ...) {
    // Ustawiamy kolor tekstu na magenta i przygotowujemy kolorowanie
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywołujemy vprintf z kodami kolorów
    printf("%s", magenta);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjścia
}


void perror_red(const char *s) {
    // funkcja drukujaca komunikat bledu w kolorze czerwonym
    fflush(stdout);
    // Ustawiamy kolor na czerwony
    printf("%s", czerwony);
    fflush(stdout);

    // Jesli uzytkownik podal komunikat, drukujemy go
    if (s) {
        printf("%s: ", s);
        fflush(stdout);
    }

    // Drukujemy komunikat bledu zgodnie z errno
    printf("%s", strerror(errno));
    fflush(stdout);


    // Resetujemy kolor
    printf("%s", reset);
    fflush(stdout);
}

void oczekujNaProces(pid_t pid, const char *nazwa_procesu) {
    
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        print("[Main]: Proces %s zakonczony z kodem %d.\n", nazwa_procesu, WEXITSTATUS(status));
    else
        print("[Main]: Proces %s zakonczony niepowodzeniem.\n", nazwa_procesu);

}

void wyczyscProcesyPacjentow()
{
system("killall pacjent");
}

void usunNiepotrzebnePliki(){
system("bash czystka.sh");
}