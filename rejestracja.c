
#include "rejestracja.h"

int main(int argc, char *argv[])
{
    
    if (argc != 2)
    {
        perror_red("[Rejestracja]: Nieprawidlowa liczba argumentow");
        exit(1);
    }
    
    // _____________________    DOLACZENIE ZASOBOW IPC   ______________________
    
    msg_key_rej = generuj_klucz_ftok(".", 'B');
    msg_id_rej = alokujKolejkeKomunikatow(msg_key_rej, IPC_CREAT | 0600);

    klucz_wyjscia = generuj_klucz_ftok(".", 'W');
    msg_id_wyjscie = alokujKolejkeKomunikatow(klucz_wyjscia, IPC_CREAT | 0600);
    
    sem_key = generuj_klucz_ftok(".", 'A');
    sem_id = alokujSemafor(sem_key, S, IPC_CREAT | 0600);

    shm_key = generuj_klucz_ftok(".", 'X');
    shm_id = alokujPamiecWspoldzielona(shm_key, PAM_SIZE * sizeof(int), IPC_CREAT | 0600);
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shm_id, 0);
    
    msg_key_1 = generuj_klucz_ftok(".",1);
    msg_id_1 = alokujKolejkeKomunikatow(msg_key_1,IPC_CREAT | 0600);
    
    msg_key_2 = generuj_klucz_ftok(".",2);
    msg_id_2 = alokujKolejkeKomunikatow(msg_key_2,IPC_CREAT | 0600);
    
    msg_key_3 = generuj_klucz_ftok(".",3);
    msg_id_3 = alokujKolejkeKomunikatow(msg_key_3,IPC_CREAT | 0600);
    
    msg_key_4 = generuj_klucz_ftok(".",4);
    msg_id_4 = alokujKolejkeKomunikatow(msg_key_4,IPC_CREAT | 0600);
    
    msg_key_5 = generuj_klucz_ftok(".",5);
    msg_id_5 = alokujKolejkeKomunikatow(msg_key_5,IPC_CREAT | 0600);
    
    // ___________________  ROZPOCZECIE PRACY REJESTRACJI   _________________________
    waitSemafor(sem_id, 6, 0);
    signalSemafor(sem_id, 5);
    signalSemafor(sem_id, 6);
    printGreen("[Przychodnia]: Otwarto budynek\n");
    
    printGreen("[Rejestracja]: Uruchomiono rejestracje\n");
    
    limit_pacjentow = atoi(argv[1]);
    zwrocTabliceLimitowLekarzy(limit_pacjentow, limity_lekarzy);
    
    printMagenta("Odczytane limity lekarzy to:\n");
    printMagenta("WSZYSCY: \t%d\n", limit_pacjentow);
    printMagenta("POZ:\t%d\n",limity_lekarzy[0]);
    printMagenta("KARDIOLOG:\t%d\n",limity_lekarzy[1]);
    printMagenta("OKULISTA:\t%d\n",limity_lekarzy[2]);
    printMagenta("PEDIATRA:\t%d\n",limity_lekarzy[3]);
    printMagenta("MEDYCYNA PRACY:\t%d\n", limity_lekarzy[4]);
    
    // Aktualny czas
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
    
    // Godziny otwarcia i zamkniecia rejestracji (w sekundach od polnocy)
    Tp = current_time;      // Aktualny czas
    Tk = current_time + 50; // Aktualny czas + x sekund

    printGreen("[Rejestracja]: Rejestracja uruchomiona, oczekuje na pacjentow\n");
    signalSemafor(sem_id, 2);   // podnosze semafor, ktory mowi o tym ze rejestracja jest uruchomiona

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
            printYellow("[Rejestracja - 1 okienko]: Rejestracja jest zamknieta.\n");
            break; // Wyjscie z petli, gdy czas jest poza godzinami otwarcia
        }

        // Czekaj na komunikat dla rejestracji od pacjenta
        if (msgrcv(msg_id_rej, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) == -1)
        {
            // Sprawdzamy, czy nie bylo wiadomosci, ale nie blokujemy procesu dzieki IPC_NOWAIT
            if (errno != ENOMSG)
            {
                perror_red("[Rejestracja - 1 okienko]: Blad msgrcv - pacjent->rejestracja\n");
            }

            sleep(2); // Czekaj 2 sekundy i sprawdz ponownie
            continue;
        }    

        int msg_id_pom; 
        switch(msg.id_lekarz){
            // przypisanie zmiennej pomocniczej odpowiedniej wartosci id kolejki lekarza
            case POZ: msg_id_pom = msg_id_1; break;
            case KARDIOLOG: msg_id_pom = msg_id_2; break;
            case OKULISTA: msg_id_pom = msg_id_3; break;
            case PEDIATRA: msg_id_pom = msg_id_4; break;
            case MEDYCYNA_PRACY: msg_id_pom = msg_id_5; break;
        }

        waitSemafor(sem_id, 3, 0);  // czekam na mozliowosc bezpiecznej operacji na liczniku przyjec

        if(limity_lekarzy[msg.id_lekarz-1]<=(pamiec_wspoldzielona[msg.id_lekarz])){
            // obsluga braku miejsc do danego lekarza

            printYellow("[Rejestracja - 1 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
            Wiadomosc msg1 = msg;
            msg1.mtype = msg.id_pacjent;
            
            // Wyslij pacjenta do domu
            if (msgsnd(msg_id_wyjscie, &msg1, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do domu\n");
            }
            signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec

            continue;
        }

        printGreen("[Rejestracja - 1 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
        msg.mtype = msg.vip; // ustalenie priorytetu przyjecia pacjenta
        // Wyslij pacjenta do lekarza
        if (msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
            perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do lekarza\n");
            signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec
            
            msg.mtype = msg.id_pacjent;
            // Wyslij pacjenta do domu
            if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Rejestracja]: Blad msgsnd - pacjent do domu\n");
            }
            continue;
        }

        ++pamiec_wspoldzielona[msg.id_lekarz];   // zwieksz +1 liczbe przyjec do lekarza o podanym id
        ++pamiec_wspoldzielona[0]; // zwieksz glowny licznik przyjec
        printGreen("[Rejestracja 1 okno]: OBECNY LICZNIK PRZYJEC LEKARZA %d PO INKREMENTACJI: %d\n", msg.id_lekarz, pamiec_wspoldzielona[msg.id_lekarz]);
        // licznik sie zwieksza wylacznie, gdy wyslanie wiadomosci do lekarza sie powiodlo
        
        waitSemafor(sem_id, 6, 0);
        if((pamiec_wspoldzielona[0] >= limit_pacjentow) && (valueSemafor(sem_id, 5) == 1)){
              waitSemafor(sem_id, 5, 0);
              printYellow("Wyczerpano limit przyjec na ten dzien, zablokowano wejscie pacjentow do budynku\n");
        }
        signalSemafor(sem_id, 6);

        signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec

        // Sprawdz liczbe procesow oczekujacych na rejestracje ponownie
        int liczba_procesow = policzProcesy(msg_id_rej);
        printRed("Liczba oczekujacych w kolejce wynosi:\t%d\n", liczba_procesow);
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
    zatrzymajOkienkoNr2(); // Zatrzymaj okienko nr 2 przed zakonczeniem pracy
    //
    waitSemafor(sem_id, 6, 0);
    if(valueSemafor(sem_id, 5) > 0){
        waitSemafor(sem_id, 5, 0);  // poinformuj ze budynek zamkniety 
        printYellow("[Rejestracja]: Zablokowano wejscie nowych pacjentow do budynku\n");    
    }
    signalSemafor(sem_id, 6);
    //inicjalizujSemafor(sem_id, 0, 0);    //zmienic sposob blokady - nie wolno reinicjalizowac
    
    waitSemafor(sem_id, 2, 0);        // Oznajmij zakonczenie rejestracji

    printYellow("[Rejestracja]: Czekam na sygnal zakonczenia generowania pacjentow...\n");
    printYellow("[Rejestracja]: Rejestracja zakonczyla dzialanie\n");
    
    // wysylanie pozostalym pacjentom komunikatu o wyjsciu
    // takze wypisz pacjentow, ktorzy byli w kolejce do rejestracji w momencie zamykania rejestracji
    int rozmiar_pozostalych = 0;
    int *pidy_pozostalych = wypiszPacjentowWKolejce(msg_id_rej, sem_id, &rozmiar_pozostalych);
    Wiadomosc msg1 = msg;
    int i;
    for(i=0;i<rozmiar_pozostalych;i++){    
        msg1.mtype = pidy_pozostalych[i];
        msg1.id_pacjent = pidy_pozostalych[i];
        // Wyslij pacjenta do domu
        if (msgsnd(msg_id_wyjscie, &msg1, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
            perror_red("[Rejestracja]: Blad msgsnd - pacjent do domu\n");
            continue;
        }
    }
    free(pidy_pozostalych);
    
    return 0;
}

void uruchomOkienkoNr2()
{
    // Uruchomienie dodatkowego procesu rejestracji (okienka nr 2)
    pid_okienka2 = fork();
    if (pid_okienka2 == 0)
    {
        // Dziecko: Okienko nr 2
        printGreen("[Rejestracja - 2 okienko]: Otworzenie okienka ...\n");
        
        // Zmienne aktualnego czasu
        time_t now;
        struct tm *local;
        int current_time;
        
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
                printYellow("[Rejestracja - 2 okienko]: Przychodnia jest zamknieta.\n");
                break; // Wyjscie z petli, gdy czas jest poza godzinami otwarcia
            }


            // Czekaj na komunikat rejestracji
            Wiadomosc msg;
            if (msgrcv(msg_id_rej, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) == -1)
            {
                // Sprawdzamy, czy nie bylo wiadomosci, ale nie blokujemy procesu dzieki IPC_NOWAIT
                if (errno != ENOMSG)
                {
                    perror_red("[Rejestracja - 2 okienko]: Blad msgrcv\n");
                    zatrzymajOkienkoNr2();
                    exit(1);
                }
                
                sleep(2); // Czekaj 2 sekundy i sprawdz ponownie
                continue;
            }
            
            int msg_id_pom; 
            switch(msg.id_lekarz){
                // przypisanie zmiennej pomocniczej odpowiedniej wartosci id kolejki lekarza
                case POZ: msg_id_pom = msg_id_1; break;
                case KARDIOLOG: msg_id_pom = msg_id_2; break;
                case OKULISTA: msg_id_pom = msg_id_3; break;
                case PEDIATRA: msg_id_pom = msg_id_4; break;
                case MEDYCYNA_PRACY: msg_id_pom = msg_id_5; break;
            }

            waitSemafor(sem_id, 3, 0);
            if(limity_lekarzy[msg.id_lekarz-1]<=(pamiec_wspoldzielona[msg.id_lekarz])){
                // obsluga braku miejsc do danego lekarza

                printYellow("[Rejestracja - 2 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
                msg.mtype = msg.id_pacjent;
                
                // Wyslij pacjenta do domu
                if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do domu\n");
                }
                signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec
                continue;
            }


            printGreen("[Rejestracja - 2 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
            msg.mtype = msg.vip; // ustalenie priorytetu przyjecia pacjenta
            // Wyslij pacjenta do lekarza
            if (msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do lekarza\n");
                
                msg.mtype = msg.id_pacjent;
                // Wyslij pacjenta do domu
                if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do domu\n");
                }
                signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec
                continue;
            }
            ++pamiec_wspoldzielona[msg.id_lekarz];   // zwieksz +1 liczbe przyjec do lekarza o podanym id
            ++pamiec_wspoldzielona[0]; // zwieksz glowny licznik przyjec
            printGreen("[Rejestracja 2 okno]: OBECNY LICZNIK PRZYJEC LEKARZA %d PO INKREMENTACJI: %d\n", msg.id_lekarz, pamiec_wspoldzielona[msg.id_lekarz]);
            // licznik sie zwieksza wylacznie, gdy wyslanie wiadomosci do lekarza sie powiodlo
            
            waitSemafor(sem_id, 6, 0);
            if((pamiec_wspoldzielona[0] >= limit_pacjentow) && (valueSemafor(sem_id, 5) == 1)){
                waitSemafor(sem_id, 5, 0);
                printYellow("Wyczerpano limit przyjec na ten dzien, zablokowano wejscie pacjentow do budynku\n");
            }
            signalSemafor(sem_id, 6);
            
    
            signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec



            sleep(2); // symulacja procesu rejestracji
        }
    }
    else if (pid_okienka2 == -1)
    {
        perror_red("[Rejestracja - 2 okienko]: Blad fork\n");
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
        waitpid(pid_okienka2, NULL, 0); // Czekaj na zakończenie procesu
        pid_okienka2 = -1;  // pomaga w uruchamianiu okienka w glownej petli
        //mowiac scislej zapobiega kilkukrotnemu uruchomieniu okienka nr2 naraz
    }
    
}