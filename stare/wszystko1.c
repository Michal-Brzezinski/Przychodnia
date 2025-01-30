#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <sys/msg.h>
#include <signal.h>

#define MAX_PATIENTS 100 // Maksymalna liczba pacjentow w przychodni
#define BUILDING_CAPACITY 10 // Maksymalna liczba pacjentow w przychodni
#define MAX_ADMISSION 3 // Ilosc pacjentow przyjmowanych do rejestracji

//volatile int keep_generating = 1;   // Zmienna do kontrolowania generowania pacjentow

// Klucz i identyfikator semafora
key_t building_key;
int building_sem_id;

// Klucz i identyfikator kolejki komunikatow
key_t queue_key;
int queue_id;

// Struktura wiadomosci w kolejce
typedef struct {
    long type;    // Typ wiadomosci (priorytet: 1 dla VIP, 2 dla zwyklych pacjentow)
    int id;       // ID pacjenta
    int age;      // Wiek pacjenta
    int is_vip;   // 1 jesli VIP, 0 jesli nie
} Message;

// Struktura reprezentujaca pacjenta
typedef struct {
    int id;
    int is_vip; // 1 jesli VIP, 0 jesli nie
    int age;    // Wiek pacjenta
} Patient;

// Wiadomosc potwierdzajaca dla pacjenta, czy zostal przyjety
typedef struct {
    long type;  // ID pacjenta (odpowiadajacy `patient.id`)
    int id;     // Identyfikator pacjenta
} Confirmation;

void initialize_semaphores() {
    building_key = ftok(".", 'B');  
    if (building_key == -1) {
        perror("Blad ftok");
        exit(1);
    }

    building_sem_id = semget(building_key, 1, IPC_CREAT | 0666);
    if (building_sem_id == -1) {
        perror("Blad semget");
        exit(1);
    }

    if (semctl(building_sem_id, 0, SETVAL, BUILDING_CAPACITY) == -1) {
        perror("Blad semctl (SETVAL)");
        exit(1);
    }
}

void initialize_message_queue() {
    queue_key = ftok(".", 'Q');  
    if (queue_key == -1) {
        perror("Blad ftok (kolejka)");
        exit(1);
    }

    queue_id = msgget(queue_key, IPC_CREAT | 0666);  
    if (queue_id == -1) {
        perror("Blad msgget");
        exit(1);
    }
}

void cleanup_message_queue() {
    if (queue_id != -1) {
        if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
            perror("Blad msgctl (usuwanie kolejki)");
        }
    }
}

void cleanup_semaphores() {
    
    if (building_sem_id != -1) {
        if (semctl(building_sem_id, 0, IPC_RMID) == -1) {
            perror("Blad semctl (IPC_RMID)");
        }
    }
}

void signal_handler(int sig) {
    //keep_generating = 0;
    //while (wait(NULL) > 0);  
    cleanup_semaphores();
    cleanup_message_queue();
    exit(0);  
}

void patient_process(Patient patient) {
    struct sembuf sem_op;
    Message msg;
    Confirmation conf;

    printf("Pacjent %d (VIP: %d, Wiek: %d) probuje wejsc do budynku.\n", patient.id, patient.is_vip, patient.age);

    sem_op.sem_num = 0;
    sem_op.sem_op = -1; // Opuszczenie semafora
    sem_op.sem_flg = 0; // Zablokuj jesli brak zasobow

    if (semop(building_sem_id, &sem_op, 1) == 0) {
        printf("Pacjent %d wszedl do budynku.\n", patient.id);

        msg.type = patient.is_vip ? 1 : 2; 
        msg.id = patient.id;
        msg.age = patient.age;
        msg.is_vip = patient.is_vip;

        if (msgsnd(queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
            perror("Blad wysylania wiadomosci do kolejki");
            exit(1);
        }

        while (1) {
            if (msgrcv(queue_id, &conf, sizeof(Confirmation) - sizeof(long), patient.id, 0) > 0) {
                printf("Pacjent %d zostal zarejestrowany.\n", patient.id);
                break;
            } else if (errno == EINTR) {
                continue; 
            } else {
                perror("Blad odbierania potwierdzenia rejestracji");
                break;
            }
        }

        printf("Pacjent %d opuszcza budynek.\n", patient.id);
        sem_op.sem_op = 1; // Zwalnianie miejsca w semaforze
        if (semop(building_sem_id, &sem_op, 1) == -1) {
            perror("Blad semop (zwolnienie)");
        }
    } else {
        perror("Bledna wartosc zwracana przez semop()");
    }

    exit(0);
}

void generate_patients() {
    int i = 0;
    while (1) {
        Patient patient;    
        patient.id = i + 1;
        i++;
        patient.is_vip = rand() % 2; 
        patient.age = rand() % 100 + 1; 

        pid_t pid = fork();
        if (pid == 0) {
            patient_process(patient);
        } else if (pid < 0) {
            perror("Blad fork");
            exit(1);
        }

        sleep(4); // Odstep miedzy pacjentami
    }

    while (wait(NULL) > 0);
}

void registration_process() {
    Message msg;
    Confirmation conf;
    int patients_admissioned = 0;

    printf("\nRejestracja: Oczekiwanie na pacjentow...\n\n");

    while (patients_admissioned < MAX_ADMISSION) {
        if (msgrcv(queue_id, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            printf("Rejestracja: Pacjent %d (VIP: %d, Wiek: %d) zglosil sie.\n", msg.id, msg.is_vip, msg.age);

            conf.type = msg.id;
            conf.id = msg.id;

            sleep(6); // Symulacja czasu rejestracji

            if (msgsnd(queue_id, &conf, sizeof(Confirmation) - sizeof(long), 0) == -1) {
                perror("Blad wysylania potwierdzenia");
            }
            patients_admissioned++;
        } else if (errno != EINTR) {
            perror("Blad odbierania wiadomosci");
        }
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Blad przy rejestracji obslugi sygnalu SIGINT");
        exit(1);
    }

    initialize_semaphores();
    initialize_message_queue();

    pid_t pid = fork();
    if (pid == 0) {
        registration_process();
    } else if (pid > 0) {
        generate_patients();
        while (1) {
            sleep(1);  
        }
    } else {
        perror("Blad fork");
        exit(1);
    }

    //while (wait(NULL) > 0);

    return 0;
}