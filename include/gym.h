// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project Gym include
//   Date    -   11-15-20
//
// ########################################## 


#ifndef GYM_H
#define GYM_H

#include <stdbool.h>

#include "trainer.h"
#include "client.h"

#define SHARED_KEY 0x1234
#define BUFFER_SIZE 1024


sem_t shared_gym_sem;

// MAINTAINS THE GYM DATABASE OF RESOURCES
typedef struct {
    Client waitingList[MAX_CLIENTS];
    Client arrivingList[MAX_CLIENTS];
    Client workoutList[MAX_CLIENTS];
    Trainer trainerList[MAX_TRAINERS];

    int len_waiting, len_arriving, len_workout, len_trainer;

    int maxCouches;
    int unit_time; // seconds
} SharedGym;

typedef struct {
    ClientList* waitingList;
    ClientList* arrivingList;
    ClientList* workoutList;
    TrainerList* trainerList;

    int maxCouches;
    int unit_time; // seconds    
} Gym;

void open_gym(int numberTrainers, int numberCouches, int numberClients, int useSemaphors);
int init_shared_gym(int maxCouches);
Gym* gym_init();

Gym* update_gym(Gym *gym, SharedGym *sharedGym);
void update_shared_gym(SharedGym* sharedGym, Gym* gym);
SharedGym* get_shared_gym();

void gym_del(Gym *gym);
void clean_shared_gym(SharedGym* sharedGym);

Client* copy_client(Client *dest, Client *src);
Trainer* copy_trainer(Trainer* dest, Trainer *src);


#endif // GYM_H