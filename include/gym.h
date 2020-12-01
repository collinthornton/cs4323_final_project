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
#include <fcntl.h>

#include "trainer.h"
#include "client.h"

#define SHARED_KEY 0x1234
#define BUFFER_SIZE 1024

// #define VERBOSE


// MAINTAINS THE GYM DATABASE OF RESOURCES
typedef struct {
    Client waitingList[MAX_CLIENTS];
    Client arrivingList[MAX_CLIENTS];
    Client workoutList[MAX_CLIENTS];
    Trainer trainerList[MAX_TRAINERS];

    int maxCouches;
    int num_trainers;
    int unit_time; // milliseconds

    bool boundary_case;
    bool realistic;
    bool fix_deadlock;
    bool detect_deadlock;
    bool trainer_log;

    pid_t deadlock_victim;
} SharedGym;

typedef struct {
    ClientList* waitingList;
    ClientList* arrivingList;
    ClientList* workoutList;
    TrainerList* trainerList;

    int maxCouches;
    int num_trainers;

    bool boundary_case;
    bool realistic;
    bool fix_deadlock;
    bool detect_deadlock;
    bool trainer_log;

    int unit_time; // seconds   
    pid_t deadlock_victim; 
} Gym;

void open_gym(int numberTrainers, int numberCouches, int numberClients, int useSemaphors);
int init_shared_gym(int maxCouches, int numTrainers, bool boundary_case, bool realistic, bool detectDeadlock, bool fixDeadlock, bool trainerLog);
Gym* gym_init();

Gym* update_gym(Gym *gym);
void update_shared_gym(Gym* gym);
void open_shared_gym();

void gym_del(Gym *gym);
void close_shared_gym();
void destroy_shared_gym();

Client* copy_client(Client *dest, Client *src);
Trainer* copy_trainer(Trainer* dest, Trainer *src);


void delay(long mS);

#endif // GYM_H