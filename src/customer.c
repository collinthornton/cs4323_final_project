#include "include/simulator.h"
#include "include/customer.h"

void customer_start(int customerNumber){
    //First get shared object from memory
    gym *sharedGym;

    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(gym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("CUSTOMER %i: Something went wrong allocating the shared memory space\n", customerNumber);
        return;
    }

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        printf("CUSTOMER %i: Could not attached to the shared memory\n", customerNumber);
        return;
    }

    printf("CUSTOMER %i: Customer has shared gym. Here are the contents:\n", customerNumber);
    printf("CUSTOMER %i: NumberCustomersArriving: %i\n", customerNumber,sharedGym->numberCustomersArriving);
    printf("CUSTOMER %i: NumberTrainersAvailable: %i\n", customerNumber, sharedGym->numberTrainersAvailable);
    printf("CUSTOMER %i: NumberCustomersWaiting: %i\n", customerNumber, sharedGym->numberCustomersWaiting);

    //Customer arrives - Checks for available trainers
    sharedGym->numberCustomersArriving += 1;

    if (sharedGym->numberTrainersAvailable > 0){
        //Do something
        printf("CUSTOMER %i: Customer found available trainers\n", customerNumber);
        sharedGym->numberTrainersAvailable -= 1;
    } else {
        //No trainers available - customer travels to waiting
        //Travel to waiting room
        sharedGym->numberCustomersArriving -= 1;
        printf("CUSTOMER %i: No trainers available, customer traveling to waiting room\n", customerNumber);
        sleep(3);
        //Customer in waiting
        sharedGym->numberCustomersWaiting += 1;
        printf("CUSTOMER %i: Customer now in waiting room\n", customerNumber);
    }

    if (shmdt(sharedGym) == -1){
        printf("CUSTOMER %i: Something happened trying to detach from shared memory\n", customerNumber);
        return;
    }    
}
