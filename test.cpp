#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int main() {
    pthread_t tid = pthread_self();
    printf("ID wątku (pthread_t): %lu\n", (unsigned long)tid);
}