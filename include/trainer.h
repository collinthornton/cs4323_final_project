#include "client.h"

typedef enum {
    FREE,
    ON_PHONE,
    WITH_CLIENT,
    TRAVELLING
} TrainerState;

typedef struct {
    int pID;
    TrainerState state;
    Client* current_client;
} Trainer;

typedef struct {
    Trainer* node;
    Trainer* prev;
    Trainer* next;
} TrainerNode;

typedef struct {
    TrainerNode* HEAD, TAIL;
    TrainerNode* node;

    int size;
    int allocated;
} TrainerList;

Trainer* trainer_init(TrainerState state, Client* client, int pID);

TrainerList* trainer_init_list();
int trainer_add_to_list(Trainer *trainer, TrainerList* list);
int trainer_rem_from_list(Trainer *trainer, TrainerList *list);

int trainer_del_list(TrainerList *list);
int trainer_del_trainer(Trainer *trainer);

void trainer_start();

Trainer* find_trainer_with_client(Client* clientToFind, TrainerList* trainerList);

Trainer* find_available_trainer(TrainerList* trainerList);
Trainer* find_trainer_on_phone(TrainerList* trainerList);
