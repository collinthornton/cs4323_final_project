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

#define MAX_WAIT_ITER 20

void start_workout_room() {
    return;
}


int client_workout_event(Gym *gym, Client *client) {
    if(gym == NULL || client == NULL) {
        perror("client_workout_event invalid_argument");
        return -1;
    }
    if(client->current_trainer.pid < 0) {
        perror("client_workout_event no trainer");
        return -1;
    }

    Trainer *trainer = trainer_list_find_pid(client->current_trainer.pid, gym->trainerList);
    if(trainer == NULL) {
        perror("client_workout_event() trainer not in list");
        return -1;
    }

    // MAKE SURE WE'VE ADDED OURSELF TO THE CORRECT LIST
    client_list_rem_client(client, gym->arrivingList);
    client_list_rem_client(client, gym->waitingList);
    client_list_add_client(client, gym->workoutList);
    update_shared_gym(gym);

    
    // WAIT FOR THE TRAINER TO SEND US A WORKOUT. MAX 10 SECONDS
    int num_wait = 0;
    while(trainer->workout.total_weight <= 0 && num_wait < MAX_WAIT_ITER) {
        printf("client %d waiting for workout\r\n", getpid());
        sleep(1*gym->unit_time);

        update_gym(gym);
        trainer = trainer_list_find_pid(client->current_trainer.pid, gym->trainerList);
        client = client_list_find_pid(getpid(), gym->workoutList);

        ++num_wait;
    }

    if(num_wait == MAX_WAIT_ITER) {
        perror("client_workout_event timed out waiting for workout");
        return -1;
    }

    // TRAINER SENT WORKOUT. COPY TO CLIENT
    client->workout = trainer->workout;
    update_shared_gym(gym);

    printf("client %d got workout. total weight: %d\r\n", getpid(), client->workout.total_weight);

    // FIGURE OUT HOW MANY PLATES WE NEED WHILE UTILIZING THE SMALLEST NUMBER
    int weight_left = client->workout.total_weight;

    int weights[NUMBER_WEIGHTS];
    
    weights[FORTY_FIVE] = 2*(weight_left/45/2);
    weight_left -= 45*weights[FORTY_FIVE];

    weights[THIRTY_FIVE] = 2*(weight_left/35/2);
    weight_left -= 35*weights[THIRTY_FIVE];

    weights[TWENTY_FIVE] = 2*(weight_left/25/2);
    weight_left -= 25*weights[TWENTY_FIVE];

    weights[TWENTY] = 2*(weight_left/20/2);
    weight_left -= 20*weights[TWENTY];

    weights[FIFTEEN] = 2*(weight_left/15/2);
    weight_left -= 15*weights[FIFTEEN];

    weights[TEN] = 2*(weight_left/10/2);
    weight_left -= 10*weights[TEN];

    weights[FIVE] = 2*(weight_left/5/2);
    weight_left -= 5*weights[FIVE];

    weights[TWO_HALF] = 2*(int)(weight_left/2.5/2);
    weight_left -= (int)(2.5*weights[TWO_HALF]);

    if(weight_left != 0) {
        perror("client_workout_event weight_left nonzero");
        printf("%d\r\n", weight_left);
        return -1;
    }

    Weight *request = weight_init(weights);
    Weight allocation = *request;

    char buffer[BUFFER_SIZE] = "\0";
    weight_to_string(request, buffer);
    printf("client %d weight request\r\n%s\r\n", getpid(), buffer);

    writeWeightRequest(getpid(), request);


    //! THIS IS WHERE WE MIGHT DEADLOCK
    while(grantWeightRequest(getpid()) < 0) {
        sleep(1*gym->unit_time);
    }

    client->workout.in_use = allocation;
    update_shared_gym(gym);
    

    // RUN THROUGH SETS
    //TODO MIGHT NEED TO ADD EXTRA WEIGHT REQUESTS TO ENCOURAGE DEADLOCK
    
    //! SHOULD BE IN A DIFFERENT FUNCTION FOR ROLLBACK
    for(int i=0; i<client->workout.total_sets; ++i) {
        printf("client %d performing set %d of %d\r\n", getpid(), i+1, client->workout.total_sets);
        sleep(2*gym->unit_time);
        --client->workout.sets_left;
        update_shared_gym(gym);
    }

    printf("client %d finished workout\r\n", getpid());


    // REPLACE WEIGHTS
    releaseWeightAllocation(getpid(), &client->workout.in_use);
    Workout *tmp = workout_init(client->workout.total_sets, client->workout.sets_left, client->workout.total_weight, NULL);
    client->workout = *tmp;
    workout_del(tmp);

    //Weight* weights = client_get_weights();
    //workout->in_use = weights;
    update_shared_gym(gym);

    //for(int i=0; i<workout->total_weight; ++i) {
    //    client_lift_weights();
    //    --workout->sets_left;
   // }

    //client_replace_weights(weights);
}


int trainer_workout_event(Gym *gym, Trainer *trainer) {
    if(gym == NULL || trainer == NULL) {
        perror("trainer_workout_event invalid_argument");
        return -1;
    }
    if(trainer->client_pid <= 0) {
        perror("trainer_workout_event no client");
        return -1;
    }

    update_gym(gym);

    Client *client = client_list_find_pid(trainer->client_pid, gym->workoutList);
    if(client == NULL) {
        perror("trainer_workout_event client not found");
        return -1;
    }

    // CREATE THE WORKOUT AND SEND TO CLIENT
    // TODO This will need to setup to cause deadlock

    // GENERATE TOTAL WEIGHT, SHOULD BE A MULTIPLE OF 5
    int total_weight = (rand() % (MAX_WEIGHT - MIN_WEIGHT + 1)) + MIN_WEIGHT;
    total_weight = 5 * (total_weight/5);

    // GENERATE TOTAL SETS
    int total_sets = rand() % 5;
    int sets_left = total_sets;

    trainer->workout.sets_left = sets_left;
    trainer->workout.total_sets = total_sets;
    trainer->workout.total_weight = total_weight;
    update_shared_gym(gym);

    printf("trainer %d picked workout for client %d: total_weight %d, num_sets %d\r\n", getpid(), trainer->client_pid, trainer->workout.total_weight, trainer->workout.total_sets);


    // NOW WAIT FOR CLIENT TO LEAVE THE WORKOUT. TIMEOUT AFTER 10 SECONDS
    // TODO make a way to cancel this

    int num_iter = 0;
    while(client != NULL && num_iter < MAX_WAIT_ITER) {
        printf("Trainer %d waiting for client to finish workout\r\n", getpid());

        sleep(1*gym->unit_time);
        update_gym(gym);
        trainer = trainer_list_find_pid(getpid(), gym->trainerList);
        client = client_list_find_pid(trainer->client_pid, gym->workoutList);
        ++num_iter;
    }

    if(num_iter == MAX_WAIT_ITER) {
        perror("trainer_workout_event timeout waiting for client");
        return -1;
    }

    printf("trainer %d client finished workout out\r\n", getpid());

    // FINISHED OUR JOB
    return 0;
}

void test_workout_room() {
    test_resource_manager();
    return;
}