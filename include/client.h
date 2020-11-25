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

struct Trainer;

typedef enum {
    ARRIVING,
    WAITING,
    TRAINING,
    LEAVING
} ClientState;

typedef struct {
    pid_t pid;
    ClientState state;

    struct Trainer* current_trainer;   
    Couch* current_couch;       //TODO Should this just be a semaphore?
    Workout* workout;           // Set by trainer
} Client;

typedef struct ClientNode {
    Client* node;
    struct ClientNode* prev;
    struct ClientNode* next;
} ClientNode;

typedef struct {
    ClientNode *HEAD, *TAIL;

    int len;
} ClientList;


// EACH CLIENT SHOULD BE ON A DIFFERENT THREAD
// - should maintain a finite state machine


// Allocate client on heap. Init params as NULL if unavailable
Client* client_init(pid_t pid, ClientState state, struct Trainer* trainer, Couch *couch, Workout *workout);
int client_del(Client* client);
const char* client_to_string(Client *client, char buffer[]);


ClientList* client_list_init();
int client_list_del(ClientList *list);
int client_list_add_client(Client *client, ClientList* list);
int client_list_rem_client(Client *client, ClientList *list);

const char* client_list_to_string(ClientList *list, char buffer[]);

ClientNode* client_list_srch(Client *client, ClientList *list);

#endif // CLIENT_H