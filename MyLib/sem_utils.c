#include "sem_utils.h"

/* _____________________________    SEMAFORY    __________________________________ */

int alokujSemafor(key_t klucz, int number, int flagi)
{
   int semID;
   if ( (semID = semget(klucz, number, flagi)) == -1)
   {
      perror_red("Blad semget (alokujSemafor): ");
      exit(1);
   }
   return semID;
}

void zwolnijSemafor(key_t key)
{
   int semid;

   // Proba uzyskania identyfikatora zbioru semaforow
   semid = semget(key, 0, 0666);
   if (semid == -1) {
      if (errno == ENOENT) {
            return;
      } else {
            perror_red("Blad 1 usuwania zbioru semaforow");
            exit(1);
      }
   }

   // Zbior semaforow istnieje, wiec probujemy go usunac
   if (semctl(semid, 0, IPC_RMID) == -1) {
      perror_red("Blad 2 usuwania zbioru semaforow");
      exit(1);
   }

   return;
}

void inicjalizujSemafor(int semID, int number, int val)
{
   
   if ( semctl(semID, number, SETVAL, val) == -1 )
   {
      if(errno == EINTR)
         exit(0);
      
      perror_red("Blad semctl (inicjalizujSemafor): ");
      exit(1);
   }
}

int waitSemafor(int semID, int number, int flags)
{
   struct sembuf operacje[1];
   operacje[0].sem_num = number;
   operacje[0].sem_op = -1;
   operacje[0].sem_flg = 0 | flags;
   
   if ( semop(semID, operacje, 1) == -1 )
   {
      if(errno == EINTR)
         waitSemafor(semID, number, flags);
      else{
      perror_red("Blad semop (waitSemafor)");
      exit(1);
      }
   }
   
   return 1;
}

void signalSemafor(int semID, int number)
{
   struct sembuf operacje[1];
   operacje[0].sem_num = number;
   operacje[0].sem_op = 1;

   if (semop(semID, operacje, 1) == -1 )
      
      if(errno != EINTR)
      perror_red("Blad semop (postSemafor)");

   return;
}

int valueSemafor(int semID, int number)
{
   return semctl(semID, number, GETVAL, NULL);
}




