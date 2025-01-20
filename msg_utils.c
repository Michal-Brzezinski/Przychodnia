#include "msg_utils.h"

int alokujKolejkeKomunikatow(key_t klucz, int flagi){

   int msgID;
   if ( (msgID = msgget(klucz, flagi)) == -1)
   {
      perror("Blad msgget (alokujKolejkeKomunikatow): ");
      exit(1);
   }

    return msgID;
}

/*
int wyslijKomunikat(int msg_id, const void *wskaznik_msg, size_t rozmiar_msg, int flagi){

    if (msgsnd(msg_id, &wskaznik_msg, rozmiar_msg, 0) == -1) {
        perror("Błąd msgsnd - wyslijKomunikat");
        exit(1);
    }
    return 0;
}

ssize_t odbierzKomunikat(int msg_id, void *wskaznik_msg, size_t rozmiar_msg, long typ_msg,int flagi){

    ssize_t odbior = msgrcv(msg_id, wskaznik_msg, rozmiar_msg, typ_msg, flagi);
        
    if (odbior == -1) {
        perror("Błąd msgrcv - odbierzKomunikat");
        exit(1);
    }

    return odbior;
}*/