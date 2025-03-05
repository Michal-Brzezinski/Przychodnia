#include "rejestracja.h"

// ________________________________________________________________________________
// #define SLEEP // zakomentowac, jesli nie chcemy sleepow w generowaniu pacjentow  <--- DO TESTOWANIA
// W PLIKU UTILS.H -> DLA WSZYSTKICH PLIKOW 
// ________________________________________________________________________________

int main(int argc, char *argv[])
{
    
    if (argc != 5)
    {
        perror_red("[Rejestracja]: Nieprawidlowa liczba argumentow");
        exit(1);
    }
    
    limit_pacjentow = atoi(argv[1]);
    const char *stringTp = argv[2];
    const char *stringTk = argv[3];
    building_max = atoi(argv[4]);

    Tp = naSekundy(stringTp);
    Tk = naSekundy(stringTk);

    signal(SIGUSR2, zatrzymajOkienkoNr2);

    // _____________________    DOLACZENIE ZASOBOW IPC   ______________________
    
    msg_key_rej = generuj_klucz_ftok(".", 'B');
    msg_id_rej = alokujKolejkeKomunikatow(msg_key_rej, IPC_CREAT | 0600);

    klucz_wyjscia = generuj_klucz_ftok(".", 'W');
    msg_id_wyjscie = alokujKolejkeKomunikatow(klucz_wyjscia, IPC_CREAT | 0600);
    
    sem_key = generuj_klucz_ftok(".", 'A');
    sem_id = alokujSemafor(sem_key, S, IPC_CREAT | 0600);

    shm_key_przyjecia = generuj_klucz_ftok(".", 'X');
    shm_id_przyjecia = alokujPamiecWspoldzielona(shm_key_przyjecia, PAM_SIZE * sizeof(int), IPC_CREAT | 0600);
    przyjecia = dolaczPamiecWspoldzielona(shm_id_przyjecia, 0);

    shm_key_dostepnosc = generuj_klucz_ftok(".", 'D');
    shm_id_dostepnosc = alokujPamiecWspoldzielona(shm_key_dostepnosc, DOSTEPNOSC * sizeof(sig_atomic_t), IPC_CREAT | 0600);
    dostepnosc_lekarza = dolaczPamiecWspoldzielona(shm_id_dostepnosc, 0);
    
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
    
    printGreen("[Rejestracja]: Wygenerowano proces rejestracji\n");

    // do tablicy limity_lekarzy przypisywane sa poszczegolne limity
    zwrocTabliceLimitowLekarzy(limit_pacjentow, limity_lekarzy);
    
    printMagenta("Odczytane limity lekarzy to:\n");
    printMagenta("WSZYSCY: \t%d\n", limit_pacjentow);
    printMagenta("POZ:\t%d\n",limity_lekarzy[POZ-1]);
    printMagenta("KARDIOLOG:\t%d\n",limity_lekarzy[KARDIOLOG-1]);
    printMagenta("OKULISTA:\t%d\n",limity_lekarzy[OKULISTA-1]);
    printMagenta("PEDIATRA:\t%d\n",limity_lekarzy[PEDIATRA-1]);
    printMagenta("MEDYCYNA PRACY:\t%d\n", limity_lekarzy[MEDYCYNA_PRACY-1]);

    //  Czekaj na godzine rozpoczecia dzialania przychodni
    while(1){
        
        if(zwrocObecnyCzas() < Tp) continue;
        
        else break;
    }
        
    signalSemafor(sem_id, 2);   // podnosze semafor, ktory mowi o tym ze rejestracja jest uruchomiona
    printGreen("[Rejestracja]: Rejestracja uruchomiona, oczekuje na pacjentow\n");
    
    Wiadomosc msg;

    while (1)
    {
        
        // Sprawdz, czy aktualny czas jest poza godzinami otwarcia
        if (zwrocObecnyCzas() > Tk)
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
                continue;
            }

            #ifdef SLEEP
            sleep(2); // Czekaj 2 sekundy i sprawdz ponownie
            #endif
            
            continue;
        }    
        
        signalSemafor(sem_id, 7);   // zwieksz licznik miejsca w kolejce do rejestracji

        int msg_id_pom; // przechowuje tymczasowo id danej kolejki w zaleznosci od wartosci w wiadomosci
        switch(msg.id_lekarz){
            // przypisanie zmiennej pomocniczej odpowiedniej wartosci id kolejki lekarza
            case POZ: msg_id_pom = msg_id_1; break;
            case KARDIOLOG: msg_id_pom = msg_id_2; break;
            case OKULISTA: msg_id_pom = msg_id_3; break;
            case PEDIATRA: msg_id_pom = msg_id_4; break;
            case MEDYCYNA_PRACY: msg_id_pom = msg_id_5; break;
        }

        waitSemafor(sem_id, 3, 0);  // czekam na mozliowosc bezpiecznej operacji na liczniku przyjec
        
        if(dostepnosc_lekarza[msg.id_lekarz - 1] == 0){
            printYellow("[Rejestracja - 1 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - lekarz niedostepny\n",msg.id_pacjent ,msg.id_lekarz);

            msg.mtype = msg.id_pacjent;

            waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
            if(kill(msg.id_pacjent, 0) == 0){
                if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce wyjscia (semafor)

                    perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do domu\n");
                }
            }

            signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec

            continue;
        }
        
        if(limity_lekarzy[msg.id_lekarz-1]<=(przyjecia[msg.id_lekarz])){
            // obsluga braku miejsc do danego lekarza

            printYellow("[Rejestracja - 1 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
            
            // Wyslij pacjenta do domu
            msg.mtype = msg.id_pacjent; // zmiana typu, aby kazdy pacjent mogl odebrac odpowiednia wiadomosc o typie id_pacjent=PID
            
            waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
            if(kill(msg.id_pacjent, 0) == 0){
                if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce wyjscia (semafor)

                    perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do domu\n");
                }
            }

            //_______________   WPISANIE NIEPRZYJETEGO PACJENTA DO RAPORTU  _____________________
            
            // Uzyskanie wylacznego dostepu do pliku raport poprzez semafor nr 4
            waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku "raport"
            // Otwarcie pliku "raport" – tworzy, jesli nie istnieje; tryb "a" dopisuje linie
            FILE *raport = fopen("raport", "a");
            if (raport == NULL) {
                perror_red("[Rejestracja - 1 okienko]: Blad otwarcia pliku raport\n");
            } else {
                time_t now = time(NULL);
                struct tm *local = localtime(&now);
                fprintf(raport, "[Rejestracja - 1 okienko]: %02d:%02d:%02d - pacjent nr %d nie przyjety do lekarza %d (brak miejsc), skierowany od %s\n",
                        local->tm_hour, local->tm_min, local->tm_sec,
                        msg.id_pacjent, msg.id_lekarz, msg.kto_skierowal);
                fflush(raport);
                fclose(raport);
            }
            signalSemafor(sem_id, 4);  // zwolnienie dostepu do pliku "raport"

            // _____________________________________________________________________________________
            
            
            signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec

            continue;
        }

        printGreen("[Rejestracja - 1 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
        
        // Wyslij pacjenta do lekarza
        msg.mtype = msg.vip; // ustalenie priorytetu przyjecia pacjenta
        errno = 0;
        waitSemafor(sem_id, 8+msg.id_lekarz, 0);  // czekaj na miejsce w kolejce do danego lekarza
        if (((kill(msg.id_pacjent, 0) == 0) && (msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1)) || zwrocObecnyCzas() > Tk) {
            signalSemafor(sem_id, 8+msg.id_lekarz);  // zwieksz licznik miejsc w kolejce do lekarza
            
            if(errno != 0){
                perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do lekarza\n");
            }
            
            msg.mtype = msg.id_pacjent;
            // Wyslij pacjenta do domu

            waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
            if(kill(msg.id_pacjent, 0) == 0){
                if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent do domu\n");
                    
                    signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce do wyjscia
                }
            }
            signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec
                      
            
            continue;
        }

        ++przyjecia[msg.id_lekarz];   // zwieksz +1 liczbe przyjec do lekarza o podanym id
        ++przyjecia[0]; // zwieksz glowny licznik przyjec
        printGreen("[Rejestracja - 1 okienko]: Obecny licznik przyjec lekarza %d po inkrementacji: %d\n", msg.id_lekarz, przyjecia[msg.id_lekarz]);
        // licznik sie zwieksza wylacznie, gdy wyslanie wiadomosci do lekarza sie powiodlo
        
        waitSemafor(sem_id, 6, 0);
        if((przyjecia[0] >= limit_pacjentow) && (valueSemafor(sem_id, 5) == 1)){
            waitSemafor(sem_id, 5, 0);
            printYellow("[Rejestracja - 1 okienko]: Wyczerpano limit przyjec na ten dzien, zablokowano wejscie pacjentow do budynku\n");
        
            odeslijPacjentowPrzekroczenieLimitu(1);
            odeslano_pacjentow_po_osiagnieciu_limitu1 = 1;
        }

        if((valueSemafor(sem_id, 5) == 0) && odeslano_pacjentow_po_osiagnieciu_limitu1 == 0){
        odeslijPacjentowPrzekroczenieLimitu(1);
        odeslano_pacjentow_po_osiagnieciu_limitu1 = 1;
        }
        signalSemafor(sem_id, 6);

        signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec

        // Sprawdz liczbe procesow oczekujacych na rejestracje ponownie
        int liczba_procesow = policzProcesy(msg_id_rej);
        printYellow("[Rejestracja - 1 okienko]: Liczba oczekujacych w kolejce do rejestracji:\t%d\n", liczba_procesow);
        // if ((liczba_procesow > building_max / 2) && (pid_okienka2 < 0)) // jezeli PID < 0 to znaczy ze okno2 jeszcze nie dziala
        // {
        //     // Uruchomienie okienka nr 2
        //     zakoncz2okienko = 0;
        //     uruchomOkienkoNr2(msg_id_rej, sem_id);
        // }
        // else if ((liczba_procesow < (building_max / 3) && (pid_okienka2 > 0)) || zakoncz2okienko == 1)
        // {
        //     // Jesli liczba procesow spadla ponizej N/3 lub osiagnieto limit przyjec, zatrzymaj okienko nr 2
        //     zatrzymajOkienkoNr2();
        // }
        
        // Proces rejestracji kontynuuje swoja prace
        
        #ifdef SLEEP
        sleep(2);
        #endif
    }
    zatrzymajOkienkoNr2(); // Zatrzymaj okienko nr 2 przed zakonczeniem pracy
    //
    waitSemafor(sem_id, 6, 0);
    if(valueSemafor(sem_id, 5) > 0){
        waitSemafor(sem_id, 5, 0);  // poinformuj ze budynek zamkniety 
        printYellow("[Rejestracja]: Zablokowano wejscie nowych pacjentow do budynku\n");    
    }
    signalSemafor(sem_id, 6);
    waitSemafor(sem_id, 2, 0);        // Oznajmij zakonczenie rejestracji

    printYellow("[Rejestracja]: Czekam na sygnal zakonczenia generowania pacjentow...\n");
    printYellow("[Rejestracja]: Rejestracja zakonczyla dzialanie\n");
    system("killall -SIGUSR1 mainprog > /dev/null 2>&1");

    // wysylanie pozostalym pacjentom komunikatu o wyjsciu
    // takze wypisz pacjentow, ktorzy byli w kolejce do rejestracji w momencie zamykania rejestracji
    int rozmiar_pozostalych = 0;
    printGreen("[Rejestracja]: Pacjenci w kolejce do rejestracji w momencie zamykania rejestracji:\n");
    Wiadomosc *pozostali = wypiszPacjentowWKolejce(msg_id_rej, &rozmiar_pozostalych);
    int i;
    
    if (pozostali != NULL) {

        waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku "raport"
        FILE *raport = fopen("raport", "a");
        if (raport == NULL) {
            perror_red("[Rejestracja]: Blad otwarcia pliku raport\n"); 
            signalSemafor(sem_id, 4);  // zwolnienie dostepu do pliku "raport"
            exit(1);
        }
        if (rozmiar_pozostalych > 0) {
            for(i=0;i<rozmiar_pozostalych;i++){    

                pozostali[i].mtype = pozostali[i].id_pacjent;
                // Wyslij pacjenta do domu

                waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
                if(kill(pozostali[i].id_pacjent, 0) == 0){
                    if (msgsnd(msg_id_wyjscie, &pozostali[i], sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                        perror_red("[Rejestracja]: Blad msgsnd - pacjent do domu\n");
                        signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce do wyjscia
                        
                        continue;
                    }
                }

                time_t now = time(NULL);
                struct tm *local = localtime(&now);
                fprintf(raport, "[Rejestracja]: %02d:%02d:%02d - pacjent nr %d w kolejce po zakonczeniu rejestracji, skierowany od %s do id: %d\n",
                        local->tm_hour, local->tm_min, local->tm_sec,
                        pozostali[i].id_pacjent, pozostali[i].kto_skierowal, pozostali[i].id_lekarz);
                fflush(raport);
                
            }
        }
        fclose(raport);
        signalSemafor(sem_id, 4);  // zwolnienie dostepu do pliku "raport"
        
        free(pozostali);
        pozostali = NULL; 
    }
    
    return 0;
}

void uruchomOkienkoNr2()
{
    // Uruchomienie dodatkowego procesu rejestracji (okienka nr 2)
    
    pid_okienka2 = fork();

    if (pid_okienka2 == -1)
    {
        perror_red("[Rejestracja - 2 okienko]: Blad fork\n");
        exit(1);
    }

    else if (pid_okienka2 == 0)
    {
        signal(SIGTERM, handlerSIGUSR2);    // obsluga przyjecia sygnalu o zakonczeniu pracy 2 okienka
        // Dziecko: Okienko nr 2
        printGreen("[Rejestracja - 2 okienko]: Otworzenie okienka ...\n");
        
        while (!zakoncz2okienko)
        {
            
            // Sprawdz, czy aktualny czas jest poza godzinami otwarcia
            if (zwrocObecnyCzas() > Tk)
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
                    continue;
                }
                
                #ifdef SLEEP
                sleep(2); // Czekaj 2 sekundy i sprawdz ponownie
                #endif
                
                continue;
            }

            signalSemafor(sem_id, 7);   // zwieksz licznik miejsca w kolejce do rejestracji

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
            
            if(dostepnosc_lekarza[msg.id_lekarz - 1] == 0){
                printYellow("[Rejestracja - 2 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - lekarz niedostepny\n",msg.id_pacjent ,msg.id_lekarz);
    
                msg.mtype = msg.id_pacjent;

                waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
                if(kill(msg.id_pacjent, 0) == 0){
                    if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                        signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce wyjscia (semafor)
        
                        perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do domu\n");
                    }
                }
                signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec
    
                continue;
            }
            
            if((limity_lekarzy[msg.id_lekarz-1]<=(przyjecia[msg.id_lekarz])) && dostepnosc_lekarza[msg.id_lekarz - 1] != 0){
                // obsluga braku miejsc do danego lekarza

                printYellow("[Rejestracja - 2 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
                
                msg.mtype = msg.id_pacjent;
                // Wyslij pacjenta do domu
                
                waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
                if(kill(msg.id_pacjent, 0) == 0){
                    if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                        perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do domu\n");
                        signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce do wyjscia
                    
                    }
                }
                //_______________   WPISANIE NIEPRZYJETEGO PACJENTA DO RAPORTU  _____________________
                
                // Uzyskanie wylacznego dostepu do pliku raport poprzez semafor nr 4
                waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku "raport"
                
                // Otwarcie pliku "raport" – tworzy, jesli nie istnieje; tryb "a" dopisuje linie
                FILE *raport = fopen("raport", "a");
                if (raport == NULL) {
                    perror_red("[Rejestracja - 2 okienko]: Blad otwarcia pliku raport\n");
                } else {
                    time_t now = time(NULL);
                    struct tm *local = localtime(&now);
                    fprintf(raport, "[Rejestracja - 2 okienko]: %02d:%02d:%02d - pacjent nr %d nie przyjety do lekarza %d (brak miejsc), skierowany od %s\n",
                            local->tm_hour, local->tm_min, local->tm_sec,
                            msg.id_pacjent, msg.id_lekarz, msg.kto_skierowal);
                    fflush(raport);
                    fclose(raport);
                }
                signalSemafor(sem_id, 4);  // zwolnienie dostepu do pliku "raport"

                // _____________________________________________________________________________________
                
                signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec
                continue;
            }

            printGreen("[Rejestracja - 2 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
            
            // Wyslij pacjenta do lekarza
            msg.mtype = msg.vip; // ustalenie priorytetu przyjecia pacjenta
            errno = 0;
            
            waitSemafor(sem_id, 8+msg.id_lekarz, 0);  // czekaj na miejsce w kolejce do lekarza
            if ((kill(msg.id_pacjent, 0) == 0) && ((msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) || zwrocObecnyCzas() > Tk) && dostepnosc_lekarza[msg.id_lekarz - 1] != 0) {
                signalSemafor(sem_id, 8+msg.id_lekarz);  // zwieksz licznik miejsc w kolejce do lekarza
                
                if(errno != 0){
                    perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do lekarza\n");
                }

                msg.mtype = msg.id_pacjent;
                // Wyslij pacjenta do domu

                waitSemafor(sem_id, 8, 0);  // czekaj az znajdzie sie miejsce w kolejce do wyjscia
                if(kill(msg.id_pacjent, 0) == 0){
                    if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                        signalSemafor(sem_id, 8);  // zwieksz licznik miejsc w kolejce do wyjscia
                        
                        perror_red("[Rejestracja - 2 okienko]: Blad msgsnd - pacjent do domu\n");
                    }
                }
                signalSemafor(sem_id, 3);   // informuje o mozliwosci dzialania na tablicy przyjec

                continue;
            }

            ++przyjecia[msg.id_lekarz];   // zwieksz +1 liczbe przyjec do lekarza o podanym id
            ++przyjecia[0]; // zwieksz glowny licznik przyjec
            printGreen("[Rejestracja 2 okno]: Obecny licznik przyjec lekarza %d po inkrementacji: %d\n", msg.id_lekarz, przyjecia[msg.id_lekarz]);
            // licznik sie zwieksza wylacznie, gdy wyslanie wiadomosci do lekarza sie powiodlo
            
            waitSemafor(sem_id, 6, 0);
            if((przyjecia[0] >= limit_pacjentow) && (valueSemafor(sem_id, 5) == 1)){
 
                waitSemafor(sem_id, 5, 0);
                printYellow("[Rejestracja - 2 okienko]: Wyczerpano limit przyjec na ten dzien, zablokowano wejscie pacjentow do budynku\n");

                odeslijPacjentowPrzekroczenieLimitu(2);
                odeslano_pacjentow_po_osiagnieciu_limitu2 = 1;
                zakoncz2okienko = 1;
                kill(getppid(), SIGUSR2);
            }

            else if((valueSemafor(sem_id, 5) == 0) && odeslano_pacjentow_po_osiagnieciu_limitu2 == 0){
                odeslijPacjentowPrzekroczenieLimitu(2);
                odeslano_pacjentow_po_osiagnieciu_limitu2 = 1;
                zakoncz2okienko = 1;
                kill(getppid(), SIGUSR2);
            }
            
            signalSemafor(sem_id, 6);
            signalSemafor(sem_id, 3); 

            #ifdef SLEEP
            sleep(2); // symulacja procesu rejestracji
            #endif
        }
        exit(0);
    }
}
void zatrzymajOkienkoNr2()
{        
    // Jesli proces okienka nr 2 istnieje, wyslij do niego sygnal zakonczenia
    if (pid_okienka2 != -1)
    {
        printYellow("[Rejestracja]: Zatrzymywanie okienka nr 2...\n");
        if(kill(pid_okienka2, 0) != -1) kill(pid_okienka2, SIGTERM); // Wyslac sygnal SIGTERM do procesu okienka nr 2 (zakonczenie), jezeli istnieje
        waitpid(pid_okienka2, NULL, 0); // Czekaj na zakonczenie procesu
        pid_okienka2 = -1;  // pomaga w uruchamianiu okienka w glownej petli
        //mowiac scislej zapobiega kilkukrotnemu uruchomieniu okienka nr2 naraz
    } else {
        // Jesli proces okienka nr 2 już nie istnieje, spróbuj usunąć potencjalny proces zombie
        waitpid(pid_okienka2, NULL, WNOHANG); // Czekaj na zakończenie procesu, ale bez zawieszania
        pid_okienka2 = -1;
    }

}

void handlerSIGUSR2(int signum){
    zakoncz2okienko = 1;
    return;
}
