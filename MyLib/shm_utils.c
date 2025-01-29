#include "shm_utils.h"

int alokujPamiecWspoldzielona(key_t klucz, int rozmiar, int flagi) {
    /*Funkcja alokuje pamięć współdzieloną o podanym kluczu i rozmiarze*/
    int shmID = shmget(klucz, rozmiar, flagi);
    if (shmID == -1) {
        perror("\033[1;31m[alokujPamiecWspoldzielona]: Błąd shmget\033[0m\n");
        exit(1);
    }

    return shmID;
}

int *dolaczPamiecWspoldzielona(int shmID, int flagi) {
/* zwraca wskaźnik na pierwszy element przyłączonej pamięci*/
    int *pamiec = (int *)shmat(shmID, NULL, flagi);
    if (pamiec == (int *)-1) {
        perror("\033[1;31m[alokujPamiecWspoldzielona]: Błąd shmat\033[0m\n");
        exit(1);
    }
    return pamiec;
}

int odlaczPamiecWspoldzielona(int *pamiec) {
    /*Funkcja odłącza pamięć współdzieloną*/
    if (shmdt(pamiec) == -1) {
        perror("\033[1;31m[odlaczPamiecWspoldzielona]: Błąd shmdt\033[0m\n");
        exit(1);
    }
    return 0;
}


void zwolnijPamiecWspoldzielona(int klucz) {
    // Sprawdzenie, czy segment pamięci dzielonej istnieje
    int shmid = shmget(klucz, 0, 0); // 0 jako rozmiar (nie potrzebujemy go do sprawdzenia)
    
    if (shmid == -1) {
        // Jeśli shmget zwróci -1, to oznacza, że segment pamięci dzielonej nie istnieje
        if (errno == ENOENT) {
            // Segment pamięci dzielonej o podanym kluczu nie istnieje
        } 
        else {
            perror("shmget() failed");
            exit(1);
        }
        return;
    }

    // Przyłączenie segmentu pamięci dzielonej (choć tu nie trzeba używać zwróconego wskaźnika)
    void* shm_ptr = shmat(shmid, NULL, 0);
    
    if (shm_ptr == (void*)-1) {
        // Jeśli segment pamięci nie jest dołączony, nie możemy go odłączyć, ale i tak będziemy go usuwać
        if (errno != EINVAL) {  // EINVAL oznacza, że segment nie jest dołączony
            perror("shmat() failed");
            exit(1);
        }
        // Segment pamięci dzielonej nie był dołączony - nie ma potrzeby odłączania

    } else {
        odlaczPamiecWspoldzielona((int*)shm_ptr);
    }

    // Usunięcie segmentu pamięci dzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl() failed");
        exit(1);
    }

}