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

sem_t *trainer_sem;

typedef enum {
    FREE,
    ON_PHONE,
    TRAVELLING,
    WITH_CLIENT
} TrainerState;

typedef struct Trainer {
    pid_t pid;
    pid_t client_pid;
    TrainerState state;
    Workout workout;
} Trainer;

typedef struct TrainerNode {
    Trainer* node;
    struct TrainerNode* prev;
    struct TrainerNode* next;
} TrainerNode;

typedef struct {
    TrainerNode *HEAD, *TAIL;

    int len;
} TrainerList;


// EACH CLIENT SHOULD BE ON A DIFFERENT PROCESS
// - should maintain a finite state machine
int init_trainer_sem();
int open_trainer_sem();
void close_trainer_sem();
void destroy_trainer_sem();

pid_t trainer_start();
int trainer_proc_state_machine();



// Allocate trainer on heap. Init params as NULL if unavailable
Trainer* trainer_init(pid_t pid, pid_t client_pid, TrainerState state);
int trainer_del(Trainer* trainer);
const char* trainer_to_string(Trainer *trainer, char buffer[]);


TrainerList* trainer_list_init();
int trainer_list_del(TrainerList *list);
int trainer_list_del_trainers(pid_t exclude, TrainerList *list);
int trainer_list_add_trainer(Trainer *trainer, TrainerList* list);
int trainer_list_rem_trainer(Trainer *trainer, TrainerList *list);

Trainer* trainer_list_find_client(pid_t client_pid, TrainerList *list);
Trainer* trainer_list_find_available(TrainerList *list);
Trainer* trainer_list_find_phone(TrainerList *list);
Trainer* trainer_list_find_pid(pid_t pid, TrainerList *list);

const char* trainer_list_to_string(TrainerList *list, char buffer[]);

TrainerNode* trainer_list_srch(Trainer *trainer, TrainerList *list);


void test_trainer_list(void);

#endif // TRAINER_H