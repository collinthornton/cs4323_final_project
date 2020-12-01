// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project trainer struct source
//   Date    -   11-24-20
//
// ########################################## 

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "trainer.h"
#include "workout_room.h"
#include "resource_manager.h"
#include "recordbook.h"
#include "gym.h"


// Define semaphore name
static const char TRAINER_SEM_NAME[] = "/sem_trainer";


// Semaphore to synchronize trainers
static sem_t *trainer_sem;



//////////////////////////////
//
// Trainer process functions
//


/**
 * @brief Spawn a trainer child process. new process will launch trainer_proc_state_machin()
 * @return (pid_t) Process ID of new child process
 */
pid_t trainer_start() {
    pid_t pid = fork();

    if(pid < 0) {
        perror("trainer_start() fork");
        return pid;
    }
    else if(pid == 0) {
        printf("new trainer pid: %d\r\n", getpid());
        int ret = trainer_proc_state_machine();
        exit(ret);
    }
    else {
        return pid;
    }
}


/**
 * @brief Execute the trainer state machine. Should only be run by trianer proccess
 * @return (int) return code. negative on error
 */
int trainer_proc_state_machine() {

    // Set the seed for the random number generator. Used when setting workouts for the client
    srand(getpid());

    open_trainer_sem();
    open_resource_manager();
    openRecordBook();

    int pid = getpid();

    Gym *gym = gym_init();

    open_shared_gym();
    update_gym(gym);

    if(gym == NULL) {
        perror("trainer_proc_state_machine get shared gym");
        return -1;
    }


    // Initialize the trainer struct
    Trainer *trainer = trainer_init(pid, -1, TRAVELLING);
    Client *client;

    // Add ourself to the gym's list of trainers and refresh the list
    trainer_list_add_trainer(trainer, gym->trainerList);
    update_shared_gym(gym);
    update_gym(gym);

    char buffer[BUFFER_SIZE] = "\0";

    bool shutdown = false;

    const int MAX_WAIT = 30;
    int num_wait = 0;


    // START STATE MACHINE
    while(!shutdown) {


        switch(trainer->state) {
            case FREE:
                printf("TRAINER %d FREE %d of %d CLIENTS IN WAITING ROOM\r\n", pid, gym->waitingList->len, gym->maxCouches);

                sem_wait(trainer_sem);
                int val;
                sem_getvalue(trainer_sem, &val);

                update_gym(gym);

                if(gym->waitingList->len > 0) {
                    ClientNode *node = gym->waitingList->HEAD;
                    while(node != NULL) {
                        // Check if client is already claimed by a trainer
                        Trainer *tmp = trainer_list_find_client(node->node->pid, gym->trainerList);
                        if(tmp == NULL) break;
                        node = node->next;
                    }
                    if(node != NULL) {
                        trainer->state = WITH_CLIENT;
                        trainer->client_pid = node->node->pid;
                    }
                }
                else {
                    trainer->state = ON_PHONE;
                }

                update_shared_gym(gym);

                sem_post(trainer_sem);

                delay(2*gym->unit_time);
                break;

            case ON_PHONE:
                // CHECK IF WE'VE BEEN CLAIMED BY A CLIENT
                printf("TRAINER %d ON PHONE\r\n", pid);

                client = client_list_find_trainer(getpid(), gym->arrivingList);
                
                if(client != NULL) {
                    trainer->client_pid = client->pid;
                    trainer->state = WITH_CLIENT;
                    update_shared_gym(gym);
                    num_wait = 0;
                }
                else {
                    ++num_wait;
                    if(num_wait == MAX_WAIT) shutdown = true;
                }

                delay(2*gym->unit_time);

                break;

            case TRAVELLING:
                printf("TRAINER %d TRAVELLING\r\n", pid);
                trainer->client_pid = -1;
                
                delay(3*gym->unit_time);
                trainer->state = FREE;

                break;

            case WITH_CLIENT:
                printf("TRAINER %d WITH_CLIENT %d\r\n", pid, trainer->client_pid);
                delay(2*gym->unit_time);
                trainer_workout_event(gym, trainer);
                trainer->state = TRAVELLING;
                
                break;
        }

        update_shared_gym(gym);
        update_gym(gym);
    }

    // Remove trainer from lists & destroy semaphores

    printf("Trainer %d destroying data\r\n", getpid());

    trainer_list_rem_trainer(trainer, gym->trainerList);
    update_shared_gym(gym);

    trainer_del(trainer);
    gym_del(gym);

    close_shared_gym();
    close_resource_manager();
    close_trainer_sem();
    closeRecordBook();

    printf("Trainer %d exiting\r\n", getpid());

    return 0;
}



//////////////////////////////
//
// Sempahore handling
//

/**
 * @brief Initalize the trainer semaphore. Should be called on the parent process.
 * @return (int) return code. negative on failure.
 */
int init_trainer_sem() {
    sem_unlink(TRAINER_SEM_NAME);
    trainer_sem = sem_open(TRAINER_SEM_NAME, O_CREAT, 0644, 1);
    if(trainer_sem == SEM_FAILED) {
        perror("trainer_sem_init failed to open");
        exit(1);
    }
}


/**
 * @brief Open the trainer sempaahore. SHould be called on the trainer process
 * @return (int) return code. negative on error
 */
int open_trainer_sem() {
    trainer_sem = sem_open(TRAINER_SEM_NAME, O_CREAT, 0644, 1);
    if(trainer_sem == SEM_FAILED) {
        perror("trainer_sem_init failed to open");
        exit(1);
    }
    return 0;
}


/**
 * @brief close the trainer semaphore. should be called after open_trainer_sem()
 */
void close_trainer_sem() {
    sem_close(trainer_sem);
    return;
}


/**
 * @brief free the trainer semaphore. should be called after init_trainer_sem() on parent
 */
void destroy_trainer_sem() {
    sem_unlink(TRAINER_SEM_NAME);
    return;
}


////

//////////////////////////
//
// Trainer struct functions
//


/**
 * @brief Allocate trainer on heap. Init params as NULL or negative if unavailable
 * @param pid (pid_t) Process ID of trainer
 * @param client_pid (pid_t) Process ID of client. -1 if unavailable
 * @param state (TrainerState) Inital state of trainer
 * @return (Trainer*) pointer to a trainer struct
 */
Trainer* trainer_init(pid_t pid, pid_t client_pid, TrainerState state) {
    Trainer* trainer = (Trainer*)malloc(sizeof(Trainer));

    if(trainer == NULL) {
        perror("trainer_init malloc()");
        return NULL;
    }

    trainer->pid = pid;
    trainer->state = state;
    trainer->client_pid = client_pid;
    
    Workout *tmp = workout_init(-1, -1, -1, NULL);
    trainer->workout = *tmp;
    workout_del(tmp);

    return trainer;
}


/**
 * @brief Delete a trainer from the heap
 * @param trainer (Trainer*) trainer to be deleted
 * @return (int)return code. negative on erro
 */
int trainer_del(Trainer* trainer) {   
    if(trainer == NULL) return -1;

    free(trainer);
    return 0;
}


/**
 * @brief Stringify a trainer struct
 * @param trainer (Traienr*) struct to be stringified
 * @param buffer (char[]) buffer to store new string
 * @return (const char*) same as buffer
 */
const char* trainer_to_string(Trainer *trainer, char buffer[]) {
    if(trainer == NULL) return NULL;

    sprintf(buffer, "pid: %d", trainer->pid);
    sprintf(buffer + strlen(buffer), "   state: %d", trainer->state);
    sprintf(buffer + strlen(buffer), "   client: %d", trainer->client_pid);
    return buffer;
}



//////////////////////////////
//
// Trainer list functions
//


/**
 * @brief Initalize a trainer LL on the heap
 * @return (TrainerList*) pointer to newly allocated LL
 */
TrainerList* trainer_list_init() {
    TrainerList* list = (TrainerList*)malloc(sizeof(TrainerList));

    if(list == NULL) {
        perror("trainer_list_init malloc()");
        return NULL;
    }

    list->HEAD = NULL;
    list->TAIL = NULL;
    list->len = 0;
    return list;
}


/**
 * @brief Free a trainer list and all internal nodes. Does not free the individual trainers
 * @param list (TrainerList*) Pointer to the current shared list
 * @return (int) return code. negative on error
 */
int trainer_list_del_trainers(pid_t exclude, TrainerList *list) {
    if(list == NULL) return 0;

    TrainerNode *tmp = list->HEAD;

    while(tmp != NULL) {
        if(tmp->node->pid != exclude) trainer_del(tmp->node);
        tmp = tmp->next;
    }
    return 0;
}


/**
 * @brief Delete trainers from a given list
 * @param exclude (pid_t) Process ID of exclude trainer
 * @param list (TrainerList*) List of current trainers
 * @return (int) return code. negative on error
 */
int trainer_list_del(TrainerList *list) {
    if(list == NULL) return 0;

    TrainerNode *tmp = list->HEAD;

    while(tmp != NULL) {
        TrainerNode *next = tmp->next;
        free(tmp);
        tmp = next;
    }

    free(list);

    return 0;
}

/**
 * @brief Add a trainer to a list
 * @param trainer (Trianer*) trainer to be added
 * @param list (TrainerList*) target list
 * @return (int) return code. negative on error
 */
int trainer_list_add_trainer(Trainer *trainer, TrainerList *list) {
    TrainerNode *new_node = (TrainerNode*)malloc(sizeof(TrainerNode));

    if(new_node == NULL) {
        perror("trainer_list_add_trainer malloc()");
        return -1;
    }

    if(trainer_list_find_pid(trainer->pid, list) != NULL) {
        free(new_node);
        return 1;
    }

    new_node->node = trainer;
    new_node->next = NULL;
    new_node->prev = list->TAIL;

    if(list->HEAD == NULL) {
        list->HEAD = new_node;
        list->TAIL = new_node;
    }
    else {
        list->TAIL->next = new_node;
        list->TAIL = new_node;
    }

    ++list->len;
    return list->len;
}


/**
 * @brief Remove a trainer from the list
 * @param trainer (Trainer*) trainer to be removed
 * @param list (TrainerList*) list from which to remove the trainer
 * @return (int) return code. negative on error.
 */
int trainer_list_rem_trainer(Trainer *trainer, TrainerList *list) {
    if(list == NULL || trainer == NULL) {
        perror("trainer_list_rem_trainer() invalid_argument");
        return -1;
    }

    TrainerNode *tmp = trainer_list_srch(trainer, list);

    if(tmp == NULL) return list->len; 

    if(tmp == list->HEAD && tmp == list->TAIL) {
        free(tmp);
        list->HEAD = NULL;
        list->TAIL = NULL;
    }
    else if(tmp == list->HEAD) {
        TrainerNode *new_head = list->HEAD->next;
        free(tmp);
        list->HEAD = new_head;
        new_head->prev = NULL;
    }
    else if(tmp == list->TAIL) {
        TrainerNode *new_tail = list->TAIL->prev;
        free(tmp);
        list->TAIL = new_tail;
        new_tail->next = NULL;
    }
    else {
        TrainerNode *prevNode = tmp->prev;
        TrainerNode *nextNode = tmp->next;
        free(tmp);
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }

    --list->len;
    return list->len;
}


/**
 * @brief Stringify a trainer list
 * @param list (TrainerList*) list to be stringified
 * @param buffer (char[]) buffer to store new string
 * @return (const char*) same as buffer
 */
const char* trainer_list_to_string(TrainerList* list, char buffer[]) {
    if(list == NULL) return NULL;

    if(list->HEAD == NULL) {
        sprintf(buffer, "EMPTY\n");
        return buffer;
    }

    TrainerNode *tmp = list->HEAD;
    char buff[1024];
    trainer_to_string(tmp->node, buff);
    sprintf(buffer, "%s   HEAD", buff);
    tmp = tmp->next;

    while(tmp != NULL) {
        char buff[1024];
        trainer_to_string(tmp->node, buff);
        sprintf(buffer + strlen(buffer), "\n%s", buff);
        tmp = tmp->next;
    }
    sprintf(buffer + strlen(buffer), "   TAIL\n");
    return buffer;
}


/**
 * @brief Search for a client in trainer LL
 * @param client_pid (pid_t) needle -> pid of client in question
 * @param list (TrainerList*) poitner to LL with client
 * @return (Trainer*) null if not found, otherwise pointer to the trainer
 */
Trainer* trainer_list_find_client(pid_t client_pid, TrainerList *list) {
    if(list == NULL || client_pid < 0) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->client_pid == client_pid) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}


/**
 * @brief Find a trainer currently on their phone
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_phone(TrainerList *list) {
    return trainer_list_find_state(ON_PHONE, list);
}


/**
 * @brief Find a trainer currently on their available
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_available(TrainerList *list) {
    return trainer_list_find_state(FREE, list);
}


/**
 * @brief Find a trainer currently at a given state
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_state(TrainerState state, TrainerList *list) {
    if(list == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->state == state) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;    
}


/**
 * @brief Find a trainer currently with pid in LL
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_pid(pid_t pid, TrainerList *list) {
    if(list == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->pid == pid) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}


/**
 * @brief Search a trainer LL for a given trainer
 * @param trainer (Trainer*) needle
 * @param list (TrainerLIst*) haystack
 * @return (TrainerNode*) pointer to node containing the Trainer struct
 */
TrainerNode* trainer_list_srch(Trainer *trainer, TrainerList *list) {
    if (trainer == NULL || list == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node == trainer) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}




void test_trainer_list() {
    printf("\r\n");

    Trainer *trainer_one = trainer_init(1, 0, FREE);
    Trainer *trainer_two = trainer_init(2, 0, FREE);
    Trainer *trainer_three = trainer_init(3, 0, FREE);

    TrainerList *trainer_list = trainer_list_init();
    trainer_list_add_trainer(trainer_one, trainer_list);
    trainer_list_add_trainer(trainer_two, trainer_list);
    trainer_list_add_trainer(trainer_three, trainer_list);

    int buffer_size = 1024*trainer_list->len;
    char buffer[buffer_size];
    trainer_list_to_string(trainer_list, buffer);

    printf("INITIAL LIST: \r\n%s\r\n", buffer);

    trainer_list_rem_trainer(trainer_one, trainer_list);
    trainer_list_to_string(trainer_list, buffer);

    printf("TRAINER ONE REMOVED: \r\n%s\r\n", buffer);

    trainer_list_rem_trainer(trainer_two, trainer_list);
    trainer_list_to_string(trainer_list, buffer);

    printf("TRAINER TWO REMOVED:\r\n%s\r\n", buffer);

    trainer_list_rem_trainer(trainer_three, trainer_list);
    trainer_list_to_string(trainer_list, buffer);

    printf("TRAINER THREE REMOVED\r\n%s\r\n", buffer);
    
    trainer_del(trainer_one);
    trainer_del(trainer_two);
    trainer_del(trainer_three);
    trainer_list_del(trainer_list);
}