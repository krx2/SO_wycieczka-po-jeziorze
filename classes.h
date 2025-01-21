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

class SharedMem {
    key_t key;
    size_t size;
    int id;
    
    public:
    SharedMem(key_t memory_key, size_t memory_size) {
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

#endif // CLASSES_H
