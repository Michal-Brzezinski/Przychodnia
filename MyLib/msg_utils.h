#pragma once
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "dekoratory.h"

int alokujKolejkeKomunikatow(key_t klucz, int flagi);
int zwolnijKolejkeKomunikatow(key_t  klucz);
int policzProcesy(int msg_id);