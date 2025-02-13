#include "lekarz.h"


// #define SLEEP // zakomentowac, jesli nie chcemy sleepow w programie

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        perror_red("[Lekarz]: Nieprawidlowa liczba argumentow\n");
        exit(1);
    }
    id_lekarz = atoi(argv[1]);
    int limit_pacjentow = atoi(argv[2]);
    zwrocTabliceLimitowLekarzy(limit_pacjentow, limity_lekarzy);
    const char *stringTp = argv[3];
    const char *stringTk = argv[4];
    Tp = naSekundy(stringTp);
    Tk = naSekundy(stringTk);

    //____________________   INICJALIZACJA LEKARZA   ______________________
    Lekarz lekarz;

    int limit_indywidualny = 0;
    switch(id_lekarz){
        case POZ:
            limit_indywidualny = limity_lekarzy[0];
            break;
        case KARDIOLOG:
            limit_indywidualny = limity_lekarzy[1];
            break;
        case OKULISTA:
            limit_indywidualny = limity_lekarzy[2];
            break;
        case PEDIATRA:
            limit_indywidualny = limity_lekarzy[3];
            break;
        case MEDYCYNA_PRACY:
            limit_indywidualny = limity_lekarzy[4];
            break;
        default:
            break;
    }
    inicjalizuj_lekarza(&lekarz, id_lekarz, limit_indywidualny);

    //  ____________________   INICJALIZACJA  ZASOBOW IPC   _____________________

    klucz_sem = generuj_klucz_ftok(".",'A');
    sem_id = alokujSemafor(klucz_sem, S, IPC_CREAT | 0600);
    
    if(lekarz.id_lekarz == 1){
        // tylko lekarze POZ potrzebuja dostepu do rejestracji w razie potrzeby skierowania do specjalisty
        klucz_kolejki_rejestracji = generuj_klucz_ftok(".", 'B');
        msg_id_rejestracja = alokujKolejkeKomunikatow(klucz_kolejki_rejestracji, IPC_CREAT | 0600);
    }

    klucz_wyjscia = generuj_klucz_ftok(".", 'W');
    msg_id_wyjscie = alokujKolejkeKomunikatow(klucz_wyjscia, IPC_CREAT | 0600);

    klucz_kolejki_lekarza = generuj_klucz_ftok(".",lekarz.id_lekarz);
    // klucz generowany jest na bazie znaku reprezentujacego lekarza
    msg_id_lekarz = alokujKolejkeKomunikatow(klucz_kolejki_lekarza, IPC_CREAT | 0600);

    // ______________________ OBSLUGA HANDLERA _____________________________

    struct sigaction ctrlc;
    ctrlc.sa_handler = obsluga_SIGINT; // Funkcja obslugujaca SIGINT
    sigemptyset(&ctrlc.sa_mask);       // Wyzerowanie maski sygnalow
    ctrlc.sa_flags = SA_RESTART | SA_NOCLDSTOP; // To samo co przy pacjencie
    if(sigaction(SIGINT, &ctrlc, NULL) == -1) {
        perror_red("[Lekarz]: Blad sigaction dla SIGINT\n");
        exit(1);
    }

    struct sigaction usr1;
    usr1.sa_handler = obsluga_SIGUSR1;
    sigemptyset(&usr1.sa_mask);
    usr1.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if(sigaction(SIGUSR1, &usr1, NULL) == -1) {
        perror_red("[Lekarz]: Blad sigaction dla SIGUSR1\n");
        exit(1);
    }


    //  ___________________________ OBSLUGA WATKU POZ2 ___________________________
    if (lekarz.id_lekarz==1){
        limit_POZ2 = lekarz.indywidualny_limit/2;
        lekarz.indywidualny_limit -= limit_POZ2;

        Lekarz lekarz2 = lekarz;
        int utworz_drugiegoPOZ = pthread_create(&POZ2,NULL,lekarzPOZ2,(void*)&lekarz2);
        if (utworz_drugiegoPOZ==-1) {
            perror_red("[Lekarz]: Blad pthread_create - tworzenie drugiego lekarza POZ\n");
            exit(1);
        }

        printMagenta("[Lekarz]: Wygenerowano 2 lekarzy: %s o id: %d, limity pacjentow: %d, %d\n", lekarz.nazwa, lekarz.id_lekarz, lekarz.indywidualny_limit, limit_POZ2);
        }
    else printMagenta("[Lekarz]: Wygenerowano lekarza: %s o id: %d, limit pacjentow: %d\n", lekarz.nazwa, lekarz.id_lekarz, lekarz.indywidualny_limit);
    // ________________________________________________________________________________________________________

    if(lekarz.id_lekarz==1) sprintf(lekarz.nazwa, "POZ1");
    czynnosci_lekarskie(&lekarz);   // symulacja pracy lekarza

    if (lekarz.id_lekarz == 1) {
    pthread_join(POZ2, NULL);  // Upewniamy sie, ze drugi lekarz POZ zakonczyl prace
    }
    return 0;
}


void czynnosci_lekarskie(Lekarz *lekarz){
    // Funkcja symulujaca prace lekarza
   

    // Aktualny czas

    int current_time;

    //  Czekaj na godzine rozpoczecia dzialania przychodni
    while(1){
        if(zwrocObecnyCzas() < Tp) continue;
        
        else break;
    }
    
    
    // Glowna petla dzialania lekarza
    while(koniec_pracy == 0 && zakoncz_program == 0){

        Wiadomosc msg;
        // Sprawdz aktualny czas
        current_time = zwrocObecnyCzas();

        // sprawdza czy na poczatku dzialania limit przyjec juz nie zostal osiagniety
        if(lekarz->licznik_pacjentow >= lekarz->indywidualny_limit) {
            printMagenta("[%s]: Lekarz o id %d osiagnal limit pacjentow i konczy prace\n", lekarz->nazwa, lekarz->id_lekarz);
            break;
            }

        // Sprawdz, czy aktualny czas jest poza godzinami otwarcia
        if (current_time > Tk) {
            printMagenta("[%s]: Przychodnia jest zamknieta. Lekarz o id: %d konczy prace.\n", lekarz->nazwa, lekarz->id_lekarz);
            printMagenta("[%s]: Po zamknieciu przychodni obsluzono pacjentow:\n", lekarz->nazwa);
            wypiszIOdeslijPacjentow(lekarz, msg_id_lekarz);
        
            #ifdef SLEEP
            sleep(8);
            #endif
        
            break; // Wyjscie z petli gdyczas dzialania sie konczy
        }
        

        if(zakoncz_program == 0 && zwrocObecnyCzas() < Tk){
            if (msgrcv(msg_id_lekarz, &msg, sizeof(Wiadomosc) - sizeof(long), -2, IPC_NOWAIT) == -1)        {
            // Odbieranie komunikatow o pacjentach w kolejnosci: 0,1,2
            //  typ 0 - odpowiada najwyzszemu priorytetowi, zostaje przyjety pierwszy (powrot z badan ambulatoryjnych)
            //  typ 1 - odpowiada wysokiemu priorytetowi - standardowy vip
            //  typ 2 - odpowiada niskiemu priorytetowi - zwykly pacjent
            // W razie braku komuniaktow nie zawieszaj dzialania i pracuj w petli (idz dalej)

                if (errno != ENOMSG)
                {
                    perror_red("[Lekarz]: Blad msgrcv\n");
                    exit(1);
                }
            }
        
            else{
                // Gdy zostanie odczytany komunikat wykonaj czynnosci:


            //______________________________________________________________
            waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
    
            FILE *raport = fopen("raport", "a");
            if (raport == NULL) {
                perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
            } else {
    
                fprintf(raport, "PACJENT %d PRZYJETY DO LEKARZA %d\n", msg.id_pacjent, msg.id_lekarz);
                fflush(raport);
                fclose(raport);
            }
            signalSemafor(sem_id, 4); 
            //______________________________________________________________ 


                printMagenta("[%s]: Lekarz o id %d przyjal pacjenta %d o priorytecie: %d\n",lekarz->nazwa, lekarz->id_lekarz, msg.id_pacjent, msg.vip);
                lekarz->licznik_pacjentow++;
                if(lekarz->licznik_pacjentow >= lekarz->indywidualny_limit && zwrocObecnyCzas() < Tk){
                    printMagenta("[%s]: Lekarz o id %d osiagnal limit pacjentow i konczy prace\n", lekarz->nazwa, lekarz->id_lekarz);

                    msg.mtype = msg.id_pacjent;
                    // Wyslij pacjenta do domu
                    if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                        perror_red("[Lekarz]: Blad msgsnd - pacjent do domu\n");
                        exit(1);
                    }
                    
                    #ifdef SLEEP
                    sleep(4);
                    #endif
                    
                    break;
                }
                
                int czy_dodatkowe_badania = losuj_int(100);
                
                // obsluga wysylania na dodatkowe badania/do specjalisty
                if(lekarz->id_lekarz == 1 && czy_dodatkowe_badania < 20 && zwrocObecnyCzas() < Tk){
                            
                    wyslij_do_specjalisty(&msg, lekarz);
                    fflush(stdout);
                    
                    #ifdef SLEEP
                    sleep(4);
                    #endif

                    continue;
                }

                if(lekarz->id_lekarz != 1 && czy_dodatkowe_badania < 10 && zwrocObecnyCzas() < Tk){
                            
                    badania_ambulatoryjne(&msg, lekarz);
                    fflush(stdout);
                    --lekarz->licznik_pacjentow;
                    // zmniejszam licznik pacjentow, bo dzieki temu pacjent, ktory
                    // wroci z badan ambulatoryjnych nie bedzie liczony x2
                    
                    #ifdef SLEEP
                    sleep(4);
                    #endif

                    continue;
                }
                
                // Informuj pacjenta, ze moze wyjsc z budynku
                Wiadomosc msg1 = msg;
                msg1.mtype = msg.id_pacjent;
                // Wyslij pacjenta do domu
                if (msgsnd(msg_id_wyjscie, &msg1, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    perror_red("[Lekarz]: Blad msgsnd - pacjent do domu\n");
                    exit(1);
                }
                
                #ifdef SLEEP
                sleep(4);
                #endif
            }

        }

    }

    if (koniec_pracy == 1) {
        
        printMagenta("[%s]: Przyjeto sygnal od Dyrektora. Lekarz konczy prace.\nNieobsluzeni pacjenci do raportu:\n", lekarz->nazwa);
        int kolejka_size;
        Wiadomosc *pozostali = wypiszPacjentowWKolejce(msg_id_lekarz, &kolejka_size, lekarz);
    
        if(pozostali  != NULL){
            waitSemafor(sem_id, 4, 0);  // blokada dostępu do pliku "raport"
            FILE *raport = fopen("raport", "a");
            if (raport == NULL) {
                perror_red("[Lekarz]: Blad otwarcia pliku raport\n");
                exit(1);
            }
        
            int j;
            for (j = 0; j < kolejka_size; j++) {
                pozostali[j].mtype = pozostali[j].id_pacjent;
                if (msgsnd(msg_id_wyjscie, &pozostali[j], sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    perror_red("[Lekarz]: Blad msgsnd - pacjent do domu\n");
                    continue;
                }
        
                time_t now = time(NULL);
                struct tm *local = localtime(&now);
                fprintf(raport, "[%s]: %02d:%02d:%02d - pacjent nr %d w kolejce lekarza po sygnale dyrektora, skierowany od %s do id: %d\n",
                        lekarz->nazwa, local->tm_hour, local->tm_min, local->tm_sec,
                        pozostali[j].id_pacjent, pozostali[j].kto_skierowal, pozostali[j].id_lekarz);
                fflush(raport);
            }
            fclose(raport);
            signalSemafor(sem_id, 4);
        
            if (pozostali != NULL) {
                free(pozostali); // zwalniam dynamicznie alokowana pamiec
            }
        }
    }
    
    return;
}


void *lekarzPOZ2(void *_arg) {
// Funkcja do wykonania przez watek lekarza POZ2
    
    Lekarz *lekarz= (Lekarz *) _arg;
    strncpy(lekarz->nazwa, "POZ2", sizeof(lekarz->nazwa) - 1); // Kopiowanie nazwy
    printMagenta("[%s]: Lekarz rozpoczal dzialanie\n",lekarz->nazwa);
    lekarz->indywidualny_limit = limit_POZ2; // tutaj sie przydaje ta zmienna globalna
    czynnosci_lekarskie(lekarz); 
    return NULL;
}

void wyslij_do_specjalisty(Wiadomosc *msg, Lekarz *lekarz){
//  funkcja potrzebna do przekierowania pacjenta do specjalisty

    strcpy(msg->kto_skierowal, lekarz->nazwa);  // daje informacje o tym, kto skierowal pacjenta dalej
    msg->id_lekarz = losuj_int(3)+2;    // losuje pacjenta o id 2-5 (specjalisci)

    if (msgsnd(msg_id_rejestracja, msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror_red("[POZ]: Blad msgsnd - wyslanie pacjenta do specjalisty\n");
        exit(1);
    }
    printMagenta("[%s]: lekarz przekierowal pacjenta nr %d do lekarza nr %d\n", lekarz->nazwa, msg->id_pacjent, msg->id_lekarz);
}

void badania_ambulatoryjne(Wiadomosc *msg, Lekarz *lekarz){
// funkcja potrzebna dla lekarza specjalisty, aby wyslac pacjenta na badania ambulatoryjne

    printMagenta("[Badania amb.]: pacjent nr %d zostal przyjety na badania; od lekarza nr %d\n", msg->id_pacjent, lekarz->id_lekarz);
    msg->vip = 0;   // nadal pacjentowi najwyzszy priorytet, aby wszedl do lekarza bez kolejki
    
    if (msgsnd(msg_id_lekarz, msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        // wysylam bezposredni do lekarza, bez rejestracji
        perror_red("[Badania amb.]: Blad msgsnd - wyslanie pacjenta do specjalisty\n");
        exit(1);
    }

    printMagenta("[Badania amb.]: pacjent nr %d skonczyl badania\n", msg->id_pacjent);
}

void obsluga_SIGUSR1(int sig){
    printYellow("%d : ODEBRANO SYGNAL OD DYREKTORA\n", getpid());
    koniec_pracy = 1;

    key_t shm_key = generuj_klucz_ftok(".", 'X');
    int shm_id = alokujPamiecWspoldzielona(shm_key, PAM_SIZE * sizeof(int), IPC_CREAT | 0600);
    int *pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shm_id, 0);

    klucz_sem = generuj_klucz_ftok(".", 'A');
    sem_id = alokujSemafor(klucz_sem, S, IPC_CREAT | 0600);

    waitSemafor(sem_id, 3, 0);
    pamiec_wspoldzielona[0] += limity_lekarzy[id_lekarz-1] - pamiec_wspoldzielona[id_lekarz];   //dodaje tylu pacjentow, aby symulowalo brak miejsc
    pamiec_wspoldzielona[id_lekarz] = limity_lekarzy[id_lekarz-1];  // zakomunikowanie do rejestracji, zeby juz nie przyjmowali do tego lekarza
    signalSemafor(sem_id, 3);

}

void obsluga_SIGINT(int sig) {
    zakoncz_program = 1;

    if (POZ2) {  // Sprawdz, czy watek zostal utworzony
        pthread_join(POZ2, NULL);
    }
    
    exit(0);
}
