#include "funkcje.h"

int main(){

    printf("Komunikat 1");

    pid_t pid; /* Identyfikator procesu potomnego */
    int status; /* Stan wyjscia z wait() */
    pid_t wpid; /* Identyfikator procesu z wait() */


    switch(fork()){

        case -1:
            fprintf(stderr,"%s:Blad\n",strerror(errno));
            exit(1);
        case 0:
            printf("PID %ld: Proces potomny uruchomiony PPID: %ld
            \n", (long)getpid(), (long)getppid());
            execl("/bin/ps","ps",NULL);
            break;
        default:
            printf("PID %ld: Proces macierzysty, PID %ld. \n",\ 
            (long)getpid(), /* nasz PID */ \
            (long)pid); /* PID proc. macierzystego */
            wpid = wait(&status); /* Stan wyjscia z procesu potomnego */
        
        if(wpid == -1) {
            fprintf(stderr, "%s: wait()\n",strerror(errno));
            return(1);
        }
        
        else if (wpid != pid)
            abort(); /*To sie nigdy nie powinno zdarzyc w tym programie */
        
        else {
            printf("Proces potomny o ident. %ld zakonczyl
            dzialanie ze stanem 0x%04X\n",
            (long)pid, /* PID proc. potomnego */
            status); /* Stan wyjscia */
        }
    

    }

    return 0;
}