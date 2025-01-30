#include "shm_utils.h"

int alokujPamiecWspoldzielona(key_t klucz, int rozmiar, int flagi) {
    /*Funkcja alokuje pamiec wspoldzielona o podanym kluczu i rozmiarze*/
    int shmID = shmget(klucz, rozmiar, flagi);
    if (shmID == -1) {
        perror("\033[1;31m[alokujPamiecWspoldzielona]: Blad shmget\033[0m\n");
        exit(1);
    }

    return shmID;
}

int *dolaczPamiecWspoldzielona(int shmID, int flagi) {
/* zwraca wskaznik na pierwszy element przylaczonej pamieci*/
    int *pamiec = (int *)shmat(shmID, NULL, flagi);
    if (pamiec == (int *)-1) {
        perror("\033[1;31m[alokujPamiecWspoldzielona]: Blad shmat\033[0m\n");
        exit(1);
    }
    return pamiec;
}

int odlaczPamiecWspoldzielona(int *pamiec) {
    /*Funkcja odlacza pamiec wspoldzielona*/
    if (shmdt(pamiec) == -1) {
        perror("\033[1;31m[odlaczPamiecWspoldzielona]: Blad shmdt\033[0m\n");
        exit(1);
    }
    return 0;
}


void zwolnijPamiecWspoldzielona(int klucz) {
    // Sprawdzenie, czy segment pamieci dzielonej istnieje
    int shmid = shmget(klucz, 0, 0); // 0 jako rozmiar (nie potrzebujemy go do sprawdzenia)
    
    if (shmid == -1) {
        // Jesli shmget zwroci -1, to oznacza, ze segment pamieci dzielonej nie istnieje
        if (errno == ENOENT) {
            // Segment pamieci dzielonej o podanym kluczu nie istnieje
        } 
        else {
            perror("shmget() failed");
            exit(1);
        }
        return;
    }

    // Przylaczenie segmentu pamieci dzielonej (choc tu nie trzeba uzywac zwroconego wskaznika)
    void* shm_ptr = shmat(shmid, NULL, 0);
    
    if (shm_ptr == (void*)-1) {
        // Jesli segment pamieci nie jest dolaczony, nie mozemy go odlaczyc, ale i tak bedziemy go usuwac
        if (errno != EINVAL) {  // EINVAL oznacza, ze segment nie jest dolaczony
            perror("shmat() failed");
            exit(1);
        }
        // Segment pamieci dzielonej nie byl dolaczony - nie ma potrzeby odlaczania

    } else {
        odlaczPamiecWspoldzielona((int*)shm_ptr);
    }

    // Usuniecie segmentu pamieci dzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl() failed");
        exit(1);
    }

}