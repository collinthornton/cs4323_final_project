/*-------------------------------------------------------------

    Author  -   Robert Cook
    Email   -   robert.cook@okstate.edu
    Date    -   11-24-2020
    Description
        This file represents the "entrance" of the gym
        This will be what handles all of the client arrivals
        and can be thought of as the front doors of the gym

-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "client.h"
#include "trainer.h"
#include "entrance.h"


void open_gym(int numberTrainers, int numberCouches, int numberClients, int useSemaphors){
    //First, established the shared memory object
    init_shared_gym(numberCouches);

    int processID;

    //Second, spin up the processes for trainers and clients
    for (int i = 0; i < (numberTrainers + numberClients); i++){
        processID = fork();
        if (processID < 0){
            //Something happened and we need to bail out
            printf("There was an error spinning up the child processes\n");
            return;
        } else if (processID == 0){
            //We are a child process so we need to get out of this loop to ensure we don't spin up MORE processes
            break;
        }
    }

    if (processID != 0){
        while(processID != waitpid(-1,NULL,0)){
            if (errno == ECHILD)
                break;
        }

        //All children have finished, let's clean things up now

    } else {
        //This is a child, so let's start up some trainers and clients
        int childProcessID = getpid();
        Gym *sharedGymObject = get_shared_gym();

        if (sharedGymObject == NULL){
            //Something went wrong retrieving it
            printf("There was an error retrieving the shared Gym object\n");
            return;
        }

        //First get the trainer list built out
        if (sharedGymObject->trainerList->size < numberTrainers){
            //Add this process to the trainer list
            Trainer *newTrainer = trainer_init(FREE, NULL, childProcessID);
            sharedGymObject->trainerList = trainer_add_to_list(newTrainer, sharedGymObject->trainerList);
        } else {
            //Second, start having clients "arrive" and get assigned to trainers

            //Make this process an arriving client
            Client *newClient = client_init(ARRIVING, childProcessID);
            client_arriving_event(sharedGymObject, newClient);

            if(newClient->state == LEAVING){
                //Something happened when the customer arrived to cause them to leave, so we'll need to kill the process after this
            }
        }

        //Fourth, figure out how to handle demonstrating the boundary cases
    }


}

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

    sharedGym->arrivingList = client_init_list();
    sharedGym->trainerList = trainer_init_list();
    sharedGym->waitingList = client_init_list();
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

void client_arriving_event(Gym* sharedGym, Client* newClient){
    //First add it to the arriving LL 
    client_add_to_list(newClient, sharedGym->arrivingList);

    //Next, check to see if a trainer grabbed the client
    if (find_trainer_with_client(newClient, sharedGym->trainerList) != NULL){
        //We are done here.
        return;
    }

    //Next see if there are any trainers on their phones
    Trainer* trainer = find_trainer_on_phone(sharedGym->trainerList);
    if (trainer != NULL){
        client_rem_from_list(newClient, sharedGym->arrivingList);

        newClient->state = TRAINING;
        trainer->current_client = newClient;
        trainer->state = WITH_CLIENT;
        
        return;
    }

    //Time to go to the waiting room
    client_rem_from_list(newClient, sharedGym->arrivingList);
    newClient->state = TRAVELING;
    
    //This is the "traveling" piece, also a semaphore area
    sleep(3);
    //End the semaphore

    //Now check if there is room on the couches
    if(sharedGym->waitingList->size<sharedGym->maxCouches){
        //Another semaphore areas
        newClient->state = WAITING;
        client_add_to_list(newClient, sharedGym->waitingList);
        sharedGym->maxCouches++;
        //End the semaphore
    } else {
        //No couches available, no trainers available, time to leave
        newClient->state = LEAVING;
    }
}


