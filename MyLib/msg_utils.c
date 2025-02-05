#include "msg_utils.h"

int alokujKolejkeKomunikatow(key_t klucz, int flagi){

   int msgID;
   if ( (msgID = msgget(klucz, flagi)) == -1)
   {
      perror_red("Blad msgget (alokujKolejkeKomunikatow): ");
      exit(1);
   }

    return msgID;
}


int zwolnijKolejkeKomunikatow(key_t klucz) {
    int msgid;

    // Proba uzyskania identyfikatora kolejki komunikatow
    msgid = msgget(klucz, 0666);
    if (msgid == -1) {
        if (errno == ENOENT) {
            return 0; // Kolejka nie istnieje
        } else {
            perror_red("msgget failed");
            exit(1);
        }
    }

    // Kolejka komunikatow istnieje, wiec probujemy ja usunac
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror_red("Blad msgctl (zwolnijKolejkeKomunikatow)");
        exit(1);
    }

    return 0;

}

// Funkcja pomocnicza do obliczania liczby procesow w kolejce
int policzProcesy(int msg_id) {
    /* Zlicz liczbe komunikatow w kolejce (w prostym przypadku nieopoznionym) */
    
    int liczba_procesow = 0;
    struct msqid_ds buf;
    if (msgctl(msg_id, IPC_STAT, &buf) == -1) {
        perror_red("[policzProcesy]: msgctl IPC_STAT\n");
        return -1;
    }
    liczba_procesow = buf.msg_qnum;
    return liczba_procesow;
}