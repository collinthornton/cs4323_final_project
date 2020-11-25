// ##########################################
// 
//   Author  -   Collin Thornton / Robert Cook
//   Email   -   robert.cook@okstate.edu
//   Brief   -   Final Project gym resource source
//   Date    -   11-20-20
//
// ########################################## 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "gym.h"


//////////////////////////////
//
// Gym functions
//

int init_shared_gym(int maxCouches){
    Gym *sharedGym;

    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(Gym), IPC_CREAT|0644);

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

    pid_t pid = 5;
    int test = 4;

    test = pid;

    sharedGym->arrivingList = client_list_init();
    sharedGym->trainerList = trainer_list_init();
    sharedGym->waitingList = client_list_init();
    sharedGym->maxCouches = maxCouches;

    if (shmdt(sharedGym) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return 1;
    }

    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1){
        printf("Something went wrong with the shmctl function\n");
        return 1;
    }

    return 0;
}

Gym* get_shared_gym(){
    //First get shared object from memory
    Gym *sharedGym;

    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(Gym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space\n");
        return NULL;
    }

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        printf("Could not attached to the shared memory\n");
        return NULL;
    }    

    return sharedGym;
}

void clean_shared_gym(Gym* sharedGym){
    if (shmdt(sharedGym) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return;
    }        
}
