
#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"
#include "MyLib/dekoratory.h"

#include "pacjent.h"


sem_t opiekun_semafor; // Semafor do synchronizacji wątków
pthread_t id_dziecko;
volatile int zakoncz_program = 0; // Flaga do zakończenia programu

void *dziecko(void* _wat);
void obsluga_SIGINT(int sig);

int main(){

    // Obsługa sygnału SIGINT
    struct sigaction sa;
    sa.sa_handler = obsluga_SIGINT;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // Inicjalizacja semafora wątków
    sem_init(&opiekun_semafor, 0, 0);

    // ----------- inicjalizacja wartości struktury pacjenta
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
            perror("\033[1;31m[Pacjent]: Błąd pthread_create - tworzenie dziecka\033[0m\n");
            exit(1);
        }
    }
    
    // _______________________________  DOSTANIE SIĘ DO BUDYNKU     _______________________________________
    
    if(pacjent.wiek >= 18){
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku\033[0m\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);}
    else{
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku z opiekunem\033[0m\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);
    }

    while (valueSemafor(semID, 2) == 0);   // czeka na otwarcie rejestracji (zapewnia, że nikogo nie wpuszczamy do budynku przed otwarciem rejestracji)    

    waitSemafor(semID, 0, 0);   /* SPRAWDZA CZY MOŻE WEJŚĆ DO BUDYNKU BAZUJĄC NA SEMAFORZE*/
    
    if(pacjent.wiek >= 18){
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku\033[0m\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);
    }
    else{
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku pod opieką\033[0m\n",msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);}

    //sleep(1); // opóźnienie 5 sekund w budynku

    //  ________________________________    KOMUNIKACJA Z REJESTRACJĄ   __________________________________________


        // Pacjent oczekuje na rejestrację
    if(pacjent.wiek >= 18){
    printf("\033[1;34m[Pacjent]: Pacjent %d czeka na rejestrację w kolejce do lekarza: %d.\033[0m\n", msg.id_pacjent, msg.id_lekarz);
    fflush(stdout);
    }
    else{
    printf("\033[1;34m[Pacjent]: Pacjent %d czeka z opiekunem na rejestrację w kolejce do lekarza: %d.\033[0m\n", msg.id_pacjent, msg.id_lekarz);
    fflush(stdout);}

        // Wyślij wiadomość do rejestracji
    if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("\033[1;31m[Pacjent]: Błąd msgsnd - pacjent\033[0m\n");
        exit(1);
    }

    waitSemafor(semID, 1, 0);   // czeka aż przyjdzie komunikat


    if(valueSemafor(semID, 2)==1)   signalSemafor(semID, 0);    // zwolnienie semafora wejścia do budynku
    if(pacjent.wiek >= 18){
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku\033[0m\n",msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);}
    else{
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku wraz z opiekunem\033[0m\n",msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);
    }

    if (pacjent.wiek < 18) {
        // Sygnalizuj dziecku zakończenie
        sem_post(&opiekun_semafor);
        zakoncz_program = 1;
        // Oczekiwanie na zakończenie pracy wątku dziecka
        
        int przylacz_dziecko=pthread_join(id_dziecko,NULL);
        if (przylacz_dziecko==-1) {
            perror("\033[1;31m[Pacjent]: Błąd pthread_join - przylaczenie dziecka\033[0m\n");
            exit(1);
        }
    }

    // Zniszczenie semafora
    sem_destroy(&opiekun_semafor);

    return 0;
}

void obsluga_SIGINT(int sig) {

    // Ustawienie flagi zakończenia
    zakoncz_program = 1;

    // Odblokuj dziecko, jeśli czeka na semaforze
    sem_post(&opiekun_semafor);

    // Anulowanie i oczekiwanie na zakończenie wątku dziecka
    pthread_cancel(id_dziecko);
    pthread_join(id_dziecko, NULL);

    // Zniszczenie semafora
    sem_destroy(&opiekun_semafor);

    exit(0);
}


void *dziecko(void* _wat){

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    // pozwala na anulowanie wątku w dowolnym momencie (asynchronicznie)

    int *pid = (int *)_wat;
    printf("\033[1;36m[Dziecko]: Wątek dziecka nr %d się bawi.\033[0m\n", *pid);
    
    while (!zakoncz_program) {
        // Czekaj na sygnał od wątku głównego
        sem_wait(&opiekun_semafor);

        if (zakoncz_program) {
            break; // Wyjście z pętli, jeśli program ma się zakończyć
        }   
    }

    printf("\033[1;36m[Dziecko]: Wątek dziecka nr %d przestał się bawić.\033[0m\n", *pid);
    return NULL;

}

