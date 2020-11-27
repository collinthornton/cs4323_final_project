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

    /*
    for(int i=0; i<MAX_CLIENTS; ++i) {
        sharedGym->arrivingList[i].current_couch = NULL;
        sharedGym->arrivingList[i].current_trainer = NULL;
        sharedGym->arrivingList[i].workout = NULL;
        sharedGym->arrivingList[i].pid = 0;
        sharedGym->arrivingList[i].state = ARRIVING;

        sharedGym->waitingList[i].current_couch = NULL;
        sharedGym->waitingList[i].current_trainer = NULL;
        sharedGym->waitingList[i].workout = NULL;
        sharedGym->waitingList[i].pid = 0;
        sharedGym->waitingList[i].state = WAITING;

        sharedGym->workoutList[i].current_couch = NULL;
        sharedGym->workoutList[i].current_trainer = NULL;
        sharedGym->workoutList[i].workout = NULL;
        sharedGym->workoutList[i].pid = 0;
        sharedGym->workoutList[i].state = TRAINING;
    }
    */

    for(int i=0; i<MAX_TRAINERS; ++i) {
        sharedGym->trainerList[i].client_pid = 0;
        sharedGym->trainerList[i].pid = 0;
        sharedGym->trainerList[i].state = FREE;
    }

    sharedGym->maxCouches = maxCouches;
    sharedGym->unit_time = UNIT_TIME;
    sharedGym->len_arriving = 0;
    sharedGym->len_waiting = 0;
    sharedGym->len_workout = 0;
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
    gym->workoutList = client_list_init();
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
    sharedGym->len_workout = gym->workoutList->len;
    sharedGym->len_trainer = gym->trainerList->len;

    ClientNode *tmp = gym->arrivingList->HEAD;
    for(int i=0; i<sharedGym->len_arriving; ++i) {
        if(tmp == NULL) {
            perror("update_shared_gym arriving_list");
            break;
        }        
        copy_client(&sharedGym->arrivingList[i], tmp->node);
        //sharedGym->arrivingList[i] = *tmp->node;
        tmp = tmp->next;
    }

    tmp = gym->waitingList->HEAD;
    for(int i=0; i<sharedGym->len_waiting; ++i) {
        if(tmp == NULL)  {
            perror("updated_shared_gym waiting list");
            break;
        }        
        copy_client(&sharedGym->waitingList[i], tmp->node);
        //sharedGym->waitingList[i] = *tmp->node;
        tmp = tmp->next;
    }

     tmp = gym->workoutList->HEAD;
    for(int i=0; i<sharedGym->len_workout; ++i) {
        if(tmp == NULL)  {
            perror("updated_shared_gym workout list");
            break;
        }        
        copy_client(&sharedGym->workoutList[i], tmp->node);
        //sharedGym->workoutList[i] = *tmp->node;
        tmp = tmp->next;
    }   

    TrainerNode *ttmp = gym->trainerList->HEAD;
    for(int i=0; i<sharedGym->len_trainer; ++i) {
        if(ttmp == NULL) {
            perror("updated_shared_gym trainer list");
            break;
        }
        copy_trainer(&sharedGym->trainerList[i], ttmp->node);
        //sharedGym->trainerList[i] = *ttmp->node;
        ttmp = ttmp->next;
    }
}


Gym* update_gym(Gym *gym, SharedGym *sharedGym)  {
    // Easiest to just delete everything and start from scratch
    client_list_del_clients(gym->arrivingList);
    client_list_del_clients(gym->waitingList);
    client_list_del_clients(gym->workoutList);
    trainer_list_del_trainers(gym->trainerList);

    client_list_del(gym->arrivingList);
    client_list_del(gym->waitingList);
    client_list_del(gym->workoutList);
    trainer_list_del(gym->trainerList);

    gym->arrivingList = client_list_init();
    gym->waitingList = client_list_init();
    gym->workoutList = client_list_init();
    gym->trainerList = trainer_list_init();

    gym->maxCouches = sharedGym->maxCouches;
    gym->unit_time = sharedGym->unit_time;

    for(int i=0; i<sharedGym->len_arriving; ++i) {
        Client *client = client_init(0, ARRIVING, NULL, NULL, NULL);
        copy_client(client, &sharedGym->arrivingList[i]);
        client_list_add_client(client, gym->arrivingList);
    }
    for(int i=0; i<sharedGym->len_waiting; ++i) {
        Client *client = client_init(0, ARRIVING, NULL, NULL, NULL);
        copy_client(client, &sharedGym->waitingList[i]);
        client_list_add_client(client, gym->waitingList);
    }
    for(int i=0; i<sharedGym->len_workout; ++i) {
        Client *client = client_init(0, ARRIVING, NULL, NULL, NULL);
        copy_client(client, &sharedGym->workoutList[i]);
        client_list_add_client(client, gym->workoutList);
    }
    for(int i=0; i<sharedGym->len_trainer; ++i) {
        Trainer *trainer = trainer_init(-1, -1, FREE);
        copy_trainer(trainer, &sharedGym->trainerList[i]);
        trainer_list_add_trainer(trainer, gym->trainerList);  
    }  
}



void gym_del(Gym *gym) {
    client_list_del_clients(gym->arrivingList);
    client_list_del_clients(gym->waitingList);
    client_list_del_clients(gym->workoutList);
    trainer_list_del_trainers(gym->trainerList);    
    client_list_del(gym->arrivingList);
    client_list_del(gym->waitingList);
    client_list_del(gym->workoutList);
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


Client* copy_client(Client *dest, Client *src) {
    if(dest == NULL || src == NULL) {
        perror("copy_client invalid_argument");
        return NULL;
    }

    *dest = *src;
    //if(src->current_trainer != NULL) {
    //dest->current_trainer = src->current_trainer;
    //}
    
    //if(src->current_couch != NULL)
    //    *dest->current_couch = *src->current_couch;
    //else
    //    dest->current_couch = NULL;
    
    //if(src->workout != NULL)
    //    *dest->workout = *src->workout;

    return dest;
}

Trainer* copy_trainer(Trainer *dest, Trainer *src) {
    if(dest == NULL || src == NULL) {
        perror("copy_trainer invalide_argument");
        return NULL;
    }

    *dest = *src;

    /*
    if(src->current_client != NULL)
        *dest->current_client = *src->current_client;
    */

    return dest;
}