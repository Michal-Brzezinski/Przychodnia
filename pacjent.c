
#include "MyLib/msg_utils.h"
#include "MyLib/sem_utils.h"
#include "MyLib/dekoratory.h"

#include "pacjent.h"

int main(){

    // ----------- inicjalizacja wartości struktury pacjenta
    Pacjent pacjent;
    inicjalizujPacjenta(&pacjent);

    // _______________________________  DOSTANIE SIĘ DO BUDYNKU     _______________________________________


    key_t klucz_wejscia = generuj_klucz_ftok(".",'A');

    int semID = alokujSemafor(klucz_wejscia, S, IPC_CREAT | 0600);

    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d próbuje wejść do budynku\033[0m\n", pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    waitSemafor(semID, 0, 0);   /* SPRAWDZA CZY MOŻE WEJŚĆ DO BUDYNKU BAZUJĄC NA SEMAFORZE*/
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wszedł do budynku\033[0m\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    sleep(1); // opóźnienie 5 sekund w budynku


    /*  ________________________________    KOMUNIKACJA Z REJESTRACJĄ   __________________________________________*/

    key_t msg_key = generuj_klucz_ftok(".",'B');
    int msg_id = alokujKolejkeKomunikatow(msg_key,IPC_CREAT | 0600);

    Wiadomosc msg;
    inicjalizujWiadomosc(&msg, &pacjent);

    // Pacjent oczekuje na rejestrację
    printf("\033[1;34m[Pacjent]: Pacjent %d czeka na rejestrację w kolejce.\033[0m\n", msg.id_pacjent);
    // inna obsługa czekania w kolejce - zgodna z treścią zadania
    printf("[Pacjent]: Wartość semafora: %d\n", semctl(semID, 0, GETVAL));
        // Wyślij wiadomość do rejestracji
    if (msgsnd(msg_id, &msg, sizeof(Wiadomosc) - sizeof(long), 0) == -1) {
        perror("\033[1;31mBłąd msgsnd - pacjent\033[0m\n");
        exit(1);
    }

    waitSemafor(semID, 1, 0);   // czeka aż przyjdzie komunikat

    /*  =======================================================================================   */


    // tutaj zrobić obsługę dla zamkniętej rejestracji
    if(valueSemafor(semID, 2)==1)signalSemafor(semID, 0);    // zwolnienie semafora wejścia do budynku
    printf("\033[1;34m[Pacjent]: Pacjent nr %d, wiek: %d, vip:%d wyszedł z budynku\033[0m\n",pacjent.id_pacjent, pacjent.wiek, pacjent.vip);
    fflush(stdout);

    return 0;
}
