#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <time.h>

#include "MyLib/sem_utils.h"
#include "MyLib/shm_utils.h"
#include "MyLib/msg_utils.h"
#include "MyLib/utils.h"

// ____________ TYP WYLICZENIOWY NUMER-TYP LEKARZY
enum lekarze{ 
    POZ = 1, 
    KARDIOLOG, 
    OKULISTA, 
    PEDIATRA, 
    MEDYCYNA_PRACY
};

volatile int running = 1; // Flaga do sygnalizowania zakonczenia
pid_t pid_okienka2 = -1; // Zmienna globalna do przechowywania pid procesu okienka nr 2

//________________ KOLEJKA DO REJESTRACJI
int msg_id_rej; 
key_t msg_key_rej;


//_______________   ZBIOR SEMAFOROW
int sem_id;
key_t sem_key;

//_______________________ KOLEJKA NA KOMUNIKATY O MOZLIWOSCI WYJSCIA Z BUDYNKU
key_t klucz_wyjscia;
int msg_id_wyjscie;

//_______________________ KOLEJKA DLA PACJENTOW PRZEKIEROWANYCH OD POZ
key_t msg_key_POZ_rej;    
int msg_id_POZ_rej;       

//_____________________ PAMIEC WSPOLDZIELONA DO LICZENIA PRZYJEC LEKARZY W REJESTRACJI 
//                                  (POMIEDZY PROCESAMI OKIENKA 1 I 2)
int shm_id_przyjecia;
int *przyjecia;        
key_t shm_key_przyjecia;  

//_______________________ PAMIEC WSPOLDZIELONA DO OCENY DOSTEPNOSCI LEKARZA 
//                              (CZY NIE DOSTAL SYGNALU OD DYREKTORA)
int shm_id_dostepnosc; 
sig_atomic_t *dostepnosc_lekarza;   
key_t shm_key_dostepnosc;


int msg_id_1; // id 1. kolejki POZ
int msg_id_2; // id 2. kolejki KARDIOLOGA
int msg_id_3; // id 3. kolejki OKULISTY
int msg_id_4; // id 4. kolejki PEDIATRY
int msg_id_5; // id 5. kolejki LEKARZA MEDYCYNY PRACY

key_t msg_key_1; // klucz do kolejki do POZ
key_t msg_key_2; // klucz do kolejki do KARDIOLOGA
key_t msg_key_3; // klucz do kolejki do OKULISTY
key_t msg_key_4; // klucz do kolejki do PEDIATRY
key_t msg_key_5; // klucz do kolejki do LEKARZA MEDYCYNY PRACY

int limity_lekarzy[5] = {0}; // Tablica przechowujaca limity pacjentow dla lekarzy
int limit_pacjentow;

// zmienne do operacji na czasie
int Tp, Tk; // czas poczatkowy i czas koncowy (czas dzialania rejestracji)

int building_max;

// flaga, ktora pozwoli na odeslanie pacjentow, ktorzy w momencie osiagniecia limitu czekali w kolejce rejestracji
int odeslano_pacjentow_po_osiagnieciu_limitu1 = 0;  //okienko 1
int odeslano_pacjentow_po_osiagnieciu_limitu2 = 0;  //okienko 2

sig_atomic_t zakoncz2okienko = 0;

// funkcje potrzebne do obslugi rejestracji

void uruchomOkienkoNr2();
void handlerSIGUSR2(int signum);
void zatrzymajOkienkoNr2();

//  Wersja funkcji dla rejestracji - rozni sie nieznacznie od tej w lekarz.h
Wiadomosc* wypiszPacjentowWKolejce(int msg_id, int *rozmiar_kolejki) {
    
    int size = 0;
    int capacity = 500;
    Wiadomosc *pacjenci_po_zamknieciu = malloc(capacity * sizeof(Wiadomosc));
    if (pacjenci_po_zamknieciu == NULL) {
        perror_red("[wypiszPacjentowWKolejceRejestracji]: malloc error\n");
        exit(1);
    }

    Wiadomosc msg;
    while (1) {
        errno = 0;
        int ret = msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT);
        if (ret != -1) {
            // Otrzymano wiadomosc
            signalSemafor(sem_id, 7);   // zwieksz licznik miejsca w kolejce do rejestracji

            print("Pacjent nr %d, wiek: %d, vip: %d\n", msg.id_pacjent, msg.wiek, msg.vip);

            // Sprawdzenie, czy potrzebna jest realokacja tablicy
            if (size >= capacity) {
                capacity *= 2;
                Wiadomosc *temp = realloc(pacjenci_po_zamknieciu, capacity * sizeof(Wiadomosc));
                if (temp == NULL) {
                    perror_red("[wypiszPacjentowWKolejceRejestracji]: realloc error\n");
                    free(pacjenci_po_zamknieciu);
                    exit(1);
                }
                pacjenci_po_zamknieciu = temp;
            }

            pacjenci_po_zamknieciu[size++] = msg;
        } else {
            if (errno == ENOMSG) {
                // Brak wiadomosci konczy petle
                break;
            } else {
                perror_red("[wypiszPacjentowWKolejceRejestracji]: Błąd msgrcv\n");
                free(pacjenci_po_zamknieciu);
                exit(1);
            }
        }
    }

    *rozmiar_kolejki = size;

    // Dopasowanie rozmiaru tablicy do rzeczywistej liczby pacjentow
    if (size == 0) {
        free(pacjenci_po_zamknieciu);
        pacjenci_po_zamknieciu = NULL;
    } else {
        Wiadomosc *temp = realloc(pacjenci_po_zamknieciu, size * sizeof(Wiadomosc));
        if (temp == NULL) {
            perror_red("[wypiszPacjentowWKolejceRejestracji]: realloc error\n");
            free(pacjenci_po_zamknieciu);
            exit(1);
        }
        pacjenci_po_zamknieciu = temp;
    }

    return pacjenci_po_zamknieciu;
}


void odeslijPacjentowPrzekroczenieLimitu(int nr_okienka){
    int rozmiar_pozostalych = 0;
    printGreen("[Rejestracja - %d okienko]: W momencie przekroczenia limitu przyjec w kolejce do rejestracji stali:\n", nr_okienka);
    Wiadomosc *pozostali = wypiszPacjentowWKolejce(msg_id_rej, &rozmiar_pozostalych);
    
    if (pozostali != NULL) {
        waitSemafor(sem_id, 4, 0);  // blokada dostępu do pliku kontrolnego
        FILE *raport = fopen("raport", "a");
        if (raport != NULL) {
            for (int i = 0; i < rozmiar_pozostalych; i++) {
                // Wyślij pacjenta do domu
                pozostali[i].mtype = pozostali[i].id_pacjent;

                waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
                if(kill(pozostali[i].id_pacjent, 0) == 0){
                    if (msgsnd(msg_id_wyjscie, &pozostali[i], sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                        printYellow("[Rejestracja]: Blad msgsnd dla PID %d\n", pozostali[i].id_pacjent);
                        signalSemafor(sem_id, 8); // zwieksz licznik miejsc w kolejce do wyjscia

                    }
                }

                                    //_______________   WYSLANIE WIADOMOSCI DO WYJSCIA PACJENTOWI _____________________
                                        fprintf(raport, "[odeslijPacjentowPoPrzekroczeniuLimitu]: 1Wyslano wiadomosc wyjscia do pacjenta nr %d\n", pozostali[i].id_pacjent);
                                    // _____________________________________________________________________________________
            
                // Zapisz informację w raporcie
                fprintf(raport, "Pacjent %d odeslany po osiagnieciu limitu %d\n", pozostali[i].id_pacjent, pozostali[i].id_lekarz);
                fflush(raport);
            }
            fclose(raport);
            signalSemafor(sem_id, 4);

            printGreen("[Rejestracja - %d okienko]: Odeslano pacjentow oczekujacych na przyjecie po osiagnieciu limitu przyjec\n", nr_okienka);
            free(pozostali);
        } else {
            perror_red("[Rejestracja]: Blad otwarcia pliku raport\n");
            printRed("Nie odeslano pacjentow\n");
            signalSemafor(sem_id, 4);
            free(pozostali);
        }
    }
}
