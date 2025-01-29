#include "lekarz.h"

int* pamiec_wspoldzielona; // Wskaźnik do pamięci współdzielonej
// pamiec_wspoldzielona[0] - wspolny licznik pacjentow

int shmID, semID, msg_id;
key_t klucz_pamieci, klucz_sem, klucz_kolejki;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("\033[1;31m[Lekarz]: Nieprawidłowa liczba argumentów\033[0m\n");
        exit(1);
    }
    Lekarz lekarz;
    int id_lekarz = atoi(argv[1]);
    int limit_pacjentow = atoi(argv[2]);

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

    printf("\033[1;35m[Lekarz]: Wygenerowano lekarza: %s o id: %d, limit pacjentów: %d\033[0m\n", lekarz.nazwa, lekarz.id_lekarz, lekarz.indywidualny_limit);

    klucz_pamieci = generuj_klucz_ftok(".", 'X');
    shmID = alokujPamiecWspoldzielona(klucz_pamieci, PAM_SIZE * sizeof(int), IPC_CREAT | 0666);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shmID, 0);

    klucz_sem = generuj_klucz_ftok(".", 'A');
    semID = alokujSemafor(klucz_sem, S, IPC_CREAT | 0600);

    czynnosci_lekarskie(&lekarz);

    return 0;
}


void czynnosci_lekarskie(Lekarz *lekarz){

    printf("Drukuje sobie lekarza: %d\n", lekarz->id_lekarz);
    lekarz->licznik_pacjentow = losuj_int(10);
    waitSemafor(semID, 3, 0);
    pamiec_wspoldzielona[lekarz->id_lekarz] = lekarz->licznik_pacjentow;
    ++pamiec_wspoldzielona[6];
    if(pamiec_wspoldzielona[6]==5){
        signalSemafor(semID, 4);
    }
    signalSemafor(semID, 3);
    

    // key_t klucz_kolejki = generuj_klucz_ftok(".",lekarz->id_lekarz);   
    // // klucz generowany jest na bazie ścieżki do pliku i znaku reprezentującego lekarza
    // msg_id = alokujKolejkeKomunikatow(klucz_kolejki, IPC_CREAT | 0600);

    // key_t kluczm=ftok(".",'B');
    // shmID = shmget(kluczm, PAM_SIZE*sizeof(int), IPC_CREAT|0666);
    // if (shmID==-1) {printf("\033[1;31m[Lekarz]: blad shmget\033[0m\n"); exit(1);}


    // // Aktualny czas
    // time_t now = time(NULL);
    // struct tm *local = localtime(&now);
    // int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

    // // Godziny otwarcia i zamknięcia rejestracji (w sekundach od północy)
    // int Tp = current_time;      // Aktualny czas
    // int Tk = current_time + 5; // Aktualny czas + x sekund 

    // while(1){
        
    //     // Sprawdź aktualny czas
    //     now = time(NULL);
    //     local = localtime(&now);
    //     current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
    //     // Sprawdź, czy aktualny czas jest poza godzinami otwarcia
    //     if (current_time < Tp || current_time > Tk)
    //     {
    //         printYellow("[Lekarz]: Przychodnia jest zamknięta. Kończenie pracy.");
    //         fflush(stdout);
    //         break; // Wyjście z pętli, gdy czas jest poza godzinami otwarcia
    //     }
        /*
        Wiadomosc msg;
        if (msgrcv(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), -2, 0) == -1)        {
            if (errno != ENOMSG)
            {
                perror("\033[1;31m[Lekarz]: Błąd msgrcv\033[0m\n");
                exit(1);
            }
        }
        printf("\033[1;35m[Lekarz]: Lekarz %s przyjął pacjenta %d\033[0m\n", lekarz->nazwa, msg.id_pacjent);
        fflush(stdout);
        sleep(3); // symulacja pracy lekarza
        lekarz->licznik_pacjentow++;
        if(lekarz->licznik_pacjentow >= lekarz->indywidualny_limit){
            printf("\033[1;35m[Lekarz]: Lekarz %s osiągnął limit pacjentów\033[0m\n", lekarz->nazwa);
            break;
        }
        */
    //}


}