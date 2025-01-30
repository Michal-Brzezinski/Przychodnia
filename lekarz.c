#include "lekarz.h"

int* pamiec_wspoldzielona; // Wskaznik do pamieci wspoldzielonej
// pamiec_wspoldzielona[0] - wspolny licznik pacjentow

int shmID, semID, msg_id_lekarz, msg_id_rejestracja;
key_t klucz_pamieci, klucz_sem, klucz_kolejki_lekarza, klucz_kolejki_rejestracji;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror_red("[Lekarz]: Nieprawidlowa liczba argumentow\n");
        exit(1);
    }
    int id_lekarz = atoi(argv[1]);
    int limit_pacjentow = atoi(argv[2]);

    //____________________   INICJALIZACJA LEKARZA   ______________________
    Lekarz lekarz;

    int limit_indywidualny = 0;
    switch(id_lekarz){
        case POZ:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 60);
            break;
        case KARDIOLOG:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        case OKULISTA:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        case PEDIATRA:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        case MEDYCYNA_PRACY:
            limit_indywidualny = procentNaNaturalna(limit_pacjentow, 10);
            break;
        default:
            break;
    }
    inicjalizuj_lekarza(&lekarz, id_lekarz, limit_indywidualny);

    printMagenta("[Lekarz]: Wygenerowano lekarza: %s o id: %d, limit pacjentow: %d\n", lekarz.nazwa, lekarz.id_lekarz, lekarz.indywidualny_limit);

    //  ____________________   INICJALIZACJA  ZASOBOW IPC   _____________________

    klucz_pamieci = generuj_klucz_ftok(".", 'X');
    shmID = alokujPamiecWspoldzielona(klucz_pamieci, PAM_SIZE * sizeof(int), IPC_CREAT | 0600);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shmID, 0); // dolaczenie pamieci wspoldzielonej

    klucz_sem = generuj_klucz_ftok(".", 'A');
    semID = alokujSemafor(klucz_sem, S, IPC_CREAT | 0600);

    if(lekarz.id_lekarz == 1){
        // tylko lekarze POZ potrzebuja dostepu do rejestracji w razie potrzeby skierowania do specjalisty
        klucz_kolejki_rejestracji = generuj_klucz_ftok(".", 'B');
        msg_id_rejestracja = alokujKolejkeKomunikatow(klucz_kolejki_rejestracji, IPC_CREAT | 0600);
    }

    klucz_kolejki_lekarza = generuj_klucz_ftok(".",lekarz.id_lekarz);
    // klucz generowany jest na bazie znaku reprezentujacego lekarza
    msg_id_lekarz = alokujKolejkeKomunikatow(klucz_kolejki_lekarza, IPC_CREAT | 0600);


    //  ____________________   INICJALIZACJA LICZNIKA LEKARZA W PAMIECI WSPOLDZIELONEJ   _____________________
    waitSemafor(semID, 3, 0);
    printMagenta("[Lekarz]: %d inicjalizuje swoj licznik w pamieci dzielonej\n", lekarz.id_lekarz);
    pamiec_wspoldzielona[lekarz.id_lekarz] = lekarz.licznik_pacjentow; // ustawia na 0, tak zainicjalizowano lekarza
    ++pamiec_wspoldzielona[6];
    if(pamiec_wspoldzielona[6]==5){
        signalSemafor(semID, 4); // pozwol na odczytanie licznikow lekarzy
    }
    signalSemafor(semID, 3);
    // ________________________________________________________________________________________________________

    czynnosci_lekarskie(&lekarz);   // symulacja pracy lekarza

    return 0;
}


void czynnosci_lekarskie(Lekarz *lekarz){
    // Funkcja symulujaca prace lekarza


    // Aktualny czas
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    // Godziny otwarcia i zamkniecia rejestracji (w sekundach od polnocy)
    int Tp = current_time;      // Aktualny czas
    int Tk = current_time + 5; // Aktualny czas + x sekund 

    
    // Glowna petla dzialania lekarza
    // while(1){
        
    //     // Sprawdz aktualny czas
    //     now = time(NULL);
    //     local = localtime(&now);
    //     current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
    //     // Sprawdz, czy aktualny czas jest poza godzinami otwarcia
    //     if (current_time < Tp || current_time > Tk)
    //     {
    //         printYellow("[Lekarz]: Przychodnia jest zamknieta. Konczenie pracy.");
    //         fflush(stdout);
    //         break; // Wyjscie z petli, gdy czas jest poza godzinami otwarcia
    //     }
        
    //     Wiadomosc msg;
    //     if (msgrcv(msg_id_lekarz, &msg, sizeof(Wiadomosc) - sizeof(long), -2, 0) == -1)        {
    //         if (errno != ENOMSG)
    //         {
    //             perror_red("[Lekarz]: Blad msgrcv");
    //             exit(1);
    //         }
    //     }
    //     printMagenta("[Lekarz]: Lekarz %s przyjal pacjenta %d", lekarz->nazwa, msg.id_pacjent);
    //     sleep(3); // symulacja pracy lekarza
    //     lekarz->licznik_pacjentow++;
    //     if(lekarz->licznik_pacjentow >= lekarz->indywidualny_limit){
    //         printMagenta("[Lekarz]: Lekarz %s osiagnal limit pacjentow", lekarz->nazwa);
    //         break;
    //     }
        
    // }


}