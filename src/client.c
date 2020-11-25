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

#include "client.h"


// #define CLIENT_TEST  // UNCOMMENT TO TEST WITH main()

Client* client_init(pid_t pid, ClientState state, Trainer* trainer, Couch* couch, Workout* workout) {
    Client* client = (Client*)malloc(sizeof(Client));

    if(client == NULL) {
        perror("client_init malloc()");
        return NULL;
    }

    client->pid = pid;
    client->state = state;
    client->current_trainer = trainer;
    client->current_couch = couch;
    client->workout = workout;
    return client;
}

int client_del(Client* client) {   
    free(client);
    return 0;
}

const char* client_to_string(Client *client, char buffer[]) {
    if(client == NULL) return NULL;

    sprintf(buffer, "pid: %d", client->pid);
    sprintf(buffer + strlen(buffer), "   state: %d", client->state);
    //sprintf(buffer + strlen(buffer), "   trainer: %ld", client->current_trainer);
    //sprintf(buffer + strlen(buffer), "   couch: %ld", client->current_couch);
    //sprintf(buffer + strlen(buffer), "   workout: %ld", client->workout);
    return buffer;
}

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



#ifdef CLIENT_TEST

int main(int argc, char** argv) {
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

#endif // CLIENT_TEST