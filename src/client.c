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
#include "workout_room.h"
#include "resource_manager.h"



static const char CLIENT_ARRIVING_SEM_NAME[] = "/sem_client_arriving";
static const char CLIENT_WAITING_SEM_NAME[] = "/sem_client_waiting";



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
        printf("new client pid: %d\r\n", getpid());
        int ret = client_proc_state_machine();
        exit(ret);
    } else {
        return pid;
    }
}

int client_proc_state_machine() {

    open_client_sem();
    open_resource_manager();
    
    int pid = getpid();

    Gym *gym = gym_init();
    open_shared_gym();
    update_gym(gym);

    if(gym == NULL) {
        perror("client_proc_state_machine() get_shared_gym");
        return -1;
    }


    Client *client = client_init(getpid(), ARRIVING, NULL, NULL, NULL);
    Trainer *trainer;

    char buffer[BUFFER_SIZE] = "\0";

    bool shutdown = false;
    bool has_couch = false;


    while(!shutdown) {
        // Execute state machine

        switch(client->state) {
            case ARRIVING:
                // ADD SEMAPHORE

                if(gym->boundary_case) {
                    sem_wait(client_arriving_sem);
                }

                printf("CLIENT %d ARRIVING\r\n", pid);

                client_list_add_client(client, gym->arrivingList);
                update_shared_gym(gym);

                
                if(gym->boundary_case) {
                    // WAIT FOR ALL TRAINERS TO FINISH TRAVELLING

                    trainer = trainer_list_find_state(TRAVELLING, gym->trainerList);

                    while(trainer != NULL) {
                        delay(1*gym->unit_time);
                        update_gym(gym);
                        trainer = trainer_list_find_state(TRAVELLING, gym->trainerList);
                    }
                }

                // NOW FIND TRAINER ON PHONE
                update_gym(gym);

                trainer = trainer_list_find_phone(gym->trainerList);
                while(trainer != NULL && trainer->client_pid > 0) {
                    update_gym(gym);
                    trainer = trainer_list_find_phone(gym->trainerList);
                    delay(1*gym->unit_time);
                }

                if(trainer != NULL && trainer->client_pid <= 0) {
                    client->current_trainer = *trainer;
                    update_shared_gym(gym);

                    while(trainer != NULL && trainer->client_pid != getpid()) {
                        // wait for the trainer to also claim us

                        update_gym(gym);
                        trainer = trainer_list_find_pid(client->current_trainer.pid, gym->trainerList);
                        delay(1*gym->unit_time);
                    }   
                    client->state = TRAINING;
                } 
                else {
                    client->state = MOVING;
                }
                
                delay(2*gym->unit_time); 
                client_list_rem_client(client, gym->arrivingList);

                // CLOSE SEMAPHORE
                if(gym->boundary_case) {
                    sem_post(client_arriving_sem);
                }

                break;

            case WAITING:
                // ADD SEMAPHORE
                if(gym->boundary_case)
                    sem_wait(client_waiting_sem);

                if(gym->waitingList->len >= gym->maxCouches && !has_couch) {
                    client->state = LEAVING; 

                    // RELEASE SEMAPHORE
                    if(gym->boundary_case)
                        sem_post(client_waiting_sem);

                    printf("CLIENT %d WAITING ROOM FULL WITH %d COUCHES OCCUPIED. LEAVING\r\n", pid, gym->waitingList->len);

                    break;
                }

                if(!gym->boundary_case)
                    delay(1*gym->unit_time);

                client_list_add_client(client, gym->waitingList);
                update_shared_gym(gym);


                if(!has_couch)
                    printf("CLIENT %d WAITING. %d OF %d COUCHES TAKEN\r\n", getpid(), gym->waitingList->len, gym->maxCouches);

                has_couch = true;


                // Check if a trainer has picked us up
                trainer = trainer_list_find_client(getpid(), gym->trainerList);
                if(trainer != NULL) {
                    has_couch = false;
                    client->state = TRAINING;
                    client_list_rem_client(client, gym->waitingList);
                    copy_trainer(&client->current_trainer, trainer);
                }

                delay(1*gym->unit_time);

                // RELEASE 
                if(gym->boundary_case)
                    sem_post(client_waiting_sem);
                break;

            case MOVING:
                    //This is the "traveling" piece,

                    printf("CLIENT %d MOVING\r\n", pid);

                    delay(6*gym->unit_time);
                    client->state = WAITING;
                break;

            case TRAINING:
                printf("CLIENT %d TRAINING\r\n", pid);

                client_list_add_client(client, gym->workoutList);
                update_shared_gym(gym);

                delay(2*gym->unit_time);

                client_workout_event(gym, client);
                client->state = LEAVING;

                break;

            case LEAVING:
                printf("CLIENT %d LEAVING\r\n", pid);

                shutdown = true;
                break;

            default:
                printf("CLIENT %d UNKOWN STATE \r\n", getpid());

                shutdown = true;
        }


        // Update local & shared memory
        update_shared_gym(gym);
        update_gym(gym);
    }

    // Remove client from any lists & close semaphores

    printf("Client %d destroying data\r\n", getpid());

    client_list_rem_client(client, gym->arrivingList);
    client_list_rem_client(client, gym->waitingList);
    client_list_rem_client(client, gym->workoutList);
    update_shared_gym(gym);

    client_del(client);
    gym_del(gym);
    
    close_shared_gym();
    close_resource_manager();
    close_trainer_sem();
    close_client_sem();

    printf("Client %d exiting\r\n", getpid());

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

    sprintf(buffer + strlen(buffer), "   trainer: %d", client->current_trainer.pid);

    int sem_val;
    sprintf(buffer + strlen(buffer), "   couch: %d", sem_getvalue(&client->current_couch.couch_mutex, &sem_val));


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

int client_list_del_clients(pid_t exclude, ClientList *list) {
    if(list == NULL) return 0;

    ClientNode *tmp = list->HEAD;

    while(tmp != NULL) {
        if(tmp->node->pid != exclude) client_del(tmp->node);
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

    // Check if we're already in list
    if(client_list_find_pid(client->pid, list) != NULL) { 
        free(new_node);
        return 1;
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
        if(tmp->node->pid == pid) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}

Client* client_list_find_trainer(pid_t pid, ClientList *list) {
    if(list == NULL) return NULL;

    ClientNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->current_trainer.pid == pid) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;    
}





int init_client_sem() {
    sem_unlink(CLIENT_ARRIVING_SEM_NAME);
    client_arriving_sem = sem_open(CLIENT_ARRIVING_SEM_NAME, O_CREAT, 0644, 1);
    if(client_arriving_sem == SEM_FAILED) {
        perror("client_sem_init failed to open arriving sem");
        exit(1);
    }

    sem_unlink(CLIENT_WAITING_SEM_NAME);
    client_waiting_sem = sem_open(CLIENT_WAITING_SEM_NAME, O_CREAT, 0644, 1);
    if(client_waiting_sem == SEM_FAILED) {
        perror("client_sem_init failed to open waiting sem");
        exit(1);
    }
}

int open_client_sem() {
    client_arriving_sem = sem_open(CLIENT_ARRIVING_SEM_NAME, O_CREAT, 0644, 1);
    if(client_arriving_sem == SEM_FAILED) {
        perror("client_sem_init failed to open arriving sem");
        exit(1);
    }

    client_waiting_sem = sem_open(CLIENT_WAITING_SEM_NAME, O_CREAT, 0644, 1);
    if(client_waiting_sem == SEM_FAILED) {
        perror("client_sem_init failed to open waiting sem");
        exit(1);
    }
    return 0;
}

void close_client_sem() {
    sem_close(client_arriving_sem);
    sem_close(client_waiting_sem);
    return;
}

void destroy_client_sem() {
    sem_unlink(CLIENT_ARRIVING_SEM_NAME);
    sem_unlink(CLIENT_WAITING_SEM_NAME);
    return;
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
