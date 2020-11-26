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

#define UNIT_TIME 1       // Seconds

//////////////////////////////
//
// Gym functions
//

int init_shared_gym(int maxCouches){
    SharedGym *sharedGym;

    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space, %d\n", errno);
        return 1;
    }

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        printf("Could not attached to the shared memory\n");
        return 1;
    }

    //sharedGym->arrivingList = client_list_init();
    //sharedGym->trainerList = trainer_list_init();
    //sharedGym->waitingList = client_list_init();
    sharedGym->maxCouches = maxCouches;
    sharedGym->unit_time = UNIT_TIME;
    sharedGym->len_arriving = 0;
    sharedGym->len_waiting = 0;
    sharedGym->len_trainer = 0;

    if (shmdt(sharedGym) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return 1;
    }
 
    return 0;
}

Gym* gym_init() {
    Gym *gym = (Gym*)malloc(sizeof(Gym));
    gym->arrivingList = client_list_init();
    gym->waitingList = client_list_init();
    gym->trainerList = trainer_list_init();
    gym->unit_time = UNIT_TIME;
    gym->maxCouches = 0;

    return gym;
}

SharedGym* get_shared_gym(){
    //First get shared object from memory
    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space\n");
        return NULL;
    }

    SharedGym *sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        printf("Could not attached to the shared memory\n");
        return NULL;
    }    

    return sharedGym;
}


void update_shared_gym(SharedGym *sharedGym, Gym *gym) {
    sharedGym->unit_time = gym->unit_time;
    sharedGym->maxCouches = gym->maxCouches;
    sharedGym->len_arriving = gym->arrivingList->len;
    sharedGym->len_waiting = gym->waitingList->len;
    sharedGym->len_trainer = gym->trainerList->len;

    ClientNode *tmp = gym->arrivingList->HEAD;
    for(int i=0; i<sharedGym->len_arriving; ++i) {
        if(tmp == NULL) {
            perror("update_shared_gym arriving_list");
            break;
        }        
        sharedGym->arrivingList[i] = *tmp->node;
        tmp = tmp->next;
    }

    tmp = gym->waitingList->HEAD;
    for(int i=0; i<sharedGym->len_waiting; ++i) {
        if(tmp == NULL)  {
            perror("updated_shared_gym waiting list");
            break;
        }        
        sharedGym->waitingList[i] = *tmp->node;
        tmp = tmp->next;
    }

    TrainerNode *ttmp = gym->trainerList->HEAD;
    for(int i=0; i<sharedGym->len_trainer; ++i) {
        if(ttmp == NULL) {
            perror("updated_shared_gym trainer list");
            break;
        }
        sharedGym->trainerList[i] = *ttmp->node;
        ttmp = ttmp->next;
    }
}


Gym* update_gym(Gym *gym, SharedGym *sharedGym)  {
    gym->maxCouches = sharedGym->maxCouches;
    gym->unit_time = sharedGym->unit_time;

    for(int i=0; i<sharedGym->len_arriving; ++i) client_list_add_client(&sharedGym->arrivingList[i], gym->arrivingList);
    for(int i=0; i<sharedGym->len_waiting; ++i) client_list_add_client(&sharedGym->waitingList[i], gym->waitingList);
    for(int i=0; i<sharedGym->len_trainer; ++i) trainer_list_add_trainer(&sharedGym->trainerList[i], gym->trainerList);    
}



void gym_del(Gym *gym) {
    client_list_del(gym->arrivingList);
    client_list_del(gym->waitingList);
    trainer_list_del(gym->trainerList);
    free(gym);
}

void clean_shared_gym(SharedGym* sharedGym){
    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);
    
    if (shmdt(sharedGym) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return;
    }      

    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1){
        printf("Something went wrong with the shmctl function\n");
        return;
    }    
}
