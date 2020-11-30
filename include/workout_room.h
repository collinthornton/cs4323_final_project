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

#include <stdbool.h>

#include "gym.h"
#include "client.h"


void init_workout_room(bool detect_deadlock, bool fix_deadlock);

int client_workout_event(Gym *gym, Client *client);
int trainer_workout_event(Gym *gym, Trainer *trainer);

int trainer_set_workout(Gym *gym, Trainer *trainer);
int client_get_workout(Gym *gym, Client *client, Trainer *trainer, bool first_time);

int client_get_weights(Gym *gym, Client *client);
bool client_request_weight_allocation(Gym *gym, Client *client, Weight *weight);

void client_lift_weights();

void test_workout_room(void);

#endif // WORKOUT_ROOM_H