// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Assignment 02 Client include
//   Date    -   11-15-20
//
// ########################################## 


#ifndef CLIENT_H
#define CLIENT_H

#include "trainer.h"
#include "workout.h"
#include "gym.h"

typedef enum {
    ARRIVING,
    WAITING,
    TRAINING,
    LEAVING
} ClientState;

typedef struct {
    ClientState state;

    Trainer* current_trainer;   
    Couch* current_couch;       //TODO Should this just be a semaphore?
    Workout* workout;           // Set by trainer
} Client;


typedef struct {
    Client* clients;

    int size = 0;
    int allocated = 0;
} ClientList;


// EACH CLIENT SHOULD BE ON A DIFFERENT THREAD
// - should maintain a finite state machine



#endif // CLIENT_H