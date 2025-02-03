#include "MyLib/shm_utils.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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

int main()
{
    klucz = ftok(".", 'X');
    shmID = alokujPamiecWspoldzielona(klucz, SIZE * sizeof(int), IPC_CREAT | 0666);

    pam = dolaczPamiecWspoldzielona(shmID, 0);
    switch(fork())
    {
        case -1:
            perror("fork");
            exit(1);
        case 0:
            pam[3] = 2654;
            pam[4] = 253;
            exit(0);
    }
    pam[0] = 2137;
    pam[1] = 1488;

    wait(NULL);

    zwolnijPamiecWspoldzielona(klucz);

    // Wywołanie funkcji printRed z formatowaniem
    printRed("Podane wartosci to: 1: %d, 2: %d, 3: %d, 4: %d\n", pam[0], pam[1], pam[3], pam[4]);
    int i = 1;
    printf("i ma wartość : %c",(char)(i));

    return 0;
}