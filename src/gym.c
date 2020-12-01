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
#include <time.h>

#include "gym.h"


// Relative speed of simulation
#define UNIT_TIME 250       // Milliseconds

// Key for shared memory
#define SHARED_KEY 0x1234

// Semaphore for accessing shared memory
static sem_t *shared_gym_sem;

// Name of semaphore
static char SHARED_GYM_SEM_NAME[] = "/sem_shmem_gym";

// SharedGym in shared memory
static SharedGym *sharedGym;



//////////////////////////////
//
// Gym functions
//


/**
 * @brief Initialize a gym on the heap
 * @return (Gym*) newly allocated struct
 */
Gym* gym_init() {
    Gym *gym = (Gym*)malloc(sizeof(Gym));
    gym->arrivingList = client_list_init();
    gym->waitingList = client_list_init();
    gym->workoutList = client_list_init();
    gym->trainerList = trainer_list_init();
    gym->unit_time = UNIT_TIME;
    gym->num_trainers = 0;
    gym->boundary_case = true;
    gym->realistic = true;
    gym->detect_deadlock = true;
    gym->fix_deadlock = true;
    gym->trainer_log = false;
    gym->maxCouches = 0;
    gym->deadlock_victim = -1;

    return gym;
}


/**
 * @brief Free a gym from the heap
 * @param gym (Gym*) gym to be freed
 */
void gym_del(Gym *gym) {
    client_list_del_clients(-100, gym->arrivingList);
    client_list_del_clients(-100, gym->waitingList);
    client_list_del_clients(-100, gym->workoutList);
    trainer_list_del_trainers(-100, gym->trainerList);    
    client_list_del(gym->arrivingList);
    client_list_del(gym->waitingList);
    client_list_del(gym->workoutList);
    trainer_list_del(gym->trainerList);
    free(gym);
}



//////////////////////////////
//
// Shared memory and semaphore functions
//


/**
 * @brief Initialize the shared memory space and semaphore. Should only be called by parent process
 * @param maxCouches (int) Max number of clients in waiting room
 * @param numTrainers (int) total number of trainers in simulation
 * @param boundary_case (bool) Flag to solve for Part B
 * @param realistic (bool) Flag to toggle how clients choose weights
 * @param detectDeadlock (bool) Flag to solve for Part C
 * @param fixDeadlock (bool) Flag to solve for Part D
 * @param trainerLog (bool) Flag to solve for Part E
 */
int init_shared_gym(int maxCouches, int numTrainers, bool boundaryCase, bool realistic, bool detectDeadlock, bool fixDeadlock, bool trainerLog){
    sem_unlink(SHARED_GYM_SEM_NAME);
    shared_gym_sem = sem_open(SHARED_GYM_SEM_NAME, O_CREAT, 0644, 1);
    if(shared_gym_sem == SEM_FAILED) {
        perror("init_shared_gym sem_failed_to_open");
        exit(1);
    }
    
    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        // FAILSAFE FOR MISHANDLED SHARED MEMORY DEALLOCATION

        system("ipcrm -M 4660");
        sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

        if(sharedMemoryID == -1) {
            perror("Something went wrong allocating the shared memory space");
            return 1;
        }
    }

    // TAKE THE SEMAPHORE
    sem_wait(shared_gym_sem);

    // ATTACH TO SHARED MEMORY
    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        sem_post(shared_gym_sem);
        perror("Could not attached to the shared memory\n");
        return 1;
    }

    // INIT CLIENTS
    for(int i=0; i<MAX_CLIENTS; ++i) {
        sharedGym->arrivingList[i].pid      = -1;
        sharedGym->arrivingList[i].state    = -1;
        sharedGym->waitingList[i].pid       = -1;
        sharedGym->waitingList[i].state     = -1;
        sharedGym->workoutList[i].pid       = -1;
        sharedGym->workoutList[i].state     = -1;
    }

    // INIT TRAINERS
    for(int i=0; i<MAX_TRAINERS; ++i) {
        sharedGym->trainerList[i].client_pid = -1;
        sharedGym->trainerList[i].pid        = -1;
        sharedGym->trainerList[i].state      = FREE;
    }

    // INIT CONSTANTS AND FLAGS
    sharedGym->maxCouches = maxCouches;
    sharedGym->num_trainers = numTrainers;
    sharedGym->boundary_case = boundaryCase;
    sharedGym->realistic = realistic;
    sharedGym->fix_deadlock = fixDeadlock;
    sharedGym->detect_deadlock = detectDeadlock;
    sharedGym->trainer_log = trainerLog;
    sharedGym->unit_time = UNIT_TIME;
    sharedGym->deadlock_victim = -1;


    // DETATCH FROM SHARED MEMORY
    if (shmdt(sharedGym) == -1 && errno != EINVAL){
        sem_post(shared_gym_sem);
        perror("Something happened trying to detach from shared memory\n");
        return 1;
    }
 
    // RELEASE THE SEMAPHORE
    sem_post(shared_gym_sem);
 
    return 0;
}


/**
 * @brief Open the shared memory and semaphore in the current process.
 */
void open_shared_gym(){

    shared_gym_sem = sem_open(SHARED_GYM_SEM_NAME, O_CREAT, 0644, 1);
    if(shared_gym_sem == SEM_FAILED) {
        perror("init_shared_gym sem_failed_to_open");
        exit(1);
    }

    //First get shared object from memory
    int sharedMemoryID;
    int *sharedMemoryAddress;

    sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);

    if (sharedMemoryID == -1){
        //something went wrong here
        perror("Something went wrong allocating the shared memory space\n");
        return;
    }

    // TAKE THE SEMAPHORE
    sem_wait(shared_gym_sem);

    sharedGym = shmat(sharedMemoryID, NULL, 0);

    if (sharedGym == (void *) -1){
        sem_post(shared_gym_sem);
        perror("Could not attached to the shared memory\n");
        return;
    }    

    // RELEASE THE SEMAPHORE
    sem_post(shared_gym_sem);
    return;
}


/**
 * @brief Close the shared memory and semaphore in the current process
 */
void close_shared_gym(){   
    sem_wait(shared_gym_sem);

    // IF ERRNO == EINVAL, MEMORY HAS ALREADY BEEN DETACHED -> WE CAN IGNORE
    if (shmdt(sharedGym) == -1 && errno != EINVAL) {
        perror("Something happened trying to detach from shared memory\n");
        return;
    }      
    sem_post(shared_gym_sem);
    sem_close(shared_gym_sem);
}


/**
 * @brief Free the shared memory and semaphore. Should follow a call to init_share_gym() in the parent process.
 */
void destroy_shared_gym() {
    int sharedMemoryID = shmget(SHARED_KEY, sizeof(SharedGym), IPC_CREAT|0644);
    
    if (shmctl(sharedMemoryID,IPC_RMID,0) == -1) {
        // It's already been closed by another process. Just ignore.
        perror("Something went wrong with the shmctl function\n");
        return;
    }    

    sem_unlink(SHARED_GYM_SEM_NAME);
}




//////////////////////////////
//
// Update shared and local memory
//


/**
 * @brief Copy a locally stored, LL-based gym, to the shared memory space. Will only modify structs with the current process ID
 * @param gym (Gym*) gym to be copied
 */
void update_shared_gym(Gym *gym) {
    //! CANNOT MODIFY OTHER PROCESS STATES

    // SAVE THE POINTER TO THE CURRENT PROCESS, IF IT EXISTS
    Client *client = client_list_find_pid(getpid(), gym->arrivingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->waitingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->workoutList);

    Trainer *trainer = NULL;
    if(client == NULL) trainer = trainer_list_find_pid(getpid(), gym->trainerList);
    

    // SEE IF WE'RE CURRENTLY IN THE LIST
    bool in_list = (client != NULL || trainer != NULL) ? true : false;

    // SEE WHETHER WE'RE A CLIENT OR TRAINER IN THE LIST
    bool is_client = (client == NULL) ? false : true;


    // TAKE THE SEMAPHORE
    sem_wait(shared_gym_sem);

    // Only the parent can set a victim for deadlock rollback
    if(!in_list)  sharedGym->deadlock_victim = gym->deadlock_victim;

    //! CURRENT VICTIM AUTOMATICALLY UNSETS ITSELF WHEN PUSHING TO SHARED MEM
    if(getpid() == sharedGym->deadlock_victim) sharedGym->deadlock_victim = -1;


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
            sharedGym->trainerList[i].pid = -1;
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


/**
 * @brief Copy the shared space, array-based gym, to the local memory space. Will not modify structs with the current process ID
 */
void update_gym(Gym *gym)  {
    //! CANNOT MODIFY CURRENT PROCESS STATE
    // Easiest to just delete everything except current process and start from scratch


    Client *client = client_list_find_pid(getpid(), gym->arrivingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->waitingList);
    if(client == NULL) client = client_list_find_pid(getpid(), gym->workoutList);

    Trainer *trainer = NULL;
    if(client == NULL) trainer = trainer_list_find_pid(getpid(), gym->trainerList);

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

    // UPDATE GYM WITH CONSTANTS & FLAGS FROM SHARED SPACE
    gym->deadlock_victim = sharedGym->deadlock_victim;
    gym->maxCouches = sharedGym->maxCouches;
    gym->unit_time = sharedGym->unit_time;
    gym->boundary_case = sharedGym->boundary_case;
    gym->num_trainers = sharedGym->num_trainers;
    gym->realistic = sharedGym->realistic;
    gym->fix_deadlock = sharedGym->fix_deadlock;
    gym->detect_deadlock = sharedGym->detect_deadlock;
    gym->trainer_log = sharedGym->trainer_log;

    // Update everything with differnet pid
    for(int i=0; i < MAX_CLIENTS; ++i) {
        pid_t tmp_pid = sharedGym->arrivingList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Client *tmp_client = client_init(0, ARRIVING, NULL, NULL, NULL);
            copy_client(tmp_client, &sharedGym->arrivingList[i]);
            client_list_add_client(tmp_client, gym->arrivingList);
        }

        tmp_pid = sharedGym->waitingList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Client *tmp_client = client_init(0, ARRIVING, NULL, NULL, NULL);
            copy_client(tmp_client, &sharedGym->waitingList[i]);
            //printf("pid %d received client w/ pid %d\r\n", getpid(), tmp_client->pid);
            client_list_add_client(tmp_client, gym->waitingList);
        }

        tmp_pid = sharedGym->workoutList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Client *tmp_client = client_init(0, ARRIVING, NULL, NULL, NULL);
            copy_client(tmp_client, &sharedGym->workoutList[i]);
            client_list_add_client(tmp_client, gym->workoutList);
        }
    }

    for(int i=0; i < MAX_TRAINERS; ++i) {
        pid_t tmp_pid = sharedGym->trainerList[i].pid;
        if(tmp_pid != getpid() && tmp_pid != -1) {
            Trainer *tmp_trainer = trainer_init(-1, -1, FREE);
            copy_trainer(tmp_trainer, &sharedGym->trainerList[i]);
            trainer_list_add_trainer(tmp_trainer, gym->trainerList);  
        }
    }  

    // RELEASE SEMAPHORE
    sem_post(shared_gym_sem);


    // update current process
    if(in_list && is_client) {
        switch(client->state) {
            case WAITING:
                //printf("my pid: %d\r\n", getpid());
                client_list_add_client(client, gym->waitingList);
                //client_list_to_string(gym->waitingList, buffer);
                //printf("%s\r\n", buffer);
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



//////////////////////////////
//
// Helper functions
//


/**
 * @brief Copy a client struct from src to dest
 * @param dest (Client*) Destination of copy
 * @param src (Client*) Source of copy
 * @return (Client*) same as dest
 */
Client* copy_client(Client *dest, Client *src) {
    if(dest == NULL || src == NULL) {
        perror("copy_client invalid_argument");
        return NULL;
    }

    *dest = *src;
    return dest;
}


/**
 * @brief Copy a trainer from src to dest
 * @param src (Trainer*) destination of copy
 * @param dest (Trainer*) src of copy
 * @return (Trainer*) same as dest
 */
Trainer* copy_trainer(Trainer *dest, Trainer *src) {
    if(dest == NULL || src == NULL) {
        perror("copy_trainer invalide_argument");
        return NULL;
    }

    *dest = *src;
    return dest;
}


/**
 * @brief Delay in milliseconds
 * @param mS (long) delay time
 */
void delay(long mS) {
    struct timespec ts;

    ts.tv_sec = mS / 1000;
    ts.tv_nsec = (mS % 1000) * 1000000L;

    int ret;
    do {
        ret = nanosleep(&ts, &ts);
    } while(ret == -1 && errno == EINTR);
}