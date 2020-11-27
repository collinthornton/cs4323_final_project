// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project client struct source
//   Date    -   11-16-20
//
// ########################################## 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

#include "client.h"
#include "gym.h"
#include "entrance.h"


//////////////////////////////
//
// Client process functions
//

pid_t client_start() {
    pid_t pid = fork();
    
    if(pid < 0) {
        perror("client_start() fork");
        return pid;
    }
    else if(pid == 0) {
        printf("child pid: %d\r\n", getpid());
        int ret = client_proc_state_machine();
        exit(ret);
    } else {
        return pid;
    }
}

int client_proc_state_machine() {

    int pid = getpid();

    Gym *gym = gym_init();
    open_shared_gym();
    update_gym(gym);

    if(gym == NULL) {
        perror("client_proc_state_machine() get_shared_gym");
        return -1;
    }


    Client *client = client_init(pid, ARRIVING, NULL, NULL, NULL);

    bool shutdown = false;


    while(!shutdown) {
        // Execute state machine

        switch(client->state) {
            case ARRIVING:
                client_list_add_client(client, gym->arrivingList);
                Workout *workout = workout_init(5, 5, 5, NULL);
                Trainer *trainer = trainer_init(getpid()+1, client->pid, WITH_CLIENT);
                client->workout = *workout;
                client->current_trainer = *trainer;
                trainer_del(trainer);
                workout_del(workout);

                update_shared_gym(gym);

                char buffer[BUFFER_SIZE] = "\0";
                update_gym(gym);
                client = client_list_find_pid(pid, gym->arrivingList);
                client_list_to_string(gym->arrivingList, buffer);
                
                //printf("\r\n CHILD ARRIVING LIST (%d) \r\n%s\r\n", getpid(), buffer);
                //printf("child trainer pid -> %d\r\n\r\n", gym->arrivingList->HEAD->node->current_trainer.pid);
                
                client->state = LEAVING;
                sleep(5); 
                //client_arriving_event(gym, client);               
                break;

            case WAITING:
                printf("PARENT CHANGED ME TO WAITING\r\n");
                sleep(1);
                client->state = LEAVING;
                update_shared_gym(gym);
                //Now check if there is room on the couches

                // client_waiting_event(gym, client);
                
                /*if(gym->waitingList->len < gym->maxCouches){
                    //Another semaphore areas
                    client->state = WAITING;
                    client_list_add_client(client, gym->waitingList);
                    //gym->maxCouches++;
                    //End the semaphore
                } else {
                    //No couches available, no trainers available, time to leave
                    client->state = LEAVING;
                } */
                break;

            case MOVING:
                    //This is the "traveling" piece, also a semaphore area
                    //sleep(2*gym->unit_time);
                    //End the semaphore
                    client->state = WAITING;
                    update_shared_gym(gym);
                break;

            case TRAINING:

                break;

            case LEAVING:
                shutdown = true;

                // DO OTHER THINGS
                break;
        }


        // Update shared memory
        update_gym(gym);

        client = client_list_find_pid(pid, gym->arrivingList);
        if(client == NULL) client = client_list_find_pid(pid, gym->waitingList);
        if(client == NULL) client = client_list_find_pid(pid, gym->workoutList);
        if(client == NULL) {
            perror("client_proc_state_machine client not found");
            gym_del(gym);
            close_shared_gym();
            return -1;
        }
    }

    //client_del(client);
    gym_del(gym);
    close_shared_gym();
    // Remove client from any lists
    // Exit cleanly

    return 0;
}






//////////////////////////////
//
// Client struct functions
//


Client* client_init(pid_t pid, ClientState state, Trainer* trainer, Couch* couch, Workout* workout) {
    Client* client = (Client*)malloc(sizeof(Client));

    if(client == NULL) {
        perror("client_init malloc()");
        return NULL;
    }

    client->pid = pid;
    client->state = state;
    
    if(trainer != NULL)
        client->current_trainer = *trainer;
    
    else
    {
        Trainer *tmp = trainer_init(-1, -1, FREE);
        client->current_trainer = *tmp;
        trainer_del(tmp);
    }
    
    if(couch != NULL)
        client->current_couch = *couch;
    
    if(workout != NULL)
        client->workout = *workout;
    
    else
    {
        Workout *tmp = workout_init(-1, -1 -1, -1, NULL);
        client->workout = *tmp;
        workout_del(tmp);
    }
    
    return client;
}

int client_del(Client* client) {   
    if(client == NULL) return -1;

    free(client);
    return 0;
}

const char* client_to_string(Client *client, char buffer[]) {
    if(client == NULL) return NULL;

    sprintf(buffer, "pid: %d", client->pid);
    sprintf(buffer + strlen(buffer), "   state: %d", client->state);

    //if(client->current_trainer != NULL)
        sprintf(buffer + strlen(buffer), "   trainer: %d", client->current_trainer.pid);

    //if(client->current_couch != NULL) {
        int sem_val;
        sprintf(buffer + strlen(buffer), "   couch: %d", sem_getvalue(&client->current_couch.couch_mutex, &sem_val));
    //}

    //if(client->workout != NULL)
        sprintf(buffer + strlen(buffer), "   workout: %d", client->workout.total_weight);

    return buffer;
}





//////////////////////////////
//
// Client list functions
//

ClientList* client_list_init() {
    ClientList* list = (ClientList*)malloc(sizeof(ClientList));

    if(list == NULL) {
        perror("client_list_init malloc()");
        return NULL;
    }

    list->HEAD = NULL;
    list->TAIL = NULL;
    list->len = 0;
    return list;
}

int client_list_del_clients(ClientList *list) {
    if(list == NULL) return 0;

    ClientNode *tmp = list->HEAD;

    while(tmp != NULL) {
        client_del(tmp->node);
        tmp = tmp->next;
    }
    return 0;
}

int client_list_del(ClientList *list) {
    if(list == NULL) return 0;

    ClientNode *tmp = list->HEAD;

    while(tmp != NULL) {
        ClientNode *next = tmp->next;
        free(tmp);
        tmp = next;
    }

    free(list);

    return 0;
}

int client_list_add_client(Client *client, ClientList *list) {
    ClientNode *new_node = (ClientNode*)malloc(sizeof(ClientNode));

    if(new_node == NULL) {
        perror("client_list_add_client malloc()");
        return -1;
    }

    new_node->node = client;
    new_node->next = NULL;
    new_node->prev = list->TAIL;

    if(list->HEAD == NULL) {
        list->HEAD = new_node;
        list->TAIL = new_node;
    }
    else {
        list->TAIL->next = new_node;
        list->TAIL = new_node;
    }

    ++list->len;
    return list->len;
}

int client_list_rem_client(Client *client, ClientList *list) {
    if(list == NULL || client == NULL) {
        perror("client_list_rem_client() invalid_argument");
        return -1;
    }

    ClientNode *tmp = client_list_srch(client, list);

    if(tmp == NULL) return list->len; 

    if(tmp == list->HEAD && tmp == list->TAIL) {
        free(tmp);
        list->HEAD = NULL;
        list->TAIL = NULL;
    }
    else if(tmp == list->HEAD) {
        ClientNode *new_head = list->HEAD->next;
        free(tmp);
        list->HEAD = new_head;
        new_head->prev = NULL;
    }
    else if(tmp == list->TAIL) {
        ClientNode *new_tail = list->TAIL->prev;
        free(tmp);
        list->TAIL = new_tail;
        new_tail->next = NULL;
    }
    else {
        ClientNode *prevNode = tmp->prev;
        ClientNode *nextNode = tmp->next;
        free(tmp);
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }

    --list->len;
    return list->len;
}

const char* client_list_to_string(ClientList* list, char buffer[]) {
    if(list == NULL) return NULL;

    if(list->HEAD == NULL) {
        sprintf(buffer, "EMPTY\n");
        return buffer;
    }

    ClientNode *tmp = list->HEAD;
    char buff[1024];
    client_to_string(tmp->node, buff);
    sprintf(buffer, "%s   HEAD", buff);
    tmp = tmp->next;

    while(tmp != NULL) {
        char buff[1024];
        client_to_string(tmp->node, buff);
        sprintf(buffer + strlen(buffer), "\n%s", buff);
        tmp = tmp->next;
    }
    sprintf(buffer + strlen(buffer), "   TAIL\n");
    return buffer;
}


ClientNode* client_list_srch(Client *client, ClientList *list) {
    if (client == NULL || list == NULL) return NULL;

    ClientNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node == client) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

Client* client_list_find_pid(pid_t pid, ClientList *list) {
    if(list == NULL) return NULL;

    ClientNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->pid = pid) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}



//////////////////////////////
//
// Client test function
//

void test_client_list() {
    printf("\r\n");

    Client *client_one = client_init(1, ARRIVING, NULL, NULL, NULL);
    Client *client_two = client_init(2, ARRIVING, NULL, NULL, NULL);

    ClientList *client_list = client_list_init();
    client_list_add_client(client_one, client_list);
    client_list_add_client(client_two, client_list);

    int buffer_size = 1024*client_list->len;
    char buffer[buffer_size];
    client_list_to_string(client_list, buffer);

    printf("INITIAL LIST: \r\n%s\r\n", buffer);

    client_list_rem_client(client_one, client_list);
    client_list_to_string(client_list, buffer);

    printf("CLIENT ONE REMOVED: \r\n%s\r\n", buffer);

    client_list_rem_client(client_two, client_list);
    client_list_to_string(client_list, buffer);

    printf("CLIENT TWO REMOVED:\r\n%s\r\n", buffer);
    
    client_del(client_one);
    client_del(client_two);
    client_list_del(client_list);
}
