#ifndef CLASSES_H
#define CLASSES_H

#include <unistd.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <string>
#include <stdexcept>
#include <cstring>

class Semaphore {
public:
    Semaphore(key_t key, int numSems, int flags) {
        semid = semget(key, numSems, flags);
        if (semid == -1) {
            throw std::runtime_error("Failed to create semaphore");
        }
    }

    void setValue(int semNum, int value) {
        union semun {
            int val;
            struct semid_ds *buf;
            unsigned short *array;
        } arg;
        arg.val = value;
        if (semctl(semid, semNum, SETVAL, arg) == -1) {
            throw std::runtime_error("Failed to set semaphore value");
        }
    }

    void wait(int semNum) {
        struct sembuf op = {semNum, -1, 0};
        if (semop(semid, &op, 1) == -1) {
            throw std::runtime_error("Semaphore wait operation failed");
        }
    }

    void signal(int semNum) {
        struct sembuf op = {semNum, 1, 0};
        if (semop(semid, &op, 1) == -1) {
            throw std::runtime_error("Semaphore signal operation failed");
        }
    }

    ~Semaphore() {
        if (semctl(semid, 0, IPC_RMID) == -1) {
            throw std::runtime_error("Failed to remove semaphore");
        }
    }

private:
    int semid;
};

class SharedMemory {
public:
    SharedMemory(key_t key, size_t size, int flags) {
        shmid = shmget(key, size, flags);
        if (shmid == -1) {
            throw std::runtime_error("Failed to create shared memory");
        }
    }

    void* attach(int flags = 0) {
        void* addr = shmat(shmid, nullptr, flags);
        if (addr == (void*)-1) {
            throw std::runtime_error("Failed to attach shared memory");
        }
        return addr;
    }

    void detach(void* addr) {
        if (shmdt(addr) == -1) {
            throw std::runtime_error("Failed to detach shared memory");
        }
    }

    ~SharedMemory() {
        if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
            throw std::runtime_error("Failed to remove shared memory");
        }
    }

private:
    int shmid;
};

class MessageQueue {
public:
    MessageQueue(key_t key, int flags) {
        msqid = msgget(key, flags);
        if (msqid == -1) {
            throw std::runtime_error("Failed to create message queue");
        }
    }

    void send(const void* msg, size_t size, int flags) {
        if (msgsnd(msqid, msg, size, flags) == -1) {
            throw std::runtime_error("Failed to send message");
        }
    }

    ssize_t receive(void* msg, size_t size, long msgType, int flags) {
        ssize_t ret = msgrcv(msqid, msg, size, msgType, flags);
        if (ret == -1) {
            throw std::runtime_error("Failed to receive message");
        }
        return ret;
    }

    ~MessageQueue() {
        if (msgctl(msqid, IPC_RMID, nullptr) == -1) {
            throw std::runtime_error("Failed to remove message queue");
        }
    }

private:
    int msqid;
};


class NowyProces {
public:
    NowyProces(const std::string& path) {
        pid = fork();

        if (pid < 0) {
            throw std::runtime_error("Fork failed");
        } else if (pid == 0) {
            // Child process
            if (execl(path.c_str(), path.c_str(), nullptr) == -1) {
                perror(("execl failed for " + path).c_str());
                _exit(EXIT_FAILURE);
            }
        }
        // Parent process continues
    }

    operator pid_t() const {
        return pid;
    }

    pid_t getPid() const {
        return pid;
    }

private:
    pid_t pid;
};

#endif // CLASSES_H
