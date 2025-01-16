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

// Deklaracje zmiennych globalnych (muszę mieć do nich dostęp w funkcji obsługi sygnałów)
extern int building_sem_id;
extern int queue_id;   //volatile używany do zmiennej, która zmienia się przez użycie sygnału np.
                                        //mówi kompilatorowi żeby niczego nie optymalizował z tą zmienną

extern volatile int keep_generating;  // Domyślnie generowanie pacjentów jest włączone

void cleanup_semaphores();
void cleanup_message_queue();
void cleanup();
void signal_handler(int sig);

