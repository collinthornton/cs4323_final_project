#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHARED_KEY 0x1234
#define BUFFER_SIZE 1024
#define NUMBER_OF_COUCHES 5
#define NUMBER_OF_TRAINERS 4

struct gym {
    int numberTrainersAvailable;
    int numberCustomersArriving;
    int numberCustomersWaiting;
} typedef gym;

int run_scenario(int scenarioNumber);