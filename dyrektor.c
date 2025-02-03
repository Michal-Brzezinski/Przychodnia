#include "dyrektor.h"
#include "lekarz.h"
#define MAX_BUILDING 23

void zakonczPraceLekarza(int pid_procesu){
    time_t now,  Tp, Tk, current_time;
    struct tm *local;
    int msg_id_rej, msg_id_1, msg_id_2, msg_id_3, msg_id_4, msg_id_5, sem_id, *pamiec_wspoldzielona, msg_id_pom, kolejka_size, msg_id_wyjscie, *limity_lekarzy;
    pid_t pid_okienka2;
    Wiadomosc msg;
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
        
        if (msgrcv(msg_id_rej, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) == -1)
        {
            // Sprawdzamy, czy nie bylo wiadomosci, ale nie blokujemy procesu dzieki IPC_NOWAIT
            if (errno != ENOMSG)
            {
                perror_red("[Rejestracja - 1 okienko]: Blad msgrcv\n");
                exit(1);
            }
            
            //printRed("[Rejestracja]: Brak pacjentow w kolejce\n");
            sleep(2); // Czekaj 2 sekundy i sprawdz ponownie
            continue;
        }
        
        printGreen("[Rejestracja - 1 okienko]: Otrzymano wiadomosc od pacjenta nr %d, ktory chce isc do lekarza nr %d\n", msg.id_pacjent, msg.id_lekarz);
        // Pobieranie informacji o kolejce komunikatow
        waitSemafor(sem_id, 3, 0); // dostep do pamieci dzielonej jednoczesnie uniemozliwiajac zapis do niej
        licznik_pom = pamiec_wspoldzielona[msg.id_lekarz];
        switch(msg.id_lekarz){
            // przypisanie zmiennej pomocniczej odpowiedniej wartosci id kolejki lekarza
            case POZ: msg_id_pom = msg_id_1; break;
            case KARDIOLOG: msg_id_pom = msg_id_2; break;
            case OKULISTA: msg_id_pom = msg_id_3; break;
            case PEDIATRA: msg_id_pom = msg_id_4; break;
            case MEDYCYNA_PRACY: msg_id_pom = msg_id_5; break;
        }
        if((kolejka_size = policzProcesy(msg_id_pom))==-1){
            Wiadomosc msg1 = msg;
            msg1.mtype = msg.id_pacjent;
            // Wyslij pacjenta do domu
            if (msgsnd(msg_id_wyjscie, &msg1, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do domu\n");
                exit(1);
            }
            // signalSemafor(sem_id, 1);   // Informuj pacjenta, ze moze wyjsc z budynku
            signalSemafor(sem_id, 3);
            continue; 
        }
        
        if(limity_lekarzy[msg.id_lekarz-1]<=(licznik_pom+kolejka_size)){
            
            printYellow("[Rejestracja - 1 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
            Wiadomosc msg1 = msg;
            msg1.mtype = msg.id_pacjent;
            // Wyslij pacjenta do domu
            if (msgsnd(msg_id_wyjscie, &msg1, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do domu\n");
                exit(1);
            }
            // signalSemafor(sem_id, 1);   // Informuj pacjenta, ze moze wyjsc z budynku
            signalSemafor(sem_id, 3);   // pozwol innym dzialac na pamieci dzielonej
            continue;
            // obsluga braku miejsc do danego lekarza
        }
        signalSemafor(sem_id, 3);   // pozwol innym dzialac na pamieci dzielonej
        
        printGreen("[Rejestracja - 1 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
        msg.mtype = msg.vip; // ustalenie priorytetu przyjecia pacjenta
        // Wyslij pacjenta do lekarza
        if (msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
            perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do lekarza\n");
            exit(1);
        }
        
        // Sprawdz liczbe procesow oczekujacych na rejestracje ponownie
        int liczba_procesow = policzProcesy(msg_id_rej);
        if ((liczba_procesow > BUILDING_MAX / 2) && (pid_okienka2 < 0)) // jezeli PID < 0 to znaczy ze okno2 jeszcze nie dziala
        {
            // Uruchomienie okienka nr 2
            uruchomOkienkoNr2(msg_id_rej, sem_id);
        }
        else if (liczba_procesow < (BUILDING_MAX / 3) && (pid_okienka2 > 0))
        {
            // Jesli liczba procesow spadla ponizej N/3, zatrzymaj okienko nr 2
            zatrzymajOkienkoNr2();
        }
        
        // Proces rejestracji kontynuuje swoja prace
        sleep(2);
    }



}