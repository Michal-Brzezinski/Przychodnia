#include "dyrektor.h"

void zakonczPraceLekarza(int pid_procesu){
     if (kill(pid_procesu, SIGUSR1) == -1) 
        perror_red("[Dyrektor]: Nie udalo sie wyslac sygnalu o zakonczeniu pracy do lekarza\n");
}

void nakarzWyjscPacjentom(){
     if (system("killall -s SIGUSR2 pacjent")== -1) 
        perror_red("[Dyrektor]: Nie udalo sie wyslac sygnalu o wyjsciu do pacjentow\n");
}

int main(int argc, char *argv[]){

    if (argc != 2)
    {
        perror_red("[Rejestracja]: Nieprawidlowa liczba argumentow");
        exit(1);
    }
    
    int pid_lekarza = atoi(argv[1]);

    printGreen("Otrzymany jako argument pid lekarza to: %d\n", pid_lekarza);

    return 0;
}