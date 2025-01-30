#include "pacjent.h"

// Klucz i identyfikator semafora
key_t building_key;
int building_sem_id;

// Klucz i identyfikator kolejki komunikatow
key_t queue_key;
int queue_id;

void initialize_semaphores() {
    building_key = ftok(".", 'B');  // utworzenie klucza
    if (building_key == -1) {
        perror("Blad ftok");
        exit(1);
    }

    // utworzenie zbioru semaforow i pobranie identyfikator
    building_sem_id = semget(building_key, 1, IPC_CREAT | 0666);    
    if (building_sem_id == -1) {
        perror("Blad semget");
        exit(1);
    }

    if (semctl(building_sem_id, 0, SETVAL, BUILDING_CAPACITY) == -1) {  // inicjalizacja semafora
        perror("Blad semctl (SETVAL)");
        exit(1);
    }
}

void initialize_message_queue() {
    queue_key = ftok(".", 'Q');  // Utworzenie klucza kolejki
    if (queue_key == -1) {
        perror("Blad ftok (kolejka)");
        exit(1);
    }

    queue_id = msgget(queue_key, IPC_CREAT | 0666);  // Utworzenie kolejki
    if (queue_id == -1) {
        perror("Blad msgget");
        exit(1);
    }
}



// Proces pacjenta
void patient_process(Patient patient) {
    struct sembuf sem_op;
    Message msg;
    Confirmation conf;

    int patient_counter = 0;

    printf("Pacjent %d (VIP: %d, Wiek: %d) probuje wejsc do budynku.\n", patient.id, patient.is_vip, patient.age);

    // Proba wejscia do budynku
    sem_op.sem_num = 0;
    sem_op.sem_op = -1; // Opuszczenie semafora
    sem_op.sem_flg = 0; // Zablokuj jesli brak zasobow

    if(patient_counter <= BUILDING_CAPACITY){
        if (semop(building_sem_id, &sem_op, 1) == 0) {
            printf("Pacjent %d wszedl do budynku.\n", patient.id);

            // Wyslanie wiadomosci do kolejki
            msg.type = patient.is_vip ? 1 : 2; // VIP ma wyzszy priorytet
            msg.id = patient.id;
            msg.age = patient.age;
            msg.is_vip = patient.is_vip;

            if (msgsnd(queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
                perror(" Blad wysylania wiadomosci do kolejki\n");
                exit(1);
            }

            // Oczekiwanie na potwierdzenie rejestracji
            while (1) {
                if (msgrcv(queue_id, &conf, sizeof(Confirmation) - sizeof(long), patient.id, 0) > 0) {
                    printf("Pacjent %d zostal zarejestrowany.\n", patient.id);
                    break;
                } else if (errno != EINTR) {
                    perror(" Blad odbierania potwierdzenia rejestracji\n");
                    break;
                }
            }

            // Opuszczanie budynku
            printf("Pacjent %d opuszcza budynek.\n", patient.id);
            sem_op.sem_op = 1; // Zwalnianie miejsca w semaforze
            if (semop(building_sem_id, &sem_op, 1) == -1) {
                perror(" Blad semop (zwolnienie)\n");
            }
        }
        else perror("Bledna wartosc zwracana przez semop()\n");
    } 
    else {
        printf("Pacjent %d nie mogl wejsc do budynku - brak miejsca.\n", patient.id);
    }

    exit(0);
}

void generate_patients() {
    int i =0;
    while(1) {

        Patient patient;    // tworzy nowy obiekt struktury pacjenta
        patient.id = i + 1;
        i++;
        patient.is_vip = rand() % 2; // 50% szans na bycie VIP
        patient.age = rand() % 100 + 1; // Wiek od 1 do 100 lat

        pid_t pid = fork();
        if (pid == 0) {
            // Proces potomny (pacjent)
            patient_process(patient);
        } else if (pid < 0) {
            perror(" Blad fork\n");
            exit(1);
        }

        sleep(4); // Odstep miedzy pacjentami
    }

    // Oczekiwanie na zakonczenie wszystkich pacjentow
    while (wait(NULL) > 0);
}
