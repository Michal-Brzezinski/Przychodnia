#include "pacjent.h"

// ___________________________________________________________________
// #define SLEEP // zakomentowac, jesli nie chcemy sleepow w programie  <-- DO TESTOW
// ___________________________________________________________________


sem_t opiekun_semafor; // Semafor do synchronizacji watkow
pthread_t id_dziecko;
volatile int zakoncz_program = 0; // Flaga do zakonczenia programu

void *dziecko(void* _wat);
void obsluga_SIGINT(int sig);
void obsluga_USR2(int sig);

int main(){
    

    signal(SIGTERM, handlerSIGTERM);
    // Obsluga sygnalu SIGINT
    struct sigaction sa;
    sa.sa_handler = obsluga_SIGINT;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror_red("[Pacjent]: sigaction\n");
        exit(1);
    }

    // Obsluga sygnalu SIGUSR2
    struct sigaction usr2;
    usr2.sa_handler = obsluga_USR2;
    usr2.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&usr2.sa_mask);
    if(sigaction(SIGUSR2, &usr2, NULL) == -1)
    {
        perror_red("[Pacjent]: sigaction\n");
        exit(1);
    }

    // Inicjalizacja semafora watkow
    // Moge tutaj inicjalizowac semafor, bo watki sa w jednym procesie
    sem_init(&opiekun_semafor, 0, 0);

    // ----------- inicjalizacja wartosci struktury pacjenta -------------
    inicjalizujPacjenta(&pacjent);

    Wiadomosc msg;
    inicjalizujWiadomosc(&msg, &pacjent);

    key_t klucz_wejscia = generuj_klucz_ftok(".",'A');
    sem_id = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    key_t msg_key_rej = generuj_klucz_ftok(".",'B');
    int msg_id_rej = alokujKolejkeKomunikatow(msg_key_rej,IPC_CREAT | 0600);

    key_t klucz_wyjscia = generuj_klucz_ftok(".", 'W');
    int msg_id_wyjscie = alokujKolejkeKomunikatow(klucz_wyjscia, IPC_CREAT | 0600);


    if(pacjent.wiek < 18){

        int utworz_dziecko=pthread_create(&id_dziecko,NULL,dziecko,(void*)&pacjent.id_pacjent);
        if (utworz_dziecko==-1) {
            perror_red("[Pacjent]: Blad pthread_create - tworzenie dziecka\n");
            exit(1);
        }
    }
    
    // _______________________________  DOSTANIE SIE DO BUDYNKU     _______________________________________
    

        //______________________________________________________________
        waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
        
        FILE *raport = fopen("raport", "a");
        if (raport == NULL) {
            perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
        } else {

            fprintf(raport, "PACJENT %d ROZPOCZAL SWOJE DZIALANIE\n", pacjent.id_pacjent);
            fflush(raport);
            fclose(raport);
        }
        signalSemafor(sem_id, 4); 
        //______________________________________________________________ 

    if(pacjent.wiek >= 18)
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d probuje wejsc do budynku\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    else
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d probuje wejsc do budynku z opiekunem\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip); 

    waitSemafor(sem_id, 0, 0);   /* SPRAWDZA CZY MOZE WEJSC DO BUDYNKU BAZUJAC NA SEMAFORZE*/

    waitSemafor(sem_id, 6, 0);
    if((valueSemafor(sem_id, 5) == 1)){
        if( sygnal2 == 0){
            if(pacjent.wiek >= 18)
            printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedl do budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
            else
            printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedl do budynku pod opieka\n",msg.id_pacjent, msg.wiek, msg.vip);
            pacjent.czy_wszedl = 1; // ustawiam paramter pacjenta, ktory mowi, ze pacjent juz wszedl do budynku, wiec musi wyjsc w przyszlosci
        
        
                //______________________________________________________________
                waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
        
                raport = fopen("raport", "a");
                if (raport == NULL) {
                    perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
                } else {
        
                    fprintf(raport, "PACJENT %d WSZEDL DO BUDYNKU\n", pacjent.id_pacjent);
                    fflush(raport);
                    fclose(raport);
                }
                signalSemafor(sem_id, 4); 
                //______________________________________________________________ 
        
        }
        else signalSemafor(sem_id, 0);
        // Jezeli sygnal2 != 0, zwalniamy semafor wejscia, by nie blokowac dalszych operacji
        
    }
    else signalSemafor(sem_id, 0); // jezeli nie udalo sie wejsc to zwolnij semafor dla innych na przyszlosc

    signalSemafor(sem_id, 6);
    
    #ifdef SLEEP
    sleep(3); // opoznienie sekundy w budynku
    #endif
    //  ________________________________    KOMUNIKACJA Z REJESTRACJA   __________________________________________


    // Pacjent oczekuje na rejestracje
    waitSemafor(sem_id, 6, 0);
    if((valueSemafor(sem_id, 2) == 1) && (valueSemafor(sem_id, 5) == 1 && sygnal2 == 0)){
        signalSemafor(sem_id, 6);

        if(sygnal2 == 0){
            if(pacjent.wiek >= 18)
            printBlue("[Pacjent]: Pacjent %d czeka w kolejce na rejestracje do lekarza: %d.\n", msg.id_pacjent, msg.id_lekarz);
            else
            printBlue("[Pacjent]: Pacjent %d czeka z opiekunem w kolejce na rejestracje do lekarza: %d.\n", msg.id_pacjent, msg.id_lekarz);
                
            // Wyslij wiadomosc do rejestracji
        
            if (msgsnd(msg_id_rej, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
                perror_red("[Pacjent]: Blad msgsnd - pacjent\n");
                exit(1);
            }
        
        //______________________________________________________________
        waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
        
        raport = fopen("raport", "a");
        if (raport == NULL) {
            perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
        } else {

            fprintf(raport, "PACJENT %d WYSLAL WIADOMOSC DO REJESTRACJI\n", pacjent.id_pacjent);
            fflush(raport);
            fclose(raport);
        }
        signalSemafor(sem_id, 4); 
        //______________________________________________________________ 

            if (msgrcv(msg_id_wyjscie, &msg, sizeof(Wiadomosc) - sizeof(long), msg.id_pacjent, 0) == -1)
            {
                // Pacjent czeka na potwierdzenie, ze moze wyjsc
                perror_red("[Pacjent]: Blad msgrcv\n");
                exit(1);
            }

        }

                //______________________________________________________________
                waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
        
                raport = fopen("raport", "a");
                if (raport == NULL) {
                    perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
                } else {
        
                    fprintf(raport, "PACJENT %d ODEBRAL WIADOMOSC O WYJSCIU\n", pacjent.id_pacjent);
                    fflush(raport);
                    fclose(raport);
                }
                signalSemafor(sem_id, 4); 
                //______________________________________________________________ 
    } 
    else signalSemafor(sem_id, 6);
    
    if(pacjent.czy_wszedl == 1){
        // jezeli pacjent wszedl do budynku to musi tez z niego wyjsc
        signalSemafor(sem_id, 0);
        if(pacjent.wiek >= 18)
        printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedl z budynku\n",msg.id_pacjent, msg.wiek, msg.vip);
        else
        printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedl z budynku wraz z opiekunem\n",msg.id_pacjent, msg.wiek, msg.vip);
    
            //______________________________________________________________
            waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
        
            raport = fopen("raport", "a");
            if (raport == NULL) {
                perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
            } else {
    
                fprintf(raport, "PACJENT %d WYSZEDL Z BUDYNKU\n", pacjent.id_pacjent);
                fflush(raport);
                fclose(raport);
            }
            signalSemafor(sem_id, 4); 
            //______________________________________________________________ 
    
    }

    if (pacjent.wiek < 18) {
        // Sygnalizuj dziecku zakonczenie
        zakoncz_program = 1;
        sem_post(&opiekun_semafor);
        sem_post(&opiekun_semafor);

        // Oczekiwanie na zakonczenie pracy watku dziecka
        
        int przylacz_dziecko=pthread_join(id_dziecko,NULL);
        if (przylacz_dziecko==-1) {
            perror_red("[Pacjent]: Blad pthread_join - przylaczenie dziecka\n");
            exit(1);
        }
    }

    // Zniszczenie semafora
    sem_destroy(&opiekun_semafor);
    
    if(pacjent.czy_wszedl == 0)
    printBlue("[Pacjent]: Pacjent nr %d nie dal rady wejsc do budynku i zakonczyl dzialanie\n", pacjent.id_pacjent);


        //______________________________________________________________
        waitSemafor(sem_id, 4, 0);  // blokada dostepu do pliku kontrolnego
        
        raport = fopen("raport", "a");
        if (raport == NULL) {
            perror_red("[Pacjent]: Blad otwarcia pliku raport\n");
        } else {

            fprintf(raport, "PACJENT %d ZAKONCZYL SWOJE DZIALANIE\n", pacjent.id_pacjent);
            fflush(raport);
            fclose(raport);
        }
        signalSemafor(sem_id, 4); 
        //______________________________________________________________ 

    return 0;
}

void *dziecko(void* _wat){


    int *pid = (int *)_wat;
    printCyan("[Dziecko]: Watek dziecka nr %d sie bawi.\n", *pid);
    
    while (!zakoncz_program) {
        // Czekaj na sygnal od watku glownego
        sem_wait(&opiekun_semafor);

        if (zakoncz_program) {
            break; // Wyjscie z petli, jesli program ma sie zakonczyc
        }   
    }

    printCyan("[Dziecko]: Watek dziecka nr %d przestal sie bawic.\n", *pid);
    return NULL;

}


void obsluga_SIGINT(int sig) {
    // Ustawienie flagi zakonczenia
    zakoncz_program = 1;

    // Odblokowanie dziecka, jeśli czeka na semaforze
    sem_post(&opiekun_semafor);
    sem_post(&opiekun_semafor);  // Dodatkowe, aby miec pewnosc, ze dziecko dostanie sygnal

    // Czekanie na zakonczenie watku dziecka (jesli istnieje)
    if (pacjent.wiek < 18) {
        pthread_join(id_dziecko, NULL);
    }

    // Zniszczenie semafora
    sem_destroy(&opiekun_semafor);

    // Bezpieczne zakonczenie procesu
    exit(0);
}

void obsluga_USR2(int sig){

    printRed("Pacjent %d otrzymal sygnal wyproszenia wszystkich pacjentow z budynku.\n", pacjent.id_pacjent);
    sygnal2 = 1;
}

void handlerSIGTERM(int signum){

    printRed("PACJENT %d OTRZYMAL SYGNAL SIGTERM I ZAKONCZYL SWOJE DZIALANIE\n", pacjent.id_pacjent);
}