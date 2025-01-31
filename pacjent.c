
#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"
#include "MyLib/dekoratory.h"

#include "pacjent.h"


sem_t opiekun_semafor; // Semafor do synchronizacji watkow
pthread_t id_dziecko;
volatile int zakoncz_program = 0; // Flaga do zakonczenia programu

void *dziecko(void* _wat);
void obsluga_SIGINT(int sig);

int main(){

    // Obsluga sygnalu SIGINT
    struct sigaction sa;
    sa.sa_handler = obsluga_SIGINT;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // Inicjalizacja semafora watkow
    // Moge tutaj inicjalizowac semafor, bo watki sa w jednym procesie
    sem_init(&opiekun_semafor, 0, 0);

    // ----------- inicjalizacja wartosci struktury pacjenta
    Pacjent pacjent;
    inicjalizujPacjenta(&pacjent);

    Wiadomosc msg;
    inicjalizujWiadomosc(&msg, &pacjent);

    key_t klucz_wejscia = generuj_klucz_ftok(".",'A');
    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    key_t msg_key = generuj_klucz_ftok(".",'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key,IPC_CREAT | 0600);


    if(pacjent.wiek < 18){

        int utworz_dziecko=pthread_create(&id_dziecko,NULL,dziecko,(void*)&pacjent.id_pacjent);
        if (utworz_dziecko==-1) {
            perror_red("[Pacjent]: Blad pthread_create - tworzenie dziecka\n");
            exit(1);
        }
    }
    
    // _______________________________  DOSTANIE SIĘ DO BUDYNKU     _______________________________________
    
    if(pacjent.wiek >= 18)
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d probuje wejsc do budynku\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    else
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d probuje wejsc do budynku z opiekunem\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);


    while (valueSemafor(semID, 2) == 0);   // czeka na otwarcie rejestracji (zapewnia, ze nikogo nie wpuszczamy do budynku przed otwarciem rejestracji)    

    waitSemafor(semID, 0, 0);   /* SPRAWDZA CZY MOŻE WEJŚĆ DO BUDYNKU BAZUJĄC NA SEMAFORZE*/
    
    if(pacjent.wiek >= 18)
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedl do budynku\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    else
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedl do budynku pod opieka\n",msg.id_pacjent, msg.wiek, msg.vip);

    sleep(1); // opoznienie sekundy w budynku

    //  ________________________________    KOMUNIKACJA Z REJESTRACJĄ   __________________________________________


    // Pacjent oczekuje na rejestracje
    if(pacjent.wiek >= 18)
    printBlue("[Pacjent]: Pacjent %d czeka na rejestracje w kolejce do lekarza: %d.\n", msg.id_pacjent, msg.id_lekarz);
    else
    printBlue("\033[1;34m[Pacjent]: Pacjent %d czeka z opiekunem na rejestracje w kolejce do lekarza: %d.\n", msg.id_pacjent, msg.id_lekarz);


        // Wyslij wiadomosc do rejestracji
    if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror_red("[Pacjent]: Blad msgsnd - pacjent\n");
        exit(1);
    }

    waitSemafor(semID, 1, 0);   // czeka az przyjdzie komunikat
    if(valueSemafor(semID, 2)==0) signalSemafor(semID, 1);  
    // oznajmia innym, ktorzy nie zdazyli przed zamknieciem, ze mozna wyjsc
    // jezeli valueSemafor(semID, 2)==0 to znaczy ze rejestracja juz zamknieta

    if(valueSemafor(semID, 2)==1)   signalSemafor(semID, 0);    // zwolnienie semafora wejscia do budynku
    if(pacjent.wiek >= 18)
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedl z budynku\n",msg.id_pacjent, msg.wiek, msg.vip);
    else
    printBlue("[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedl z budynku wraz z opiekunem\n",msg.id_pacjent, msg.wiek, msg.vip);


    if (pacjent.wiek < 18) {
        // Sygnalizuj dziecku zakonczenie
        sem_post(&opiekun_semafor);
        zakoncz_program = 1;
        // Oczekiwanie na zakonczenie pracy watku dziecka
        
        int przylacz_dziecko=pthread_join(id_dziecko,NULL);
        if (przylacz_dziecko==-1) {
            perror_red("[Pacjent]: Blad pthread_join - przylaczenie dziecka\n");
            exit(1);
        }
    }

    // Zniszczenie semafora
    sem_destroy(&opiekun_semafor);

    return 0;
}

void obsluga_SIGINT(int sig) {

    // Ustawienie flagi zakonczenia
    zakoncz_program = 1;

    // Odblokuj dziecko, jesli czeka na semaforze
    sem_post(&opiekun_semafor);

    // Anulowanie i oczekiwanie na zakonczenie watku dziecka
    pthread_cancel(id_dziecko);
    pthread_join(id_dziecko, NULL);

    // Zniszczenie semafora
    sem_destroy(&opiekun_semafor);

    exit(0);
}


void *dziecko(void* _wat){

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    // pozwala na anulowanie watku w dowolnym momencie (asynchronicznie)

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

