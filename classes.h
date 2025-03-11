#ifndef CLASSES_H
#define CLASSES_H

#include <unistd.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <stdexcept>
#include <signal.h>
#include <wait.h>

// Tych definów nie ruszać:
#define VIP_QUEUE_SEM 5
#define LOADING_SEM 9
#define POMOST_SEM 3
#define POMOST_ZAL_SEM 11
#define POMOST_WYL_SEM 7


// Poniższe definy można edytować, byle były typu int:
// stałe z polecenia:
#define K 10
#define N1 30
#define N2 25
#define T1 1
#define T2 2
#define Tp 1
#define Tk 30


// Cennik biletów, N - normalny, U - ulgowy:
# define T1N 15
# define T1U 7
# define T2N 10
# define T2U 5

// liczba pasażerów nad jeziorem:
#define MAX_PASSENGERS 500


void error(const char* msg) {
    char errmsg[50];
    sprintf(errmsg, "\033[1;31m[ERROR]\033[0m: %s, PID: %d", msg, getpid());
    perror(errmsg);
    exit(EXIT_FAILURE);
}

union semun {
    int val;              // Wartość semafora (np. 0 lub 1 dla semafora binarnego).
    struct semid_ds *buf; // Struktura do pobierania lub ustawiania informacji o semaforze.
    unsigned short *array; // Tablica wartości semaforów (jeśli mamy zestaw semaforów).
    struct seminfo *__buf; // Struktura informacyjna, zawiera statystyki o semaforach.
};

class SharedMem {
    public:
    key_t key;
    int size;
    int id;
    SharedMem(key_t memory_key, int memory_size) {
        key = memory_key;
        size = memory_size;
    }

    void shm_create() {
        id = shmget(key, sizeof(int) * size, IPC_CREAT | 0666);
        if (id == -1) error("shmget error");
    }

    void shm_attach() {
        id = shmget(key, sizeof(int) * size, 0666);
        if (id == -1) error("shmget error");
    }

    int* shm_get() {
        return (int*)shmat(id, NULL, 0);
    }

    void shm_detach(int* shared_mem) {
        if (shmdt(shared_mem) == -1) error("shmdt error");
    }

    void shm_delete() {
        if (shmctl(id, IPC_RMID, NULL) == -1) error("shmctl error");
    }

};

class Sem {
    public:
    key_t key;
    int id;
    Sem(key_t sem_key) {
        key = sem_key;
    }

    void sem_create(int num_sems = 1) {
        id = semget(key, num_sems, IPC_CREAT | 0666);
        if (id == -1) {
            error("semget create error");
        }
    }

    void sem_attach() {
        id = semget(key, 0, 0666);
        if (id == -1) {
            error("semget attach error");
        }
    }

    void sem_op(short unsigned sem_num, short sem_op) {
        struct sembuf op = {sem_num, sem_op, 0};
        if (semop(id, &op, 1) == -1) {
            if (errno == EINTR) {
                semop(id, &op, 1);
            } else {
                printf("semnum: %d sem_get_value: %d\n", sem_num, sem_get_value(sem_num));
                error("semop error");
            }
            
        }
    }

    int sem_get_value(short sem_num) {
        int value = semctl(id, sem_num, GETVAL);
        if (value == -1) {
            printf("semctl GETVAL error semnum: %d\n", sem_num);
            error("semctl GETVAL error");
        }
        return value;
    }

    void sem_set_value(short sem_num, int value) {
        union semun arg;
        arg.val = value;
        if(semctl(id, sem_num, SETVAL, arg) == -1) error("semctl SETVAL error");
    }

    void sem_remove() {
        if (semctl(id, 0, IPC_RMID) == -1) {
            error("semctl IPC_RMID error");
        }
    }
};

class MsgQueue {
    public:
    key_t key;
    int id;
    MsgQueue(key_t msg_key) {
        key = msg_key;
    }

    void msg_create() {
        id = msgget(key, IPC_CREAT | 0666);
        if(id == -1) error("msgget error");
    }

    void msg_attach() {
        id = msgget(key, 0666);
        if(id == -1) error("msgget error");
    }

    void msg_send(int msgtype) {
        msgbuf message = {.mtype = msgtype};
        msgsnd(id, &message, 1, 0);
    }

    void msg_rcv(int msgtype) {
        msgbuf message;
        msgrcv(id, &message, 1, msgtype, 0);
    }

    void msg_ctl() {
        if (msgctl(id, IPC_RMID, NULL) == -1) error("msgctl error");
    }
};



#endif // CLASSES_H
