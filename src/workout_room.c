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
#include "vector.h"

#define CAUSE_DEADLOCK

#define MAX_WAIT_ITER 20

void start_workout_room() {
    return;
}




//////////////////////////////
//
// Client perform workout
//

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

    
    // GET THE INTIAL WORKOUT FROM THE TRAINER
    client_get_workout(gym, client, trainer);
    update_shared_gym(gym);

    printf("client %d got workout. total weight: %d\r\n", getpid(), client->workout.total_weight);

    for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) client->workout.in_use.num_plates[i] = 0;
    client->workout.sets_left = client->workout.total_sets;
    

    // RUN THROUGH SETS
    //TODO MIGHT NEED TO ADD EXTRA WEIGHT REQUESTS TO ENCOURAGE DEADLOCK
    
    //! SHOULD BE IN A DIFFERENT FUNCTION FOR ROLLBACK
    for(int i=0; i<client->workout.total_sets; ++i) {
        sleep(1*gym->unit_time);
        printf("client %d performing set %d of %d\r\n", getpid(), i+1, client->workout.total_sets);

        // Redo the set if we're the deadlock victim
        if(client_get_weights(gym, client) == -2) {
            
            printf("\r\nclient %d successfully rolled back from set %d to set %d\r\n\r\n", getpid(), i+1, i);
            --i;
            continue;
        }
        update_shared_gym(gym);       
       
        client_lift_weights(gym, client);
        update_shared_gym(gym);

        #ifndef CAUSE_DEADLOCK
        releaseWeightAllocation(getpid(), &client->workout.in_use);
        #endif // CAUSE_DEADLOCK
        
        // Let the trainer change the weight
        //client_get_workout(gym, client, trainer);
        //client->workout.sets_left = client->workout.total_sets - i;
        //update_shared_gym(gym);

        // Get the new weights
        //client_get_weights(gym, client);
        //update_shared_gym(gym);

        //--client->workout.sets_left;
    }

    #ifdef CAUSE_DEADLOCK
    releaseWeightAllocation(getpid(), &client->workout.in_use);
    #endif // CAUSE_DEADLOCK

    printf("client %d finished workout\r\n", getpid());


    // REPLACE WEIGHTS
    Workout *tmp = workout_init(client->workout.total_sets, client->workout.sets_left, client->workout.total_weight, NULL);
    client->workout = *tmp;
    workout_del(tmp);


    update_shared_gym(gym);

}


//////////////////////////////
//
// Trainer perform workout
//

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
    trainer_set_workout(gym, trainer);
    update_shared_gym(gym);

    printf("trainer %d picked workout for client %d: total_weight %d, num_sets %d\r\n", getpid(), trainer->client_pid, trainer->workout.total_weight, trainer->workout.total_sets);


    // NOW WAIT FOR CLIENT TO LEAVE THE WORKOUT. TIMEOUT AFTER 10 SECONDS
    // TODO make a way to cancel this

    int num_iter = 0;

    while(client != NULL && num_iter < MAX_WAIT_ITER) {
        //printf("Trainer %d waiting for client to finish set\r\n", getpid());

        sleep(2*gym->unit_time);
        update_gym(gym);
        trainer = trainer_list_find_pid(getpid(), gym->trainerList);
        client = client_list_find_pid(trainer->client_pid, gym->workoutList);
       // ++num_iter;
    }

    trainer->workout.sets_left = -1;
    trainer->workout.total_sets = -1;
    trainer->workout.total_weight = -1;

    if(num_iter == MAX_WAIT_ITER) {
        perror("trainer_workout_event timeout waiting for client");
        return -1;
    }

    printf("trainer %d client finished workout out\r\n", getpid());

    // FINISHED OUR JOB
    return 0;
}





//////////////////////////////
//
// Helper functions
//

int trainer_set_workout(Gym *gym, Trainer *trainer) {
    // TODO This will need to setup to cause deadlock

    // GENERATE TOTAL WEIGHT, SHOULD BE A MULTIPLE OF 5
    int total_weight = (rand() % (MAX_WEIGHT - MIN_WEIGHT + 1)) + MIN_WEIGHT;
    total_weight = 5 * (total_weight/5);

    // GENERATE TOTAL SETS
    //int total_sets = (rand() % 5) + 1;

    int total_sets = 5;             // Easier to just set to a number
    int sets_left = -1;             // Decided by the client

    trainer->workout.sets_left = sets_left;
    trainer->workout.total_sets = total_sets;
    trainer->workout.total_weight = total_weight;

    return 0;
}



int client_get_workout(Gym *gym, Client *client, Trainer *trainer) {
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
    client->workout.total_sets = trainer->workout.total_sets;
    client->workout.sets_left = client->workout.total_sets;
    client->workout.total_weight = trainer->workout.total_weight;
    return 0;
}


int client_get_weights(Gym *gym, Client *client) {
    // FIGURE OUT HOW MANY PLATES WE NEED WHILE UTILIZING THE SMALLEST NUMBER
    int weight_left = client->workout.total_weight;

    int weights[NUMBER_WEIGHTS];
    
    #ifndef CAUSE_DEADLOCK
    weights[FORTY_FIVE] = 0;//2*(weight_left/45/2);
    weight_left -= 45*weights[FORTY_FIVE];

    weights[THIRTY_FIVE] = 0;//2*(weight_left/35/2);
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

    #else

    printf("Client %d has %d sets left\r\n", getpid(), client->workout.sets_left);
    switch(client->workout.sets_left) {
        case 5:
            for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 2;
            break;
        case 4:
            for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 5;
            break;
        case 3:
            for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 7;
            break;
        case 2:
            for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 6;
            break;
        case 1:
            for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 5;
            break;
    }

    //for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 5+(rand() % 5);
 
    Weight *request = weight_init(weights);


    vector_subtract(request->num_plates, client->workout.in_use.num_plates, NUMBER_WEIGHTS);

    #endif // CAUSE_DEADLOCK


    Weight req = *request;

    char buffer[BUFFER_SIZE] = "\0";
    weight_to_string(&client->workout.in_use, buffer);
    printf("client %d in use\r\n%s\r\n", getpid(), buffer);

    weight_to_string(request, buffer);
    printf("client %d making weight request\r\n%s\r\n", getpid(), buffer);

    int ret;
    while((ret = writeWeightRequest(getpid(), request)) < 0) {
        printf("client %d request denied: %d\r\n", getpid(), ret);
        sleep(1*gym->unit_time);
    }


    printf("client %d successfully requested weights\r\n", getpid());
    sleep(2*gym->unit_time);



    //! THIS IS WHERE WE MIGHT DEADLOCK
    bool success = false;
    do {
        success = client_request_weight_allocation(gym, client, &req);

        // CHECK IF WE'RE SET AS THE DEADLOCK VICTIM
        if(gym->deadlock_victim == getpid()) {
            printf("\r\n Client %d targeted as deadlock victim -> releasing weight allocation and requests\r\n\r\n", getpid());


            releaseWeightAllocation(getpid(), &client->workout.in_use); 
            vector_subtract(client->workout.in_use.num_plates, client->workout.in_use.num_plates, NUMBER_WEIGHTS);

            Weight *tmp_req = weight_init(req.num_plates);
            removeWeightRequest(getpid(), tmp_req);

            weight_to_string(&client->workout.in_use, buffer);
            printf("Client %d released allocation\r\n%s\r\n", getpid(), buffer);

            weight_to_string(&req, buffer);
            printf("Client %d release request\r\n%s\r\n", getpid(), buffer);

            // Roll back a set
            //++client->workout.sets_left;
            update_shared_gym(gym);
            update_gym(gym);
            return -2;
        }

    } while(success == false);



    printf("client %d got weight allocation\r\n", getpid());

    vector_add(client->workout.in_use.num_plates, req.num_plates, NUMBER_WEIGHTS);
    //client->workout.in_use = req;
    return 0;
}



bool client_request_weight_allocation(Gym *gym, Client *client, Weight *weight) {
    int ret;
    while((ret = grantWeightRequest(getpid())) < 0 && gym->deadlock_victim != getpid()) {
        printf("Client %d allocation denied\r\n", getpid());
        sleep(1*gym->unit_time);
        update_gym(gym);
    }
    return (ret == 0) ? true : false;
}

void client_lift_weights(Gym *gym, Client *client) {
    sleep(2*gym->unit_time);
    --client->workout.sets_left;
}

void test_workout_room() {
    test_resource_manager();
    return;
}