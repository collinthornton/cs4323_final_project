// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project par2 include
//   Date    -   11-20-20
//
// ########################################## 


#ifndef WORKOUT_ROOM_H
#define WORKOUT_ROOM_H

#include "gym.h"
#include "client.h"




void start_workout_room(void);


int client_workout_event(Gym *gym, SharedGym *shGym,  Client *client);
int trainer_workout_event(Gym *gym, SharedGym *shGym, Trainer *trainer);

int init_shared_workout(int maxWeight);
int trainer_set_worktout();
Workout* client_get_workout();

Weight* client_get_weights();
void client_replace_weights(Weight* weight);

void client_lift_weights();

void test_workout_room(void);

#endif // WORKOUT_ROOM_H