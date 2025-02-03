#include "MyLib/shm_utils.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#pragma warning(disable:4996)

#define SIZE 6

int *pam;
int shmID;
key_t klucz;

// Funkcja drukująca w kolorze (teraz tylko z jednym argumentem format)
void printRed(const char *format, ...) {
    // Ustawiamy kolor tekstu (przykład dla koloru czerwonego)
    printf("\033[1;31m");

    // Zmienna lista argumentów do printf
    va_list args;
    va_start(args, format);
    
    // Wywołujemy printf z odpowiednimi argumentami
    vprintf(format, args);
    
    // Kończymy, resetując kolor
    va_end(args);
    printf("\033[0m");
}



// int procentNaNaturalna(int n, int x) {
//     double procent = (double)x / 100.0;
//     double s = floor(n * procent);
//     return (int)s;
// }

int main()
{
    // klucz = ftok(".", 'X');
    // shmID = alokujPamiecWspoldzielona(klucz, SIZE * sizeof(int), IPC_CREAT | 0666);

    // pam = dolaczPamiecWspoldzielona(shmID, 0);
    // switch(fork())
    // {
    //     case -1:
    //         perror("fork");
    //         exit(1);
    //     case 0:
    //         pam[3] = 2654;
    //         pam[4] = 253;
    //         exit(0);
    // }
    // pam[0] = 2137;
    // pam[1] = 1488;

    // wait(NULL);

    // zwolnijPamiecWspoldzielona(klucz);

    // // Wywołanie funkcji printRed z formatowaniem
    // printRed("Podane wartosci to: 1: %d, 2: %d, 3: %d, 4: %d\n", pam[0], pam[1], pam[3], pam[4]);
    // int i = 1;
    // printf("i ma wartość : %c",(char)(i));


    int limit_pacjentow;
    printf("Podaj liczbę: ");
    scanf("%d",&limit_pacjentow);

    // Deklaracja tablic i zmiennych
    double procenty[5] = {60.0, 10.0, 10.0, 10.0, 10.0};
    double limity_lekarzy_double[5];
    int limity_lekarzy[5] = {0};
    double reszty[5];
    int suma_limity = 0;
    int i;

    // Oblicz dokładne limity jako liczby zmiennoprzecinkowe
    for (i = 0; i < 5; i++) {
        limity_lekarzy_double[i] = (limit_pacjentow * procenty[i]) / 100.0;
        limity_lekarzy[i] = (int)limity_lekarzy_double[i]; // Część całkowita
        reszty[i] = limity_lekarzy_double[i] - limity_lekarzy[i]; // Reszta
        suma_limity += limity_lekarzy[i];
    }

    // Oblicz pozostałych pacjentów do przydzielenia
    int pozostalo_pacjentow = limit_pacjentow - suma_limity;

    // Przydziel pozostałych pacjentów na podstawie największych reszt
    while (pozostalo_pacjentow > 0) {
        // Znajdź lekarza z największą resztą
        int max_index = 0;
        for (i = 1; i < 5; i++) {
            if (reszty[i] > reszty[max_index]) {
                max_index = i;
            }
        }
        // Przydziel jednego pacjenta temu lekarzowi
        limity_lekarzy[max_index]++;
        reszty[max_index] = 0; // Zresetuj resztę, aby zapobiec ponownej selekcji
        pozostalo_pacjentow--;
    }
    printf("\n");
    // Wyświetl wynik
    printf("Limity dla każdego lekarza:\n");
    for (i = 0; i < 5; i++) {
        printf("Lekarz %d: %d pacjentów\n", i, limity_lekarzy[i]);
    }

    return 0;
}