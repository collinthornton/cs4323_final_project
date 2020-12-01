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
#include "gym_resources.h"

#define MAX_CLIENTS 10

typedef enum {
    ARRIVING,
    WAITING,
    MOVING,
    TRAINING,
    LEAVING
} ClientState;

/**
 * @brief Maintain information relative to a specific client
 */
typedef struct Client {
    pid_t pid;
    ClientState state;

    Trainer current_trainer;   
    Couch current_couch;       //TODO Should this just be a semaphore?
    Workout workout;           // Set by trainer
} Client;

/**
 * @brief Node for a linked list of clients
 */
typedef struct ClientNode {
    Client* node;
    struct ClientNode* prev;
    struct ClientNode* next;
} ClientNode;

/**
 * @brief Linked list of clients
 */
typedef struct {
    ClientNode *HEAD, *TAIL;

    int len;
} ClientList;


// EACH CLIENT SHOULD BE ON A DIFFERENT THREAD
// - should maintain a finite state machine

//////////////////////////////
//
// Semaphore handling
//

/**
 * @brief Inititialize the client semaphore. Should be called on the parent process
 * @return (int) return code. Negative on failure
 */
int init_client_sem();

/**
 * @brief Open the client semaphore. Should be called on the client processes
 * @return (int) negative on failure
 */
int open_client_sem();

/**
 * @brief close the client semaphore. Should be called after open_client_sem()
 */
void close_client_sem();

/**
 * @brief free the client semaphore. Should be called after init_client_sem() on parent 
 */
void destroy_client_sem();




//////////////////////////////
//
// Client process
//

/**
 * @brief Spawn a client child process. New process will launch client_proc_state_machine()
 * @return (pid_t) Process ID of new child process
 */
pid_t client_start();

/**
 * @brief Execute the client state machine. Should be run by client process
 * @return (int) return code. negative on error
 */
int client_proc_state_machine();



//////////////////////////////
//
// Client struct
//

/**
 * @brief Initialize a client struct on the heap
 * @param pid (pid_t) Process ID of client
 * @param state (ClientState) Initial state of client
 * @param trainer (Trainer*) Current trainer (NULL if none)
 * @param couch (Couch*) Current couch of client (NULL if none)
 * @param worktout (Workout*) Current workout of client (NULL if none)
 * @return (Client*) Struct initialized on heap
 */
Client* client_init(pid_t pid, ClientState state, Trainer* trainer, Couch *couch, Workout *workout);

/**
 * @brief Free a client struct from the heap
 * @param client (Client*) struct to be deleted
 * @return (int) return code. negative on error
 */
int client_del(Client* client);

/**
 * @brief stringify a client struct
 * @param client (Client*) struct to be stringified
 * @param buffer (char[]) string buffer
 * @return (const char*) same as buffer
 */
const char* client_to_string(Client *client, char buffer[]);


//////////////////////////////
//
// Client list
//

/**
 * @brief Initalize a client LL on the heap
 * @return (ClientList*) newly allocated LL
 */
ClientList* client_list_init();

/**
 * @brief Delete a client LL from the heap
 * @param list (ClientList*) list to be deleted
 * @return (int) return code. negative on error
 */
int client_list_del(ClientList *list);

/**
 * @brief Delete clients from LL. Will free the client structs
 * @param exclude (pid_t) Client PID to exclude from deletion
 * @param list (ClientList*) LL from which to delete clients
 * @return (int) return code. negative on error
 */
int client_list_del_clients(pid_t exclude, ClientList *list);

/**
 * @brief Add a client to the LL
 * @param client (Client*) client to be added
 * @param list (ClientList*) list to which client is added
 * @return (int) return code. negative on error
 */
int client_list_add_client(Client *client, ClientList* list);

/**
 * @brief Remove a client from an LL
 * @param client (Client*) client to be removed
 * @param list (ClientList*) list from which to remove client
 * @return (int) return code. negative on error
 */
int client_list_rem_client(Client *client, ClientList *list);

/**
 * @brief stringify a client LL
 * @param list (ClientList*) LL to be stringified
 * @param bufer (char[]) buffer to store new string
 * @return (const char*) same as buffer
 */
const char* client_list_to_string(ClientList *list, char buffer[]);


/**
 * @brief search a client LL for a given client
 * @param client (Client*) needle
 * @param list (ClientList*) haystack
 * @return (ClientNode*) the node containing the requested client. NULL if not found
 */
ClientNode* client_list_srch(Client *client, ClientList *list);

/**
 * @brief search a client LL for a given client
 * @param pid (pid_t) needle
 * @param list (ClientList*) haystack
 * @return (Client*) pointer to client struct. NULL if not found
 */
Client* client_list_find_pid(pid_t pid, ClientList *list);

/**
 * @brief search a client LL for a paired trainer (client->current_trainer.pid)
 * @param pid (pid_t) needle -> pid of trainer
 * @param list (ClientList*) haystack
 * @return (Client*) poitner to client struct with specified trainer. NULL if not found
 */ 
Client* client_list_find_trainer(pid_t pid, ClientList *list);


void test_client_list();

#endif // CLIENT_H