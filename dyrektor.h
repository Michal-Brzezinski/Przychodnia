#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>

#include "MyLib/dekoratory.h"


void zakonczPraceLekarza(int pid_procesu);
void nakarzWyjscPacjentom();

int naSekundy(const char *time_str) {
    int hour, minute;
    if (sscanf(time_str, "%d:%d", &hour, &minute) != 2) {
        fprintf(stderr, "Blad: niepoprawny format czasu: %s\n", time_str);
        exit(1);
    }
    return hour * 3600 + minute * 60;   // bo w godzinie jest 3600 sek, a w minucie 60 sek
}

