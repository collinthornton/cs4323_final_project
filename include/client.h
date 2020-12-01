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

#include "client_trainer.h"
#include "trainer.h"
#include "workout.h"
#include "gym_resources.h"

#define MAX_CLIENTS 10

sem_t *client_arriving_sem;
sem_t *client_waiting_sem;

typedef enum {
    ARRIVING,
    WAITING,
    MOVING,
    TRAINING,
    LEAVING
} ClientState;

struct Client {
    pid_t pid;
    ClientState state;

    Trainer current_trainer;   
    Couch current_couch;       //TODO Should this just be a semaphore?
    Workout workout;           // Set by trainer
};

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
int init_client_sem();
int open_client_sem();
void close_client_sem();
void destroy_client_sem();


pid_t client_start();
int client_proc_state_machine();



// Allocate client struct on heap. Init params as NULL if unavailable
Client* client_init(pid_t pid, ClientState state, Trainer* trainer, Couch *couch, Workout *workout);
int client_del(Client* client);
const char* client_to_string(Client *client, char buffer[]);


ClientList* client_list_init();
int client_list_del(ClientList *list);
int client_list_del_clients(pid_t exclude, ClientList *list);
int client_list_add_client(Client *client, ClientList* list);
int client_list_rem_client(Client *client, ClientList *list);

const char* client_list_to_string(ClientList *list, char buffer[]);

ClientNode* client_list_srch(Client *client, ClientList *list);
Client* client_list_find_pid(pid_t pid, ClientList *list);
Client* client_list_find_trainer(pid_t pid, ClientList *list);


void test_client_list();

#endif // CLIENT_H