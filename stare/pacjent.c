#include "pacjent.h"

// Klucz i identyfikator semafora
key_t building_key;
int building_sem_id;

// Klucz i identyfikator kolejki komunikatów
key_t queue_key;
int queue_id;

void initialize_semaphores() {
    building_key = ftok(".", 'B');  // utworzenie klucza
    if (building_key == -1) {
        perror("Błąd ftok");
        exit(1);
    }

    // utworzenie zbioru semaforow i pobranie identyfikator
    building_sem_id = semget(building_key, 1, IPC_CREAT | 0666);    
    if (building_sem_id == -1) {
        perror("Błąd semget");
        exit(1);
    }

    if (semctl(building_sem_id, 0, SETVAL, BUILDING_CAPACITY) == -1) {  // inicjalizacja semafora
        perror("Błąd semctl (SETVAL)");
        exit(1);
    }
}

void initialize_message_queue() {
    queue_key = ftok(".", 'Q');  // Utworzenie klucza kolejki
    if (queue_key == -1) {
        perror("Błąd ftok (kolejka)");
        exit(1);
    }

    queue_id = msgget(queue_key, IPC_CREAT | 0666);  // Utworzenie kolejki
    if (queue_id == -1) {
        perror("Błąd msgget");
        exit(1);
    }
}



// Proces pacjenta
void patient_process(Patient patient) {
    struct sembuf sem_op;
    Message msg;
    Confirmation conf;

    int patient_counter = 0;

    printf("Pacjent %d (VIP: %d, Wiek: %d) próbuje wejść do budynku.\n", patient.id, patient.is_vip, patient.age);

    // Próba wejścia do budynku
    sem_op.sem_num = 0;
    sem_op.sem_op = -1; // Opuszczenie semafora
    sem_op.sem_flg = 0; // Zablokuj jeśli brak zasobów

    if(patient_counter <= BUILDING_CAPACITY){
        if (semop(building_sem_id, &sem_op, 1) == 0) {
            printf("Pacjent %d wszedł do budynku.\n", patient.id);

            // Wysłanie wiadomości do kolejki
            msg.type = patient.is_vip ? 1 : 2; // VIP ma wyższy priorytet
            msg.id = patient.id;
            msg.age = patient.age;
            msg.is_vip = patient.is_vip;

            if (msgsnd(queue_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
                perror(" Błąd wysyłania wiadomości do kolejki\n");
                exit(1);
            }

            // Oczekiwanie na potwierdzenie rejestracji
            while (1) {
                if (msgrcv(queue_id, &conf, sizeof(Confirmation) - sizeof(long), patient.id, 0) > 0) {
                    printf("Pacjent %d został zarejestrowany.\n", patient.id);
                    break;
                } else if (errno != EINTR) {
                    perror(" Błąd odbierania potwierdzenia rejestracji\n");
                    break;
                }
            }

            // Opuszczanie budynku
            printf("Pacjent %d opuszcza budynek.\n", patient.id);
            sem_op.sem_op = 1; // Zwalnianie miejsca w semaforze
            if (semop(building_sem_id, &sem_op, 1) == -1) {
                perror(" Błąd semop (zwolnienie)\n");
            }
        }
        else perror("Błędna wartość zwracana przez semop()\n");
    } 
    else {
        printf("Pacjent %d nie mógł wejść do budynku - brak miejsca.\n", patient.id);
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
            perror(" Błąd fork\n");
            exit(1);
        }

        sleep(4); // Odstęp między pacjentami
    }

    // Oczekiwanie na zakończenie wszystkich pacjentów
    while (wait(NULL) > 0);
}
