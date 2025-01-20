#pragma once
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>

int losuj_int(int N);
key_t generuj_klucz_ftok(const char *sciezka, char projek_id);