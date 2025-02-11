#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>

#include "MyLib/utils.h"
#include "MyLib/sem_utils.h"

#define FIFO_DYREKTOR "fifo_dyrektor"   // nazwa kolejki fifo do przekazywania pidu lekarza dyrektorowi
#define S 7             // ilosc semaforow w zbiorze - w razie potrzeby zwiekszyc

void zakonczPraceLekarza(int pid_procesu);
void nakarzWyjscPacjentom();
int naSekundy(const char *time_str);

