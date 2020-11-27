// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project part2 source
//   Date    -   11-20-20
//
// ########################################## 


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "client.h"
#include "workout_room.h"
#include "resource_manager.h"
#include "workout.h"


//#define TEST_WORKOUT_ROOM   // UNCOMMENT TO TEST WITH main()



void start_workout_room() {
    return;
}


int client_workout_event(Gym *gym, SharedGym *shGym, Client *client) {
    if(gym == NULL || client == NULL) {
        perror("client_workout_event invalid_argument");
        return -1;
    }
    if(client->current_trainer.pid < 0) {
        perror("client_workout_event no trainer");
        return -1;
    }

    update_gym(gym, shGym);
    client_list_rem_client(client, gym->arrivingList);
    client_list_rem_client(client, gym->waitingList);
    client_list_add_client(client, gym->workoutList);
    update_shared_gym(shGym, gym);

    //Workout *workout = client_get_workout();
    //client->workout = workout;

    //Weight* weights = client_get_weights();
    //workout->in_use = weights;
    update_shared_gym(shGym, gym);

    //for(int i=0; i<workout->total_weight; ++i) {
    //    client_lift_weights();
    //    --workout->sets_left;
   // }

    //client_replace_weights(weights);
}


int trainer_workout_event(Gym *gym, SharedGym *shGym, Trainer *trainer) {
    if(gym == NULL || trainer == NULL) {
        perror("trainer_workout_event invalid_argument");
        return -1;
    }
    if(trainer->current_client == NULL) {
        perror("trainer_workout_event no client");
        return -1;
    }


}

void test_workout_room() {
    test_resource_manager();
    return;
}