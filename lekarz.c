#include "lekarz.h"


int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        perror_red("[Lekarz]: Nieprawidlowa liczba argumentow\n");
        exit(1);
    }
    int id_lekarz = atoi(argv[1]);
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

    struct sigaction sa;
    sa.sa_handler = obsluga_SIGINT; // Funkcja obslugujaca SIGINT
    sigemptyset(&sa.sa_mask);       // Wyzerowanie maski sygnalow
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // To samo co przy pacjencie

    sigaction(SIGINT, &sa, NULL);   // Ustawienie obsługi SIGINT



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
    time_t now;
    struct tm *local;
    int current_time;

    //  Czekaj na godzine rozpoczecia dzialania przychodni
    while(1){
        now = time(NULL);
        local = localtime(&now);
        current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
        if(current_time < Tp) sleep(5); //sprawdzaj czas co 5 sek
        else break;
        continue;
    }
    
    // Glowna petla dzialania lekarza
    while(1){
        
        Wiadomosc msg;
        // Sprawdz aktualny czas
        now = time(NULL);
        local = localtime(&now);
        current_time = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;
        
        // Sprawdz, czy aktualny czas jest poza godzinami otwarcia
        if (current_time > Tk)
        {
            printMagenta("[%s]: Przychodnia jest zamknieta. Lekarz o id: %d konczy prace.\n", lekarz->nazwa, lekarz->id_lekarz);
            
            wypiszIOdeslijPacjentow(lekarz, msg_id_lekarz);

            break; // Wyjscie z petli, gdy czas jest poza godzinami otwarcia
        }

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

            printMagenta("[%s]: Lekarz o id %d przyjal pacjenta %d o priorytecie: %d\n",lekarz->nazwa, lekarz->id_lekarz, msg.id_pacjent, msg.vip);
            lekarz->licznik_pacjentow++;
            if(lekarz->licznik_pacjentow >= lekarz->indywidualny_limit){
                printMagenta("[%s]: Lekarz o id %d osiagnal limit pacjentow i konczy prace\n", lekarz->nazwa, lekarz->id_lekarz);

                msg.mtype = msg.id_pacjent;
                // Wyslij pacjenta do domu
                if (msgsnd(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                    perror_red("[Lekarz]: Blad msgsnd - pacjent do domu\n");
                    exit(1);
                }

                break;
            }
            
            int czy_dodatkowe_badania = losuj_int(100);
            
            // obsluga wysylania na dodatkowe badania/do specjalisty
            if(lekarz->id_lekarz == 1 && czy_dodatkowe_badania < 20){
                        
                wyslij_do_specjalisty(&msg, lekarz);
                fflush(stdout);
                continue;
            }

            if(lekarz->id_lekarz != 1 && czy_dodatkowe_badania < 10){
                        
                badania_ambulatoryjne(&msg, lekarz);
                fflush(stdout);
                --lekarz->licznik_pacjentow;
                // zmniejszam licznik pacjentow, bo dzieki temu pacjent, ktory
                // wroci z badan ambulatoryjnych nie bedzie liczony x2
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

void obsluga_SIGINT(int sig) {
    zakoncz_program = 1;

    if (POZ2) {  // Sprawdz, czy wątek zostal utworzony
        pthread_join(POZ2, NULL);
    }
    
    exit(0);
}
