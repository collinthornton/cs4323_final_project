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

typedef enum {
    FREE,
    ON_PHONE,
    WITH_CLIENT,
    TRAVELLING
} TrainerState;

typedef struct {
    TrainerState state;
    Client* current_client;
} Trainer;



// EACH TRAINER SHOULD RUN ON A DIFFERENT THREAD
// - should maintain a finite state machine



#endif // TRAINER_H