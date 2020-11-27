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

#include "entrance.h"

/*
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
        if (sharedGymObject->trainerList->len < numberTrainers){
            //Add this process to the trainer list
            Trainer *newTrainer = trainer_init(childProcessID, FREE, NULL);
            trainer_list_add_trainer(newTrainer, sharedGymObject->trainerList);
        } else {
            //Second, start having clients "arrive" and get assigned to trainers

            //Make this process an arriving client
            Client *newClient = client_init(childProcessID, ARRIVING, NULL, NULL, NULL);
            client_arriving_event(sharedGymObject, newClient);

            if(newClient->state == LEAVING){
                //Something happened when the customer arrived to cause them to leave, so we'll need to kill the process after this
            }
        }

        //Fourth, figure out how to handle demonstrating the boundary cases
    }


}
*/
void client_arriving_event(Gym* sharedGym, Client* newClient){
    //First add it to the arriving LL 
    client_list_add_client(newClient, sharedGym->arrivingList);

    //Next, check to see if a trainer grabbed the client
    if (trainer_list_find_client(newClient->pid, sharedGym->trainerList) != NULL){
        //We are done here.
        return;
    }

    //Next see if there are any trainers on their phones
    Trainer* trainer = trainer_list_find_phone(sharedGym->trainerList);
    if (trainer != NULL){
        client_list_rem_client(newClient, sharedGym->arrivingList);

        newClient->state = TRAINING;
        trainer->client_pid = newClient->pid;
        trainer->state = WITH_CLIENT;
        
        return;
    }

    //Time to go to the waiting room
    client_list_rem_client(newClient, sharedGym->arrivingList);
    newClient->state = MOVING;
}
