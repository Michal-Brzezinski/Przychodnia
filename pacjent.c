
#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"
#include "MyLib/dekoratory.h"

#include "pacjent.h"

typedef struct _Wat {
    int msg_id;
    int semID;
	Wiadomosc *msg;
	} Watek;

void *opiekun(void* _wat);

int main(){

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

        pthread_t id_opiekun;
        Watek watek = {msg_id,semID,&msg};

        int utworz_opiekun=pthread_create(&id_opiekun,NULL,opiekun,&watek);
        if (utworz_opiekun==-1) {
            perror("\033[1;31m[Pacjent]: Błąd pthread_create - tworzenie opiekuna\033[0m\n");
            exit(1);
        }
        int przylacz_opiekuna=pthread_join(id_opiekun,NULL);
        if (przylacz_opiekuna==-1) {
            perror("\033[1;31m[Pacjent]: Błąd pthread_join - przylaczenie opiekuna\033[0m\n");
            exit(1);
        }
        
        int odlacz_opiekuna=pthread_detach(id_opiekun);
        if (odlacz_opiekuna==-1) {
            perror("\033[1;31m[Pacjent]: Błąd pthread_detach - odlaczenie opiekuna\033[0m\n");
            exit(1);
        }
    }
    
    // _______________________________  DOSTANIE SIĘ DO BUDYNKU     _______________________________________
    else{

    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku\033[0m\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    waitSemafor(semID, 0, 0);   /* SPRAWDZA CZY MOŻE WEJŚĆ DO BUDYNKU BAZUJĄC NA SEMAFORZE*/
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku\033[0m\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    //sleep(1); // opóźnienie 5 sekund w budynku

    //  ________________________________    KOMUNIKACJA Z REJESTRACJĄ   __________________________________________


        // Pacjent oczekuje na rejestrację
    printf("\033[1;34m[Pacjent]: Pacjent %d czeka na rejestrację w kolejce.\033[0m\n", msg.id_pacjent);
    fflush(stdout);
        // Wyślij wiadomość do rejestracji
    if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("\033[1;31m[Pacjent]: Błąd msgsnd - pacjent\033[0m\n");
        exit(1);
    }

    waitSemafor(semID, 1, 0);   // czeka aż przyjdzie komunikat

    // tutaj zrobić obsługę dla zamkniętej rejestracji
    if(valueSemafor(semID, 2)==1)signalSemafor(semID, 0);    // zwolnienie semafora wejścia do budynku
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku\033[0m\n",msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);
    }

    return 0;
}





void *opiekun(void* _wat){

        Watek *watek = (Watek *)_wat;
        Wiadomosc msg = *(watek->msg);

    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku z opiekunem\033[0m\n", msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);

    waitSemafor(watek->semID, 0, 0);   /* SPRAWDZA CZY MOŻE WEJŚĆ DO BUDYNKU BAZUJĄC NA SEMAFORZE*/
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku pod opieką\033[0m\n",msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);

        // Pacjent oczekuje na rejestrację
    printf("\033[1;34m[Pacjent]: Pacjent %d czeka z opiekunem na rejestrację w kolejce.\033[0m\n", msg.id_pacjent);
    fflush(stdout);
        // Wyślij wiadomość do rejestracji
    if (msgsnd(watek->msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("\033[1;31m[Pacjent]: Błąd msgsnd - pacjent(dziecko)\033[0m\n");
        exit(1);
    }

    waitSemafor(watek->semID, 1, 0);   // czeka aż przyjdzie komunikat

    // tutaj zrobić obsługę dla zamkniętej rejestracji
    if(valueSemafor(watek->semID, 2)==1)signalSemafor(watek->semID, 0);    // zwolnienie semafora wejścia do budynku
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku wraz z opiekunem\033[0m\n",msg.id_pacjent, msg.wiek, msg.vip);
    fflush(stdout);

}

