#include "dyrektor.h"

void zakonczPraceLekarza(int pid_procesu){
    if (kill(pid_procesu, SIGUSR1) == -1) 
        perror_red("[Dyrektor]: Nie udalo sie wyslac sygnalu o zakonczeniu pracy do lekarza\n");
    else
        printRed("[Dyrektor]: Wyslano sygnal do zakonczenia pracy przez lekarza o pid: %d\n", pid_procesu);
}

void nakarzWyjscPacjentom(){
    if (system("killall -s SIGUSR2 pacjent")== -1) 
        perror_red("[Dyrektor]: Nie udalo sie wyslac sygnalu o wyjsciu do pacjentow\n");
    else
        printRed("[Dyrektor]: Wyslano sygnal do opuszczenia budynku przez wszystkich pacjentow\n");
}

int main(int argc, char *argv[]){

    if (argc != 4)
    {
        perror_red("[Dyrektor]: Nieprawidlowa liczba argumentow");
        exit(1);
    }
    
    int pid_lekarza = atoi(argv[1]);
    printGreen("Otrzymany jako argument pid lekarza to: %d\n", pid_lekarza);
    
    const char *stringTp = argv[2];
    const char *stringTk = argv[3];

    int Tp = naSekundy(stringTp);
    int Tk = naSekundy(stringTk);

    printf("Odczytane sekundy to: %d - Tp oraz %d - Tk\n", Tp, Tk);

    time_t now;
    struct tm *local;
    int current_seconds;

    int czynna = 0;

    while(1){

        now = time(NULL);
        local = localtime(&now);
        current_seconds = local->tm_hour * 3600 + local->tm_min * 60 + local->tm_sec;

        if(current_seconds >= Tp && current_seconds <= Tk){
            if(czynna == 0)
            {            
                printGreen("[Dyrektor]: Uruchomiono przychodnie: %02d:%02d:%02d\n", local->tm_hour, local->tm_min, local->tm_sec);
                czynna = 1;
            }
            printGreen("[Dyrektor]: Wykonuje papierologie dyrektora\n");
            sleep(10);
            continue;
        }

        if(czynna == 1){
            printYellow("[Dyrektor]: Przychodnia zostala zamknieta\n");
            break;
        }
        else printCyan("[Dyrektor]: Przychodnia jest poza godzinami pracy\n");
        sleep(5);
    }

    return 0;
}