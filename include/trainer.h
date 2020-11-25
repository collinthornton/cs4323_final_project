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
    TrainerState state;
    struct Client* current_client;
} Trainer;



// EACH TRAINER SHOULD RUN ON A DIFFERENT THREAD
// - should maintain a finite state machine


Trainer *trainer_init(TrainerState state, struct Client *client);
int trainer_del(Trainer *trainer);

#endif // TRAINER_H