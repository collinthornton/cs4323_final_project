// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project Trainer include
//   Date    -   11-15-20
//
// ########################################## 

#ifndef TRAINER_H
#define TRAINER_H

#include <stdlib.h>
#include <sys/wait.h>

#include "workout.h"

#define MIN_TRAINERS 3
#define MAX_TRAINERS 5


typedef enum {
    FREE,
    ON_PHONE,
    TRAVELLING,
    WITH_CLIENT
} TrainerState;

/**
 * @brief Maintain information relative to a specific trainer
 */
typedef struct Trainer {
    pid_t pid;
    pid_t client_pid;
    TrainerState state;
    Workout workout;
} Trainer;

/**
 * @brief Node for a linked list of trainers
 */
typedef struct TrainerNode {
    Trainer* node;
    struct TrainerNode* prev;
    struct TrainerNode* next;
} TrainerNode;

/**
 * @brief Linked list of trainers
 */
typedef struct {
    TrainerNode *HEAD, *TAIL;

    int len;
} TrainerList;


// EACH CLIENT SHOULD BE ON A DIFFERENT PROCESS
// - should maintain a finite state machine

//////////////////////////////
//
// Semaphore handling
//


/**
 * @brief Initalize the trainer semaphore. Should be called on the parent process.
 * @return (int) return code. negative on failure.
 */
int init_trainer_sem();


/**
 * @brief Open the trainer sempaahore. SHould be called on the trainer process
 * @return (int) return code. negative on error
 */
int open_trainer_sem();


/**
 * @brief close the trainer semaphore. should be called after open_trainer_sem()
 */
void close_trainer_sem();


/**
 * @brief free the trainer semaphore. should be called after init_trainer_sem() on parent
 */
void destroy_trainer_sem();



//////////////////////////////
//
// Trainer process
//


/**
 * @brief Spawn a trainer child process. new process will launch trainer_proc_state_machin()
 * @return (pid_t) Process ID of new child process
 */
pid_t trainer_start();


/**
 * @brief Execute the trainer state machine. Should only be run by trianer proccess
 * @return (int) return code. negative on error
 */
int trainer_proc_state_machine();




//////////////////////////////
//
// Client struct
//

/**
 * @brief Allocate trainer on heap. Init params as NULL or negative if unavailable
 * @param pid (pid_t) Process ID of trainer
 * @param client_pid (pid_t) Process ID of client. -1 if unavailable
 * @param state (TrainerState) Inital state of trainer
 * @return (Trainer*) pointer to a trainer struct
 */
Trainer* trainer_init(pid_t pid, pid_t client_pid, TrainerState state);


/**
 * @brief Delete a trainer from the heap
 * @param trainer (Trainer*) trainer to be deleted
 * @return (int)return code. negative on erro
 */
int trainer_del(Trainer* trainer);


/**
 * @brief Stringify a trainer struct
 * @param trainer (Traienr*) struct to be stringified
 * @param buffer (char[]) buffer to store new string
 * @return (const char*) same as buffer
 */
const char* trainer_to_string(Trainer *trainer, char buffer[]);


//////////////////////////////
//
// Trainer list
//

/**
 * @brief Initalize a trainer LL on the heap
 * @return (TrainerList*) pointer to newly allocated LL
 */
TrainerList* trainer_list_init();


/**
 * @brief Free a trainer list and all internal nodes. Does not free the individual trainers
 * @param list (TrainerList*) Pointer to the current shared list
 * @return (int) return code. negative on error
 */
int trainer_list_del(TrainerList *list);


/**
 * @brief Delete trainers from a given list
 * @param exclude (pid_t) Process ID of exclude trainer
 * @param list (TrainerList*) List of current trainers
 * @return (int) return code. negative on error
 */
int trainer_list_del_trainers(pid_t exclude, TrainerList *list);


/**
 * @brief Add a trainer to a list
 * @param trainer (Trianer*) trainer to be added
 * @param list (TrainerList*) target list
 * @return (int) return code. negative on error
 */
int trainer_list_add_trainer(Trainer *trainer, TrainerList* list);


/**
 * @brief Remove a trainer from the list
 * @param trainer (Trainer*) trainer to be removed
 * @param list (TrainerList*) list from which to remove the trainer
 * @return (int) return code. negative on error.
 */
int trainer_list_rem_trainer(Trainer *trainer, TrainerList *list);


/**
 * @brief Search for a client in trainer LL
 * @param client_pid (pid_t) needle -> pid of client in question
 * @param list (TrainerList*) poitner to LL with client
 * @return (Trainer*) null if not found, otherwise pointer to the trainer
 */
Trainer* trainer_list_find_client(pid_t client_pid, TrainerList *list);


/**
 * @brief Find a trainer currently on their available
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_available(TrainerList *list);


/**
 * @brief Find a trainer currently on their phone
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_phone(TrainerList *list);


/**
 * @brief Find a trainer currently at a given state
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_state(TrainerState state, TrainerList *list);


/**
 * @brief Find a trainer currently with pid in LL
 * @param list (TrainerList*) list to be searched
 * @return (Trainer*) null if not found, othewise poitner to the trainer
 */
Trainer* trainer_list_find_pid(pid_t pid, TrainerList *list);


/**
 * @brief Stringify a trainer list
 * @param list (TrainerList*) list to be stringified
 * @param buffer (char[]) buffer to store new string
 * @return (const char*) same as buffer
 */
const char* trainer_list_to_string(TrainerList *list, char buffer[]);


/**
 * @brief Search a trainer LL for a given trainer
 * @param trainer (Trainer*) needle
 * @param list (TrainerLIst*) haystack
 * @return (TrainerNode*) pointer to node containing the Trainer struct
 */
TrainerNode* trainer_list_srch(Trainer *trainer, TrainerList *list);


void test_trainer_list(void);

#endif // TRAINER_H