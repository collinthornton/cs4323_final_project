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



//////////////////////////////
//
// Event functions
//


/**
 * @brief Execute the workout event on client process.
 * @param gym (Gym*) gym struct used for IPC
 * @param client (Client*) client in workout event
 * @return (int) return code. negative on error
 */
int client_workout_event(Gym *gym, Client *client);


/**
 * @brief Execute the workout event on trainer proccess.
 * @param gym (Gym*) gym struct used for IPC
 * @param trianer (Trainer*) trainer in workout event
 * @return (int) return code. negative on error
 */
int trainer_workout_event(Gym *gym, Trainer *trainer);



//////////////////////////////
//
// Helper functions
//


/**
 * @brief Choose total weight and total sets. Place in IPC
 * @param gym (Gym*) gym struct for IPC
 * @param trainer (Trainer*) trainer in event
 * @return (int) return code. negative on error
 */
int trainer_set_workout(Gym *gym, Trainer *trainer);


/**
 * @brief Get total weight and total sets from trianer over shared memory
 * @param gym (Gym*) gym struct for IPC
 * @param client (Client*) client in event
 * @param trainer (Trainer*) trainer that's paired with client
 * @param first_time (bool) flag to toggle whether the trainer can change the total number of sets
 */
int client_get_workout(Gym *gym, Client *client, Trainer *trainer, bool first_time);


/**
 * @brief Get grip plates from the resource manager
 * @param gym (Gym*) gym struct for IPC
 * @param client (Client*) client in event
 */
int client_get_weights(Gym *gym, Client *client);


/**
 * @brief Make a weight request from the resource manaegr
 * @param gym (Gym*) gym struct for IPC
 * @param client (Client*) client in event
 * @param weight (Weight*) request
 * @return bool true on succes. else false
 */
bool client_request_weight_allocation(Gym *gym, Client *client, Weight *weight);


/**
 * @brief Delay for a bit and set flags as the client lifts weights
 */
void client_lift_weights();


void test_workout_room(void);

#endif // WORKOUT_ROOM_H