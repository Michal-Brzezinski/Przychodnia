#pragma once
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <stdio.h>

int alokujKolejkeKomunikatow(key_t klucz, int flagi);
int zwolnijKolejkeKomunikatow(key_t  klucz);
//int wyslijKomunikat(int msg_id, const void *wskaznik_msg, size_t rozmiar_msg, int flagi);
//ssize_t odbierzKomunikat(int msg_id, void *wskaznik_msg, size_t rozmiar_msg, long typ_msg,int flagi);