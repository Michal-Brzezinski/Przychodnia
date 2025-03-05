#include "dyrektor.h"

// ________________________________________________________________________________
// #define SLEEP // zakomentowac, jesli nie chcemy sleepow w generowaniu pacjentow  <--- DO TESTOWANIA
// W PLIKU UTILS.H -> DLA WSZYSTKICH PLIKOW 
// ________________________________________________________________________________


int sem_id;
//___________________________________________   FUNKCJE DO WYSLANIA SYGNALOW _______________________________

void zakonczPraceLekarza(int pid_procesu){
    if (kill(pid_procesu, 0) == -1) {  // Sprawdzenie, czy proces istnieje
        if (errno == ESRCH) {
            printYellow("[Dyrektor]: Proces lekarza o pid %d juz nie istnieje\n", pid_procesu);
        } else {
            perror_red("[Dyrektor]: Wystapil nieoczekiwany blad podczas sprawdzania statusu lekarza\n");
        }
        return;
    }

    // Jesli proces istnieje, wysylamy sygnal
    if (kill(pid_procesu, SIGUSR1) == -1) { 
        perror_red("[Dyrektor]: Nie udalo sie wyslac sygnalu o zakonczeniu pracy do lekarza\n");
    } else {
        printRed("[Dyrektor]: Wyslano sygnal do zakonczenia pracy przez lekarza o pid: %d\n", pid_procesu);
        printRed("Wartosc semafora to %d\n", valueSemafor(sem_id, 14));
    }
}

void nakarzWyjscPacjentom(){
    if (system("killall -s SIGUSR2 pacjent > /dev/null 2>&1")== -1) 
        perror_red("[Dyrektor]: Nie udalo sie wyslac sygnalu o wyjsciu do pacjentow\n");
    else
        printRed("[Dyrektor]: Wyslano sygnal do opuszczenia budynku przez wszystkich pacjentow\n");
}


//  _______________________________ GLOWNA FUNKCJA PROGRAMU _________________________

int main(int argc, char *argv[]){


    //  -----------------  KONWERTOWANIE ARGUMENTOW WYWOLANIA  --------------

    if (argc != 3)
    {
        perror_red("[Dyrektor]: Nieprawidlowa liczba argumentow");
        exit(1);
    }
    
    const char *stringTp = argv[1];
    const char *stringTk = argv[2];

    int Tp = naSekundy(stringTp);
    int Tk = naSekundy(stringTk);

    printf("Odczytane sekundy to: %d - Tp oraz %d - Tk\n", Tp, Tk);


    //  -------------------     INICJALIZACJA NIEZBEDNYCH IPC   ----------------------

    key_t sem_key = generuj_klucz_ftok(".", 'A');
    sem_id = alokujSemafor(sem_key, S, IPC_CREAT | 0600);


    //  ----------------------  ODCZYTANIE Z FIFO PIDU LOSOWEGO LEKARZA  ---------------------- 

    // Odczyt PID lekarza z potoku nazwanego
    int fifo_fd = open(FIFO_DYREKTOR, O_RDONLY);
    if(fifo_fd == -1) {
         perror_red("[Dyrektor]: Blad otwarcia potoku\n");
         exit(1);
    }
    char pid_string[20];
    if(read(fifo_fd, pid_string, sizeof(pid_string)) <= 0) {
         perror_red("[Dyrektor]: Blad odczytu z potoku\n");
         close(fifo_fd);
         exit(1);
    }
    close(fifo_fd);
    int pid_lekarza = atoi(pid_string);
    printRed("Otrzymany pid losowego lekarza to: %d\n", pid_lekarza);

    int czynna = 0;
    int zakonczWywolanego = 0; // flaga, czy juz wykonano zakonczPraceLekarza
    int wypros_pacjentow = 0;  // flaga informujaca o tym czym wysylano juz sygnal do pacjentow
    int los_wyzwalacz = 0;   // moment (w sekundach) w przedziale [Tp, Tk], w ktorym wywolamy funkcje


    // -------------------- GLOWNA PETLA DZIALANIA DYREKTORA ------------------------
    while(1){


        if(zwrocObecnyCzas() >= Tp && zwrocObecnyCzas() <= Tk){
            // Jesli budynek jest otwarty
            if(czynna == 0)
            {     
                signalSemafor(sem_id, 5);
                printGreen("[Dyrektor]: Otwarto budynek przychodni\n");
                czynna = 1;
                // Inicjalizuje ziarno liczb losowych i wybieram moment 
                // (moge uzyc losowy moment, ale to ma sens przy uruchomieniu programu z sleep)
                srand(time(NULL));
                los_wyzwalacz = Tp + 1;//(Tp+Tk)/2 ; //w połowie programu lub: + rand() % (Tk - Tp + 1);
            }

            // Jesli jeszcze nie wykonano wywolania i osiagnieto losowy moment
            if(!zakonczWywolanego && (zwrocObecnyCzas() >= los_wyzwalacz) && (valueSemafor(sem_id, 14) == 5)) {
                zakonczPraceLekarza(pid_lekarza);
                zakonczWywolanego = 1;
                printYellow("[Dyrektor]: Wyslano sygnal do Lekarza: %d\n", pid_lekarza);
            }

            if(zwrocObecnyCzas()> Tp + 2 && wypros_pacjentow == 0) 
            {
                // w losowy sposob wysylam takze drugi wygnał - w zależności o czasu
                nakarzWyjscPacjentom();
                wypros_pacjentow = 1;
            }

            continue;
        }

        if(czynna == 1){
            printYellow("[Dyrektor]: Przychodnia zostala zamknieta\n");
            break;
        }
        
    }

    return 0;
}