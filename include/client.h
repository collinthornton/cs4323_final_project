// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project Client include
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
    Client* node;
    Client* prev;
    Client* next;
} ClientNode;

typedef struct {
    ClientNode* HEAD, TAIL;
    ClientNode* node;

    int size;
    int allocated;
} ClientList;


// EACH CLIENT SHOULD BE ON A DIFFERENT THREAD
// - should maintain a finite state machine


// Allocate client on heap. Init params as NULL if unavailable
Client* client_init(ClientState state, Trainer* trainer, Couch *couch, Workout *workout);

ClientList* client_init_list();
int client_add_to_list(Client *client, ClientList* list);
int client_rem_from_list(Client *client, ClientList *list);

int client_del_list(ClientList *list);
int client_del_client(Client *client);


#endif // CLIENT_H