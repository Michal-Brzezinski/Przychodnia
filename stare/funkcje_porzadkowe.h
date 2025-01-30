#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>

// Deklaracje zmiennych globalnych (musze miec do nich dostep w funkcji obslugi sygnalow)
extern int building_sem_id;
extern int queue_id;   //volatile uzywany do zmiennej, ktora zmienia sie przez uzycie sygnalu np.
                                        //mowi kompilatorowi zeby niczego nie optymalizowal z ta zmienna

extern volatile int keep_generating;  // Domyslnie generowanie pacjentow jest wlaczone

void cleanup_semaphores();
void cleanup_message_queue();
void signal_handler(int sig);

