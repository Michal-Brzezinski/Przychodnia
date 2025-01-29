#include "rejestracja.h"

int msg_id;

volatile int running = 1; // Flaga do sygnalizowania zakończenia

// Zmienna globalna do przechowywania pid procesu okienka nr 2
pid_t pid_okienka2 = -1;

int *pamiec_wspoldzielona;  // Wskaźnik do pamięci współdzielonej
int shmID;
key_t klucz_pamieci;

int limity_lekarzy[5] = {0}; // Tablica przechowująca limity pacjentów dla lekarzy

void uruchomOkienkoNr2(int msg_id, int semID);

void zatrzymajOkienkoNr2();

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        perror("\033[1;31m[Rejestracja]: Nieprawidłowa liczba argumentów\033[0m\n");
        exit(1);
    }

    printGreen("[Rejestracja]: Uruchomiono rejestrację");

    int limit_pacjentow = atoi(argv[1]);
    limity_lekarzy[0] = procentNaNaturalna(limit_pacjentow, 60); // Limit pacjentów dla lekarza POZ
    int procent10 = procentNaNaturalna(limit_pacjentow, 10);
    for (int i = 1; i < 5; i++)
    {
        limity_lekarzy[i] = procent10;
    }

    print_fflush("Odczytane limity lekarzy to:");
    fflush(stdout);
    printf("\033[1;38mWSZYSCY: \t%d\033[0m\n", limit_pacjentow);
    fflush(stdout);
    printf("\033[1;38mPOZ:\t%d\033[0m\n",limity_lekarzy[0]);
    fflush(stdout);
    printf("\033[1;38mKARDIOLOG:\t%d\033[0m\n",limity_lekarzy[1]);
    fflush(stdout);
    printf("\033[1;38mOKULISTA:\t%d\033[0m\n",limity_lekarzy[2]);
    fflush(stdout);
    printf("\033[1;38mPEDIATRA:\t%d\033[0m\n",limity_lekarzy[3]);
    fflush(stdout);
    printf("\033[1;38mMEDYCYNA PRACY:\t%d\033[0m\n", limity_lekarzy[4]);
    fflush(stdout);

    key_t msg_key = generuj_klucz_ftok(".", 'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key, IPC_CREAT | 0600);
    key_t klucz_wejscia = generuj_klucz_ftok(".", 'A');
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    // Aktualny czas
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    // Godziny otwarcia i zamknięcia rejestracji (w sekundach od północy)
    int Tp = current_time;      // Aktualny czas
    int Tk = current_time + 5; // Aktualny czas + x sekund

    printGreen("[Rejestracja]: Rejestracja uruchomiona, oczekuje na pacjentów");

    while (1)
    {

        // Sprawdź aktualny czas
        now = time(NULL);
        local = localtime(&now);
        current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

        //printf("\033[35mCzas: %02d:%02d:%02d\033[0m\n", local->tm_hour, local->tm_min, local->tm_sec);

        // Sprawdź, czy aktualny czas jest poza godzinami otwarcia
        if (current_time < Tp || current_time > Tk)
        {
            printYellow("[Rejestracja]: Przychodnia jest zamknięta. Kończenie pracy.");
            fflush(stdout);
            break; // Wyjście z pętli, gdy czas jest poza godzinami otwarcia
        }

        // Czekaj na komunikat rejestracji
        Wiadomosc msg;
        if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) == -1)
        {
            // Sprawdzamy, czy nie było wiadomości, ale nie blokujemy procesu dzięki IPC_NOWAIT
            if (errno != ENOMSG)
            {
                perror("\033[1;31m[Rejestracja]: Błąd msgrcv\033[0m\n");
                exit(1);
            }

            //printRed("[Rejestracja]: Brak pacjentów w kolejce\n");
            //sleep(4); // Czekaj 4 sekundy i sprawdź ponownie
            continue;
        }

        // Tutaj należy sprawdzić limit indywidulany lekarza (przed zarejestrowaniem pacjenta)
        printf("\033[1;32m[Rejestracja - 1 okienko]: Rejestracja pacjenta nr %d do lekarza: %d\033[0m\n", msg.id_pacjent, msg.id_lekarz);


        /*Obsługa wysyłania pacjenta do danego lekarza*/


        // Sprawdź liczbę procesów oczekujących na rejestrację ponownie
        int liczba_procesow = policzProcesy(msg_id);
        if (liczba_procesow > BUILDING_MAX / 2)
        {
            // Uruchomienie okienka nr 2
            uruchomOkienkoNr2(msg_id, semID);
        }
        else if (liczba_procesow < BUILDING_MAX / 3)
        {
            // Jeśli liczba procesów spadła poniżej N/3, zatrzymaj okienko nr 2
            zatrzymajOkienkoNr2();
        }

        // Informuj pacjenta, że może wyjść z budynku
        signalSemafor(semID, 1);

        // Proces rejestracji kontynuuje swoją pracę
        //sleep(4);
    }
    zatrzymajOkienkoNr2(); // Zatrzymaj okienko nr 2 przed zakończeniem pracy
    //
    printRed("Zablokowano wejście nowych pacjentów do budynku\n");
    inicjalizujSemafor(semID, 0, 0); // Zablokuj semafor wejścia do budynku
    waitSemafor(semID, 2, 0);        // Oznajmij zakończenie rejestracji
    wypiszPacjentowWKolejce(msg_id, semID);

    printYellow("Czekam na sygnał zakończenia generowania pacjentów...\n");

    klucz_pamieci = generuj_klucz_ftok(".", 'X');
    shmID = alokujPamiecWspoldzielona(klucz_pamieci, PAM_SIZE * sizeof(int), IPC_CREAT | 0666);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shmID, 0);
    waitSemafor(semID, 4, 0);
    int j;
    for(int j = 1; j < 6; j++){
        printf("Lekarz o id %d ma obecnie nastawiony licznik na: %d\n",j, pamiec_wspoldzielona[j]);
        fflush(stdout);
    }
    // Wypisz pacjentów, którzy byli w kolejce do rejestracji w momencie zamykania rejestracji
    //wypiszPacjentowWKolejce(msg_id, semID);
    printYellow("Rejestracja zakończyła działanie\n");
    
    return 0;
}

void uruchomOkienkoNr2(int msg_id, int semID)
{
    // Uruchomienie dodatkowego procesu rejestracji (okienka nr 2)
    pid_okienka2 = fork();
    if (pid_okienka2 == 0)
    {
        // Dziecko: Okienko nr 2
        printGreen("[Rejestracja - 2 okienko]: Otworzenie okienka nr 2...");
        while (1)
        {
            // Czekaj na komunikaty
            Wiadomosc msg;
            if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1)
            {
                perror("\033[1;31m[Rejestracja]: Błąd msgrcv\033[0m");

                exit(1);
            }
            printf("\033[1;32m[Rejestracja - 2 okienko]: Rejestracja pacjenta nr %d do lekarza: %d\033[0m\n", msg.id_pacjent, msg.id_lekarz);
            //sleep(3); // symulacja procesu rejestracji
        }
    }
    else if (pid_okienka2 == -1)
    {
        perror("\033[1;31m[Rejestracja]: Błąd fork (okienko nr 2)\033[0m\n");
        exit(1);
    }
}
void zatrzymajOkienkoNr2()
{
    // Jeśli proces okienka nr 2 istnieje, wyślij do niego sygnał zakończenia
    if (pid_okienka2 != -1)
    {
        printYellow("[Rejestracja]: Zatrzymywanie okienka nr 2...\n");
        kill(pid_okienka2, SIGTERM); // Wysłać sygnał SIGTERM do procesu okienka nr 2 (zakończenie)
    }
}