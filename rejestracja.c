#include "rejestracja.h"

int msg_id;

volatile int running = 1; // Flaga do sygnalizowania zakonczenia

// Zmienna globalna do przechowywania pid procesu okienka nr 2
pid_t pid_okienka2 = -1;

int *pamiec_wspoldzielona;  // Wskaznik do pamieci wspoldzielonej
int shmID;
key_t klucz_pamieci;

int limity_lekarzy[5] = {0}; // Tablica przechowujaca limity pacjentow dla lekarzy

void uruchomOkienkoNr2(int msg_id, int semID);

void zatrzymajOkienkoNr2();

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        perror_red("[Rejestracja]: Nieprawidlowa liczba argumentow");
        exit(1);
    }

    printGreen("[Rejestracja]: Uruchomiono rejestracje\n");

    int limit_pacjentow = atoi(argv[1]);
    limity_lekarzy[0] = procentNaNaturalna(limit_pacjentow, 60); // Limit pacjentow dla lekarza POZ
    int procent10 = procentNaNaturalna(limit_pacjentow, 10);
    for (int i = 1; i < 5; i++)
    {
        limity_lekarzy[i] = procent10;
    }

    print("Odczytane limity lekarzy to:\n");
    print("WSZYSCY: \t%d\n", limit_pacjentow);
    print("POZ:\t%d\n",limity_lekarzy[0]);
    print("KARDIOLOG:\t%d\n",limity_lekarzy[1]);
    print("OKULISTA:\t%d\n",limity_lekarzy[2]);
    print("PEDIATRA:\t%d\n",limity_lekarzy[3]);
    print("MEDYCYNA PRACY:\t%d\n", limity_lekarzy[4]);

    key_t msg_key = generuj_klucz_ftok(".", 'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key, IPC_CREAT | 0600);
    key_t klucz_wejscia = generuj_klucz_ftok(".", 'A');
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    // Aktualny czas
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    // Godziny otwarcia i zamkniecia rejestracji (w sekundach od polnocy)
    int Tp = current_time;      // Aktualny czas
    int Tk = current_time + 5; // Aktualny czas + x sekund

    printGreen("[Rejestracja]: Rejestracja uruchomiona, oczekuje na pacjentow\n");

    while (1)
    {

        // Sprawdz aktualny czas
        now = time(NULL);
        local = localtime(&now);
        current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

        //printf("\033[35mCzas: %02d:%02d:%02d\033[0m\n", local->tm_hour, local->tm_min, local->tm_sec);

        // Sprawdz, czy aktualny czas jest poza godzinami otwarcia
        if (current_time < Tp || current_time > Tk)
        {
            printYellow("[Rejestracja]: Przychodnia jest zamknieta. Konczenie pracy.\n");
            break; // Wyjscie z petli, gdy czas jest poza godzinami otwarcia
        }

        // Czekaj na komunikat rejestracji
        Wiadomosc msg;
        if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) == -1)
        {
            // Sprawdzamy, czy nie bylo wiadomosci, ale nie blokujemy procesu dzieki IPC_NOWAIT
            if (errno != ENOMSG)
            {
                perror_red("[Rejestracja]: Blad msgrcv\n");
                exit(1);
            }

            //printRed("[Rejestracja]: Brak pacjentow w kolejce\n");
            sleep(4); // Czekaj 4 sekundy i sprawdz ponownie
            continue;
        }

        // Tutaj nalezy sprawdzic limit indywidulany lekarza (przed zarejestrowaniem pacjenta)
        printGreen("[Rejestracja - 1 okienko]: Rejestracja pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);


        /*Obsluga wysylania pacjenta do danego lekarza*/


        // Sprawdz liczbe procesow oczekujacych na rejestracje ponownie
        int liczba_procesow = policzProcesy(msg_id);
        if (liczba_procesow > BUILDING_MAX / 2)
        {
            // Uruchomienie okienka nr 2
            uruchomOkienkoNr2(msg_id, semID);
        }
        else if (liczba_procesow < BUILDING_MAX / 3)
        {
            // Jesli liczba procesow spadla ponizej N/3, zatrzymaj okienko nr 2
            zatrzymajOkienkoNr2();
        }

        // Informuj pacjenta, ze moze wyjsc z budynku
        signalSemafor(semID, 1);

        // Proces rejestracji kontynuuje swoja prace
        sleep(4);
    }
    zatrzymajOkienkoNr2(); // Zatrzymaj okienko nr 2 przed zakonczeniem pracy
    //
    printRed("Zablokowano wejscie nowych pacjentow do budynku\n");
    inicjalizujSemafor(semID, 0, 0); // Zablokuj semafor wejscia do budynku
    waitSemafor(semID, 2, 0);        // Oznajmij zakonczenie rejestracji
    wypiszPacjentowWKolejce(msg_id, semID);

    printYellow("Czekam na sygnal zakonczenia generowania pacjentow...\n");

    klucz_pamieci = generuj_klucz_ftok(".", 'X');
    shmID = alokujPamiecWspoldzielona(klucz_pamieci, PAM_SIZE * sizeof(int), IPC_CREAT | 0666);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shmID, 0);
    waitSemafor(semID, 4, 0);
    int j;
    for(int j = 1; j < 6; j++){
        print("Lekarz o id %d ma obecnie nastawiony licznik na: %d\n",j, pamiec_wspoldzielona[j]);
    }
    // Wypisz pacjentow, ktorzy byli w kolejce do rejestracji w momencie zamykania rejestracji
    //wypiszPacjentowWKolejce(msg_id, semID);
    printYellow("Rejestracja zakonczyla dzialanie\n");
    
    return 0;
}

void uruchomOkienkoNr2(int msg_id, int semID)
{
    // Uruchomienie dodatkowego procesu rejestracji (okienka nr 2)
    pid_okienka2 = fork();
    if (pid_okienka2 == 0)
    {
        // Dziecko: Okienko nr 2
        printGreen("[Rejestracja - 2 okienko]: Otworzenie okienka nr 2...\n");
        while (1)
        {
            // Czekaj na komunikaty
            Wiadomosc msg;
            if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0, 0) == -1)
            {
                perror_red("[Rejestracja]: Blad msgrcv (okienko nr 2)\n");

                exit(1);
            }
            printGreen("[Rejestracja - 2 okienko]: Rejestracja pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
            sleep(3); // symulacja procesu rejestracji
        }
    }
    else if (pid_okienka2 == -1)
    {
        perror_red("[Rejestracja]: Blad fork (okienko nr 2)\n");
        exit(1);
    }
}
void zatrzymajOkienkoNr2()
{
    // Jesli proces okienka nr 2 istnieje, wyslij do niego sygnal zakonczenia
    if (pid_okienka2 != -1)
    {
        printYellow("[Rejestracja]: Zatrzymywanie okienka nr 2...\n");
        kill(pid_okienka2, SIGTERM); // Wyslac sygnal SIGTERM do procesu okienka nr 2 (zakonczenie)
    }
}