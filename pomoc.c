#include "MyLib/shm_utils.h"
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 6

int *pam;
int shmID;
key_t klucz;


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

    printf("Podane wartości to: 1: %d, 2: %d, 3: %d, 4: %d\n", pam[0], pam[1], pam[3], pam[4]);

    return 0;
}