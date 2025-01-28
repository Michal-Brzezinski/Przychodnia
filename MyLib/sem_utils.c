#include "sem_utils.h"

/* _____________________________    SEMAFORY    __________________________________ */

int alokujSemafor(key_t klucz, int number, int flagi)
{
   int semID;
   if ( (semID = semget(klucz, number, flagi)) == -1)
   {
      perror("Blad semget (alokujSemafor): ");
      exit(1);
   }
   return semID;
}

void zwolnijSemafor(key_t key)
{
   int semid;

   // Próba uzyskania identyfikatora zbioru semaforów
   semid = semget(key, 0, 0666);
   if (semid == -1) {
      if (errno == ENOENT) {
            return;
      } else {
            perror("Błąd 1 usuwania zbioru semaforów");
            exit(1);
      }
   }

   // Zbiór semaforów istnieje, więc próbujemy go usunąć
   if (semctl(semid, 0, IPC_RMID) == -1) {
      perror("Błąd 2 usuwania zbioru semaforów");
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
      
      perror("Blad semctl (inicjalizujSemafor): ");
      exit(1);
   }
}

int waitSemafor(int semID, int number, int flags)
{
   //int result;
   struct sembuf operacje[1];
   operacje[0].sem_num = number;
   operacje[0].sem_op = -1;
   operacje[0].sem_flg = 0 | flags;//SEM_UNDO;
   
   if ( semop(semID, operacje, 1) == -1 )
   {
      if(errno == EINTR)
         return 0;

      perror("Blad semop (waitSemafor)");
      return -1;
   }
   
   return 1;
}

void signalSemafor(int semID, int number)
{
   struct sembuf operacje[1];
   operacje[0].sem_num = number;
   operacje[0].sem_op = 1;
   //operacje[0].sem_flg = SEM_UNDO;

   if (semop(semID, operacje, 1) == -1 )
      
      if(errno != EINTR)
      perror("Blad semop (postSemafor): ");

   return;
}

int valueSemafor(int semID, int number)
{
   return semctl(semID, number, GETVAL, NULL);
}




