#include "utils.h"


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
    // Wywolujemy vprintf z kodami kolorow
    printf("%s", czerwony);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjscia
}

void printBlue(const char *format, ...) {
    // funkcja drukujaca w kolorze niebieskim
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywolujemy vprintf z kodami kolorow
    printf("%s", niebieski);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjscia
}

void printGreen(const char *format, ...) {
    // funkcja drukujaca w kolorze zielonym
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywolujemy vprintf z kodami kolorow
    printf("%s", zielony);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjscia
}

void printYellow(const char *format, ...) {
    // funkcja drukujaca w kolorze zoltym
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywolujemy vprintf z kodami kolorow
    printf("%s", zolty);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjscia
}

void printCyan(const char *format, ...) {
    // funkcja drukujaca w kolorze cyjan 
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywolujemy vprintf z kodami kolorow
    printf("%s", cyjan);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjscia
}

void printMagenta(const char *format, ...) {
    // Ustawiamy kolor tekstu na magenta i przygotowujemy kolorowanie
    va_list args;
    va_start(args, format);

    fflush(stdout);
    // Wywolujemy vprintf z kodami kolorow
    printf("%s", magenta);
    vprintf(format, args);
    printf("%s", reset);

    va_end(args);
    fflush(stdout); // Wymusza natychmiastowe wypisanie do standardowego wyjscia
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

void czekaj_na_procesy(pid_t *pid_array, int size) {
    for (int i = 0; i < size; i++) {
        if (pid_array[i] > 0) {
            waitpid(pid_array[i], NULL, 0);
        }
    }
}

void wyczyscProcesyPacjentow()
{
system("killall pacjent > /dev/null 2>&1");
}

void usunNiepotrzebnePliki(){
system("bash czystka.sh");
}

void zwrocTabliceLimitowLekarzy(int limit_pacjentow, int *limity_lekarzy){
//  Funkcja przydzielajaca dana liczbe pacjentow pomiedzy lekarzy
//  Zwraca wskaznik do tablicy limitow:
//  - limity_lekarzy[0] - limit pacjentow dla lekarza 1
//  - limity_lekarzy[1] - limit pacjentow dla lekarza 2
//  - limity_lekarzy[2] - limit pacjentow dla lekarza 3
//  - limity_lekarzy[3] - limit pacjentow dla lekarza 4
//  - limity_lekarzy[4] - limit pacjentow dla lekarza 5

    // Deklaracja tablic i zmiennych
    double procenty[5] = {60.0, 10.0, 10.0, 10.0, 10.0};
    double limity_lekarzy_double[5];
    double reszty[5];
    int suma_limity = 0;
    int i;

    // Oblicz dokladne limity jako liczby zmiennoprzecinkowe
    for (i = 0; i < 5; i++) {
        limity_lekarzy_double[i] = (limit_pacjentow * procenty[i]) / 100.0;
        limity_lekarzy[i] = (int)limity_lekarzy_double[i]; // Czesc calkowita
        reszty[i] = limity_lekarzy_double[i] - limity_lekarzy[i]; // Reszta
        suma_limity += limity_lekarzy[i];   // suma calkowita obliczonych limitow
    }

    // Oblicz pozostalych pacjentow do przydzielenia
    int pozostalo_pacjentow = limit_pacjentow - suma_limity;

    // Przydziel pozostalych pacjentow na podstawie najwiekszych reszt
    while (pozostalo_pacjentow > 0) {
        // Znajdz lekarza z najwieksza reszta i przydziel mu pacjenta
        int max_index = 0;
        for (i = 1; i < 5; i++) {
            if (reszty[i] > reszty[max_index]) {
                max_index = i;
            }
        }
        // Przydziel jednego pacjenta temu lekarzowi
        limity_lekarzy[max_index]++;
        reszty[max_index] = 0; // Zresetuj reszte, aby zapobiec ponownej selekcji
        pozostalo_pacjentow--;
    }

    return;
}

int naSekundy(const char *time_str) {
    int hour, minute;
    if (sscanf(time_str, "%d:%d", &hour, &minute) != 2) {
        fprintf(stderr, "Blad: niepoprawny format czasu: %s\n", time_str);
        exit(1);
    }
    return hour * 3600 + minute * 60;   // bo w godzinie jest 3600 sek, a w minucie 60 sek
}

int zwrocObecnyCzas(){

    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
    return current_time;
}