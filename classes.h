#ifndef CLASSES_H
#define CLASSES_H

#include <unistd.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <string>
#include <stdexcept>
#include <cstring>

void error(const char* msg) {
    char errmsg[50];
    sprintf(errmsg, "Error %s", msg);
    perror(errmsg);
    exit(EXIT_FAILURE);
}

/*struct msgbuf {
    long mtype;
    char mtext[1];
};
*/

class SharedMem {
    key_t key;
    int size;
    int id;
    
    public:
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

class MsgQueue {
    key_t key;
    int id;

    public:
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
