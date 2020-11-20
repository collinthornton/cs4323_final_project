#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "include/simulator.h"
#include "include/customer.h"
#include "include/trainer.h"

int main(){
    printf("-----------------Scenario 1: Deadlock----------------\n");
    run_scenario(1);

    sleep(5);
    
    printf("-----------------Scenario 3: First come first serve violation----------------\n");
    run_scenario(3);
}

int run_scenario(int scenarioNumber){
    gym *sharedGym;

    int waitingProcessID = 0;
    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(gym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space\n");
        return 1;
    }

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        printf("Could not attached to the shared memory\n");
        return 1;
    }

    sharedGym->numberCustomersArriving = 0;
    sharedGym->numberCustomersWaiting = 0;
    sharedGym->numberTrainersAvailable = 0;

    if (shmdt(sharedGym) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return 1;
    }

    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1){
        printf("Something went wrong with the shmctl function\n");
        return 1;
    }

    int processOne = fork();

    if (processOne < 0){
        printf("Something went wrong with the fork()");
        return 1;
    } else if (processOne == 0){
        //Child process
        customer_start(1);
    } else {
        //Parent process
        int processTwo = fork();
        if (processTwo < 0){
            printf("Something went wrong with the second fork()");
        } else if (processTwo == 0){
            if (scenarioNumber == 1 || scenarioNumber == 3){
                trainer_start();
            } else if (scenarioNumber == 2) {
                customer_start(2);
            }
        } else {
            while ((waitingProcessID = wait(NULL)) > 0);
            if (scenarioNumber == 3){
                customer_start(2);
            }
        }
    }

    return 0;
}