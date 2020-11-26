// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project trainer struct source
//   Date    -   11-24-20
//
// ########################################## 

#include <stdio.h>
#include <string.h>

#include "trainer.h"


// #define TRAINER_TEST  // UNCOMMENT TO TEST WITH main()

Trainer* trainer_init(pid_t pid, TrainerState state, Client* client) {
    Trainer* trainer = (Trainer*)malloc(sizeof(Trainer));

    if(trainer == NULL) {
        perror("trainer_init malloc()");
        return NULL;
    }

    trainer->pid = pid;
    trainer->state = state;
    trainer->current_client = client;
    return trainer;
}


int trainer_del(Trainer* trainer) {   
    free(trainer);
    return 0;
}

const char* trainer_to_string(Trainer *trainer, char buffer[]) {
    if(trainer == NULL) return NULL;

    sprintf(buffer, "pid: %d", trainer->pid);
    sprintf(buffer + strlen(buffer), "   state: %d", trainer->state);
    //sprintf(buffer + strlen(buffer), "   trainer: %ld", trainer->current_client);
    return buffer;
}

TrainerList* trainer_list_init() {
    TrainerList* list = (TrainerList*)malloc(sizeof(TrainerList));

    if(list == NULL) {
        perror("trainer_list_init malloc()");
        return NULL;
    }

    list->HEAD = NULL;
    list->TAIL = NULL;
    list->len = 0;
    return list;
}

int trainer_list_del(TrainerList *list) {
    if(list == NULL) return 0;

    TrainerNode *tmp = list->HEAD;

    while(tmp != NULL) {
        TrainerNode *next = tmp->next;
        free(tmp);
        tmp = next;
    }

    free(list);

    return 0;
}

int trainer_list_add_trainer(Trainer *trainer, TrainerList *list) {
    TrainerNode *new_node = (TrainerNode*)malloc(sizeof(TrainerNode));

    if(new_node == NULL) {
        perror("trainer_list_add_trainer malloc()");
        return -1;
    }

    new_node->node = trainer;
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

int trainer_list_rem_trainer(Trainer *trainer, TrainerList *list) {
    if(list == NULL || trainer == NULL) {
        perror("trainer_list_rem_trainer() invalid_argument");
        return -1;
    }

    TrainerNode *tmp = trainer_list_srch(trainer, list);

    if(tmp == NULL) return list->len; 

    if(tmp == list->HEAD && tmp == list->TAIL) {
        free(tmp);
        list->HEAD = NULL;
        list->TAIL = NULL;
    }
    else if(tmp == list->HEAD) {
        TrainerNode *new_head = list->HEAD->next;
        free(tmp);
        list->HEAD = new_head;
        new_head->prev = NULL;
    }
    else if(tmp == list->TAIL) {
        TrainerNode *new_tail = list->TAIL->prev;
        free(tmp);
        list->TAIL = new_tail;
        new_tail->next = NULL;
    }
    else {
        TrainerNode *prevNode = tmp->prev;
        TrainerNode *nextNode = tmp->next;
        free(tmp);
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }

    --list->len;
    return list->len;
}

const char* trainer_list_to_string(TrainerList* list, char buffer[]) {
    if(list == NULL) return NULL;

    if(list->HEAD == NULL) {
        sprintf(buffer, "EMPTY\n");
        return buffer;
    }

    TrainerNode *tmp = list->HEAD;
    char buff[1024];
    trainer_to_string(tmp->node, buff);
    sprintf(buffer, "%s   HEAD", buff);
    tmp = tmp->next;

    while(tmp != NULL) {
        char buff[1024];
        trainer_to_string(tmp->node, buff);
        sprintf(buffer + strlen(buffer), "\n%s", buff);
        tmp = tmp->next;
    }
    sprintf(buffer + strlen(buffer), "   TAIL\n");
    return buffer;
}



Trainer* trainer_list_find_client(Client *client, TrainerList *list) {
    if(list == NULL || client == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->current_client == client) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}

Trainer* trainer_list_find_phone(TrainerList *list) {
    if(list == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->state == ON_PHONE) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}

Trainer* trainer_list_find_available(TrainerList *list) {
    if(list == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node->state == FREE) return tmp->node;
        tmp = tmp->next;
    }
    return NULL;
}


TrainerNode* trainer_list_srch(Trainer *trainer, TrainerList *list) {
    if (trainer == NULL || list == NULL) return NULL;

    TrainerNode *tmp = list->HEAD;
    while(tmp != NULL) {
        if(tmp->node == trainer) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}



#ifdef TRAINER_TEST

int main(int argc, char** argv) {
    printf("\r\n");

    Trainer *trainer_one = trainer_init(1, FREE, NULL);
    Trainer *trainer_two = trainer_init(2, FREE, NULL);
    Trainer *trainer_three = trainer_init(3, FREE, NULL);

    TrainerList *trainer_list = trainer_list_init();
    trainer_list_add_trainer(trainer_one, trainer_list);
    trainer_list_add_trainer(trainer_two, trainer_list);
    trainer_list_add_trainer(trainer_three, trainer_list);

    int buffer_size = 1024*trainer_list->len;
    char buffer[buffer_size];
    trainer_list_to_string(trainer_list, buffer);

    printf("INITIAL LIST: \r\n%s\r\n", buffer);

    trainer_list_rem_trainer(trainer_one, trainer_list);
    trainer_list_to_string(trainer_list, buffer);

    printf("TRAINER ONE REMOVED: \r\n%s\r\n", buffer);

    trainer_list_rem_trainer(trainer_two, trainer_list);
    trainer_list_to_string(trainer_list, buffer);

    printf("TRAINER TWO REMOVED:\r\n%s\r\n", buffer);

    trainer_list_rem_trainer(trainer_three, trainer_list);
    trainer_list_to_string(trainer_list, buffer);

    printf("TRAINER THREE REMOVED\r\n%s\r\n", buffer);
    
    trainer_del(trainer_one);
    trainer_del(trainer_two);
    trainer_del(trainer_three);
    trainer_list_del(trainer_list);
}

#endif // CLIENT_TEST
#include "trainer.h"


// #define TRAINER_TEST // UNCOMMENT TO TEST WITH main()
