#include <iostream>
#include "classes.h"

using namespace std;

int main() {
    key_t key;
    key = ftok("mainp", 1);

    SharedMemory(key, 1024, 0666);
    MessageQueue(key, 0666);

}