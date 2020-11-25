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


// MAINTAINS THE GYM DATABASE OF RESOURCES
typedef struct {
    ClientList* waitingList;
    ClientList* arrivingList;
    TrainerList* trainerList;
    int maxCouches;
} Gym;


void open_gym(int numberTrainers, int numberCouches, int numberClients, int useSemaphors);
int init_shared_gym(int maxCouches);
Gym* get_shared_gym();
void clean_shared_gym(Gym* sharedGym);



#endif // GYM_H