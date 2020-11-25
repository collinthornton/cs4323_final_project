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

#include "client.h"

struct Client;

typedef enum {
    FREE,
    ON_PHONE,
    WITH_CLIENT,
    TRAVELLING
} TrainerState;

typedef struct {
    pid_t pid;
    TrainerState state;
    struct Client* current_client;
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
Trainer* trainer_init(pid_t pid, TrainerState state, struct Client* client);
int trainer_del(Trainer* trainer);
const char* trainer_to_string(Trainer *trainer, char buffer[]);


TrainerList* trainer_list_init();
int trainer_list_del(TrainerList *list);
int trainer_list_add_trainer(Trainer *trainer, TrainerList* list);
int trainer_list_rem_trainer(Trainer *trainer, TrainerList *list);

const char* trainer_list_to_string(TrainerList *list, char buffer[]);

TrainerNode* trainer_list_srch(Trainer *trainer, TrainerList *list);

#endif // TRAINER_H