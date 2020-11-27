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

static SharedGym *sharedGym;
static char SHARED_GYM_SEM_NAME[] = "/sem_shmem_gym";
static bool sem_opened = false;


int init_shared_gym(int maxCouches){
    shared_gym_sem = sem_open(SHARED_GYM_SEM_NAME, O_CREAT, 0644, 1);
    if(shared_gym_sem == SEM_FAILED) {
        perror("init_shared_gym sem_failed_to_open");
        exit(1);
    }
    
    sem_opened = true;

    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space, %d\n", errno);
        return 1;
    }

    // TAKE THE SEMAPHORE
    sem_wait(shared_gym_sem);
    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        sem_post(shared_gym_sem);
        printf("Could not attached to the shared memory\n");
        return 1;
    }

    for(int i=0; i<MAX_CLIENTS; ++i) {
        sharedGym->arrivingList[i].pid      = -1;
        sharedGym->arrivingList[i].state    = -1;
        sharedGym->waitingList[i].pid       = -1;
        sharedGym->waitingList[i].state     = -1;
        sharedGym->workoutList[i].pid       = -1;
        sharedGym->workoutList[i].state     = -1;
    }
    for(int i=0; i<MAX_TRAINERS; ++i) {
        sharedGym->trainerList[i].client_pid = -1;
        sharedGym->trainerList[i].pid        = -1;
        sharedGym->trainerList[i].state      = FREE;
    }

    sharedGym->maxCouches = maxCouches;
    sharedGym->unit_time = UNIT_TIME;

    if (shmdt(sharedGym) == -1){
        sem_post(shared_gym_sem);
        printf("Something happened trying to detach from shared memory\n");
        return 1;
    }
 
    // RELEASE THE SEMAPHORE
    sem_post(shared_gym_sem);
 
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

void open_shared_gym(){
    if(sem_opened == false)
        shared_gym_sem = sem_open(SHARED_GYM_SEM_NAME, O_CREAT, 0644, 1);

    if(shared_gym_sem == SEM_FAILED) {
        perror("open_shared_gym sem_failed");
        exit(1);
    }

    //First get shared object from memory
    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        printf("Something went wrong allocating the shared memory space\n");
        return;
    }

    // TAKE THE SEMAPHORE
    sem_wait(shared_gym_sem);

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        sem_post(shared_gym_sem);
        printf("Could not attached to the shared memory\n");
        return;
    }    

    // RELEASE THE SEMAPHORE
    sem_post(shared_gym_sem);
    return;
}


//TODO This can't wipe out the old data. Only update data if pid's are equivalent
void update_shared_gym(Gym *gym) {
    //! CANNOT MODIFY OTHER PROCESS STATES

    Client *client = client_list_find_pid(getpid(), gym->arrivingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->waitingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->workoutList);

    Trainer *trainer = NULL;
    if(client == NULL) trainer = trainer_list_find_pid(getpid(), gym->trainerList);

    /*
    if(client == NULL && trainer == NULL) {
        perror("update_shared_gym pid not found");
        exit(1);
    }
    */

    bool in_list = (client != NULL || trainer != NULL) ? true : false;
    bool is_client = (client == NULL) ? false : true;

    sem_wait(shared_gym_sem);

    //! We shouldn't change these
    //sharedGym->unit_time = gym->unit_time;
    //sharedGym->maxCouches = gym->maxCouches;


    // 1.) Iterate through array
    // 1a.) If entry is in wrong list, set pid and state to -1
    // 1b.) If entry should be in list, set equal to client
    // 1bi.) Add to first empty spot


    // Delete everything with same pid

    for(int i=0; i < MAX_CLIENTS; ++i) {
        pid_t tmp_pid = sharedGym->arrivingList[i].pid;
        if(tmp_pid == getpid()) {
            sharedGym->arrivingList[i].pid = -1;
            sharedGym->arrivingList[i].state = -1;
        }

        tmp_pid = sharedGym->waitingList[i].pid;
        if(tmp_pid == getpid()) {
            sharedGym->waitingList[i].pid = -1;
            sharedGym->waitingList[i].state = -1;
        }

        tmp_pid = sharedGym->workoutList[i].pid;
        if(tmp_pid == getpid()) {
            sharedGym->workoutList[i].pid = -1;
            sharedGym->workoutList[i].state = -1;
        }
    }

    for(int i=0; i < MAX_TRAINERS; ++i) {
        pid_t tmp_pid = sharedGym->trainerList[i].pid;
        if(tmp_pid == getpid()) {
            sharedGym->trainerList[i].client_pid = -1;
            sharedGym->trainerList[i].state = -1;
        }
    }
        

    // Add us back to the correct list

    if(in_list && is_client) {
        if(client->state == ARRIVING) {
            for(int i=0; i < MAX_CLIENTS; ++i) {
                if(sharedGym->arrivingList[i].pid == -1) {
                    copy_client(&sharedGym->arrivingList[i], client);
                    break;
                }
            }
        }
        else if(client->state == WAITING) {
            for(int i=0; i < MAX_CLIENTS; ++i) {
                if(sharedGym->waitingList[i].pid == -1) {
                    copy_client(&sharedGym->waitingList[i], client);
                    break;
                }
            }
        }
        else if(client->state == TRAINING) {
            for(int i=0; i < MAX_CLIENTS; ++i) {
                if(sharedGym->workoutList[i].pid == -1) {
                    copy_client(&sharedGym->workoutList[i], client);
                    break;
                }
            }
        }
    }
    else if(in_list && !is_client) {
        for(int i=0; i < MAX_TRAINERS; ++i) {
            if(sharedGym->trainerList[i].pid == -1) {
                copy_trainer(&sharedGym->trainerList[i], trainer);
                break;
            }
        }
    }


    sem_post(shared_gym_sem);
}

Gym* update_gym(Gym *gym)  {
    //! CANNOT MODIFY CURRENT PROCESS STATE
    // Easiest to just delete everything except current process and start from scratch

    Client *client = client_list_find_pid(getpid(), gym->arrivingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->waitingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->workoutList);

    Trainer *trainer = NULL;
    if(client == NULL) trainer = trainer_list_find_pid(getpid(), gym->trainerList);

    /*
    if(client == NULL && trainer == NULL) {
        perror("update_gym pid not found");
        exit(1);
    }
    */

    bool in_list = (client != NULL || trainer != NULL) ? true : false;
    bool is_client = (client == NULL) ? false : true;

    // Spare current process from deletion
    client_list_del_clients(getpid(), gym->arrivingList);
    client_list_del_clients(getpid(), gym->waitingList);
    client_list_del_clients(getpid(), gym->workoutList);
    trainer_list_del_trainers(getpid(), gym->trainerList);

    client_list_del(gym->arrivingList);
    client_list_del(gym->waitingList);
    client_list_del(gym->workoutList);
    trainer_list_del(gym->trainerList);

    gym->arrivingList = client_list_init();
    gym->waitingList = client_list_init();
    gym->workoutList = client_list_init();
    gym->trainerList = trainer_list_init();


    sem_wait(shared_gym_sem);

    gym->maxCouches = sharedGym->maxCouches;
    gym->unit_time = sharedGym->unit_time;

    // Update everything with differnt pid
    for(int i=0; i < MAX_CLIENTS; ++i) {
        pid_t tmp_pid = sharedGym->arrivingList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Client *client = client_init(0, ARRIVING, NULL, NULL, NULL);
            copy_client(client, &sharedGym->arrivingList[i]);
            client_list_add_client(client, gym->arrivingList);
        }

        tmp_pid = sharedGym->waitingList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Client *client = client_init(0, ARRIVING, NULL, NULL, NULL);
            copy_client(client, &sharedGym->waitingList[i]);
            client_list_add_client(client, gym->waitingList);
        }

        tmp_pid = sharedGym->workoutList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Client *client = client_init(0, ARRIVING, NULL, NULL, NULL);
            copy_client(client, &sharedGym->workoutList[i]);
            client_list_add_client(client, gym->workoutList);
        }
    }

    for(int i=0; i < MAX_TRAINERS; ++i) {
        pid_t tmp_pid = sharedGym->trainerList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Trainer *trainer = trainer_init(-1, -1, FREE);
            copy_trainer(trainer, &sharedGym->trainerList[i]);
            trainer_list_add_trainer(trainer, gym->trainerList);  
        }
    }  

    sem_post(shared_gym_sem);


    // update current process
    if(in_list && is_client) {
        switch(client->state) {
            case WAITING:
                client_list_add_client(client, gym->waitingList);
                break;
            case TRAINING:
                client_list_add_client(client, gym->workoutList);
                break;
            case ARRIVING:
                client_list_add_client(client, gym->arrivingList);
                break;
        }
    }
    else if(in_list && !is_client) {
        trainer_list_add_trainer(trainer, gym->trainerList);
    }
}



void gym_del(Gym *gym) {
    client_list_del_clients(-1, gym->arrivingList);
    client_list_del_clients(-1, gym->waitingList);
    client_list_del_clients(-1, gym->workoutList);
    trainer_list_del_trainers(-1, gym->trainerList);    
    client_list_del(gym->arrivingList);
    client_list_del(gym->waitingList);
    client_list_del(gym->workoutList);
    trainer_list_del(gym->trainerList);
    free(gym);
}

void close_shared_gym(){   
    sem_wait(shared_gym_sem);
    if (shmdt(sharedGym) == -1){
        printf("Something happened trying to detach from shared memory\n");
        return;
    }      
    sem_post(shared_gym_sem);
    sem_close(shared_gym_sem);
}

void destroy_shared_gym() {
    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);
    
    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1){
        // It's already been closed by another process. Just ignore.
        printf("Something went wrong with the shmctl function\n");
        return;
    }    

    sem_unlink(SHARED_GYM_SEM_NAME);
}


Client* copy_client(Client *dest, Client *src) {
    if(dest == NULL || src == NULL) {
        perror("copy_client invalid_argument");
        return NULL;
    }

    *dest = *src;
    return dest;
}

Trainer* copy_trainer(Trainer *dest, Trainer *src) {
    if(dest == NULL || src == NULL) {
        perror("copy_trainer invalide_argument");
        return NULL;
    }

    *dest = *src;
    return dest;
}