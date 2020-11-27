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

#define MAX_TRAINERS 4

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




// Allocate trainer on heap. Init params as NULL if unavailable
Trainer* trainer_init(pid_t pid, pid_t client_pid, TrainerState state);
int trainer_del(Trainer* trainer);
const char* trainer_to_string(Trainer *trainer, char buffer[]);


TrainerList* trainer_list_init();
int trainer_list_del(TrainerList *list);
int trainer_list_del_trainers(TrainerList *list);
int trainer_list_add_trainer(Trainer *trainer, TrainerList* list);
int trainer_list_rem_trainer(Trainer *trainer, TrainerList *list);

Trainer* trainer_list_find_client(pid_t client_pid, TrainerList *list);
Trainer* trainer_list_find_available(TrainerList *list);
Trainer* trainer_list_find_phone(TrainerList *list);

const char* trainer_list_to_string(TrainerList *list, char buffer[]);

TrainerNode* trainer_list_srch(Trainer *trainer, TrainerList *list);


void test_trainer_list(void);

#endif // TRAINER_H
