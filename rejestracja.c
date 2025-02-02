
#include "rejestracja.h"

volatile int running = 1; // Flaga do sygnalizowania zakonczenia

// Zmienna globalna do przechowywania pid procesu okienka nr 2
pid_t pid_okienka2 = -1;

int shm_id; // id  i klucz pamieci dzielonej
key_t shm_key;
int *pamiec_wspoldzielona;  // Wskaznik do pamieci wspoldzielonej

int msg_id_rej; // id i klucz kolejki do rejestracji
key_t msg_key_rej;

int sem_id; // id  i klucz do zbioru semaforow
key_t sem_key;

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

int licznik_pom;    // pomocniczna zmienna do przechowywania licznika danego lekarza
int kolejka_size;   // pomocniczna zmienna do przechowywania rozmiary kolejki
int msg_id_pom;     // pomocnicza zmienna do przechowywania id danej kolejki komunikatow

int limity_lekarzy[5] = {0}; // Tablica przechowujaca limity pacjentow dla lekarzy

void uruchomOkienkoNr2();

void zatrzymajOkienkoNr2();

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
    
    sem_key = generuj_klucz_ftok(".", 'A');
    sem_id = alokujSemafor(sem_key, S, IPC_CREAT | 0600);
    
    shm_key = generuj_klucz_ftok(".", 'X');
    shm_id = alokujPamiecWspoldzielona(shm_key, PAM_SIZE * sizeof(int), IPC_CREAT | 0666);
    
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
    
    printGreen("[Rejestracja]: Uruchomiono rejestracje\n");
    
    int limit_pacjentow = atoi(argv[1]);
    limity_lekarzy[0] = procentNaNaturalna(limit_pacjentow, 60); 
    // Limit pacjentow dla lekarza POZ
    
    int procent10 = procentNaNaturalna(limit_pacjentow, 10);
    // Limity pacjentow dla lekarzy specjalistow
    for (int i = 1; i < 5; i++)
    {
        limity_lekarzy[i] = procent10;
    }
    
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
    int Tp = current_time;      // Aktualny czas
    int Tk = current_time + 30; // Aktualny czas + x sekund
    
    pamiec_wspoldzielona = dolaczPamiecWspoldzielona(shm_id, 0);
    
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
        
        printGreen("[Rejestracja]: Otrzymano wiadomosc od pacjenta nr %d, ktory chce isc do lekarza nr %d\n", msg.id_pacjent, msg.id_lekarz);
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
            signalSemafor(sem_id, 1);
            signalSemafor(sem_id, 3);
            continue; 
        }
        
        if(limity_lekarzy[msg.id_lekarz-1]<=(licznik_pom+kolejka_size)){
            
            printYellow("[Rejestracja - 1 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
            signalSemafor(sem_id, 1);   // Informuj pacjenta, ze moze wyjsc z budynku
            signalSemafor(sem_id, 3);   // pozwol innym dzialac na pamieci dzielonej
            continue;
            // obsluga braku miejsc do danego lekarza
        }
        signalSemafor(sem_id, 3);   // pozwol innym dzialac na pamieci dzielonej
        
        printGreen("[Rejestracja - 1 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
        msg.mtype = msg.vip; // ustalenie priorytetu przyjecua pacjenta
        // Wyslij pacjenta do lekarza
        if (msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
            perror_red("[Rejestracja - 1 okienko]: Blad msgsnd - pacjent\n");
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
    zatrzymajOkienkoNr2(); // Zatrzymaj okienko nr 2 przed zakonczeniem pracy
    //
    printRed("[Rejestracja]: Zablokowano wejscie nowych pacjentow do budynku\n");
    inicjalizujSemafor(sem_id, 0, 0); // Zablokuj semafor wejscia do budynku
    waitSemafor(sem_id, 2, 0);        // Oznajmij zakonczenie rejestracji
    //signalSemafor(sem_id, 1); -> ten jest niepewny
    wypiszPacjentowWKolejce(msg_id_rej, sem_id);
    
    printYellow("[Rejestracja]: Czekam na sygnal zakonczenia generowania pacjentow...\n");
    
    // Wypisz pacjentow, ktorzy byli w kolejce do rejestracji w momencie zamykania rejestracji
    //wypiszPacjentowWKolejce(msg_id_rej, semID);
    printYellow("[Rejestracja]: Rejestracja zakonczyla dzialanie\n");
    
    signalSemafor(sem_id,1); // oznajmia wyjscie tym, ktorzy nie zdazyli przed zamknieciem rejestracji
    
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
        while (1)
        {
            // Czekaj na komunikat rejestracji
            Wiadomosc msg;
            if (msgrcv(msg_id_rej, &msg, sizeof(Wiadomosc) - sizeof(long), 0, IPC_NOWAIT) == -1)
            {
                // Sprawdzamy, czy nie bylo wiadomosci, ale nie blokujemy procesu dzieki IPC_NOWAIT
                if (errno != ENOMSG)
                {
                    perror_red("[Rejestracja - 1 okienko]: Blad msgrcv\n");
                    zatrzymajOkienkoNr2();
                    exit(1);
                }
                
                //printRed("[Rejestracja]: Brak pacjentow w kolejce\n");
                sleep(2); // Czekaj 2 sekundy i sprawdz ponownie
                continue;
            }
            
            printGreen("[Rejestracja]: Otrzymano wiadomosc od pacjenta nr %d, ktory chce isc do lekarza nr %d\n", msg.id_pacjent, msg.id_lekarz);
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
                signalSemafor(sem_id, 1);
                signalSemafor(sem_id, 3);
                continue; 
            }
            if(limity_lekarzy[msg.id_lekarz-1]<=(licznik_pom+kolejka_size)){
                
                printYellow("[Rejestracja - 2 okienko]: Pacjent nr %d nie moze wejsc do lekarza o id %d - brak miejsc\n",msg.id_pacjent ,msg.id_lekarz);
                signalSemafor(sem_id, 1);   // Informuj pacjenta, ze moze wyjsc z budynku
                signalSemafor(sem_id, 3);   // pozwol innym dzialac na pamieci dzielonej
                continue;
                // obsluga braku miejsc do danego lekarza
            }
            signalSemafor(sem_id, 3);   // pozwol innym dzialac na pamieci dzielonej
            
            printGreen("[Rejestracja - 2 okienko]: Zarejestrowano pacjenta nr %d do lekarza: %d\n", msg.id_pacjent, msg.id_lekarz);
            msg.mtype = msg.vip;     // ustalenie priorytetu przyjecia pacjenta
            // Wyslij pacjenta do lekarza
            if (msgsnd(msg_id_pom, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Rejestracja - 2 okienko]: Blad msgsnd\n");
                exit(1);
            }
            
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