#include "include/simulator.h"
#include "include/trainer.h"

void trainer_start(){
    //First get shared object from memory
    gym *sharedGym;

    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(gym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("TRAINER: Something went wrong allocating the shared memory space\n");
        return;
    }

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        printf("TRAINER: Could not attached to the shared memory\n");
        return;
    }

    printf("TRAINER: Trainer has shared gym. Here are the contents:\n");
    printf("TRAINER: NumberCustomersArriving: %i\n",sharedGym->numberCustomersArriving);
    printf("TRAINER: NumberTrainersAvailable: %i\n", sharedGym->numberTrainersAvailable);
    printf("TRAINER: NumberCustomersWaiting: %i\n", sharedGym->numberCustomersWaiting);

    sleep(1);


    //Trainer checks for waiting customers
    if (sharedGym->numberCustomersArriving > 0){
        printf("TRAINER: Trainer found a customer in the arrival area\n");
        sharedGym->numberCustomersArriving -= 1;
    } else {
        //Trainer checks for arriving customers
        if (sharedGym->numberCustomersWaiting > 0){
            printf("TRAINER: Trainer found a customer in the waiting room\n");
            sharedGym->numberCustomersWaiting -= 1;
        } else {
            printf("TRAINER: No customers in the waiting room\n");
            printf("TRAINER: Trainer now available, waiting for customers\n");
            sharedGym->numberTrainersAvailable += 1;
        }
    }

    if (shmdt(sharedGym) == -1){
        printf("TRAINER: Something happened trying to detach from shared memory\n");
        return;
    }    

}

Trainer* find_trainer_with_client(Client* clientToFind, TrainerList* trainerList){
    if (trainerList == NULL) return NULL;

    TrainerNode *tmp = trainerList->HEAD;

    while(tmp != NULL){
        if (tmp->node->current_client == clientToFind){
            return tmp->node;
        }
    }

    return NULL;
}

Trainer* find_available_trainer(TrainerList* trainerList){
    if (trainerList == NULL) return NULL;

    TrainerNode *tmp = trainerList->HEAD;

    while(tmp != NULL){
        if (tmp->node->current_client == NULL){
            return tmp->node;
        }
    }

    return NULL;
}

Trainer* find_trainer_on_phone(TrainerList* trainerList){
    if (trainerList == NULL) return NULL;

    TrainerNode *tmp = trainerList->HEAD;

    while(tmp != NULL){
        if (tmp->node->state == ON_PHONE){
            return tmp->node;
        }
    }

    return NULL;
}