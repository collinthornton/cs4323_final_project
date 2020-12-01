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
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "client.h"
#include "workout_room.h"
#include "resource_manager.h"
#include "workout.h"
#include "vector.h"
#include "recordbook.h"


static int init_weight;





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
    client_get_workout(gym, client, trainer, true);
    update_shared_gym(gym);

    if(!gym->realistic)
        printf("client %d got workout. total sets: %d\r\n", getpid(), client->workout.total_sets);

    else
        printf("client %d got workout. total weight: %d\r\n", getpid(), client->workout.total_weight);

    for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) client->workout.in_use.num_plates[i] = 0;
    client->workout.sets_left = client->workout.total_sets;
    

    // RUN THROUGH SETS
    //TODO MIGHT NEED TO ADD EXTRA WEIGHT REQUESTS TO ENCOURAGE DEADLOCK
    
    //! SHOULD BE IN A DIFFERENT FUNCTION FOR ROLLBACK
    for(int i=0; i<client->workout.total_sets; ++i) {
        delay(1*gym->unit_time);

        if(!gym->realistic)
            printf("client %d performing set %d of %d\r\n", getpid(), i+1, client->workout.total_sets);
            
        else
            printf("client %d performing set %d of %d with %d weight\r\n", getpid(), i+1, client->workout.total_sets, client->workout.total_weight);

        // Redo the set if we're the deadlock victim
        if(client_get_weights(gym, client) == -2) {
            
            printf("\r\nclient %d successfully rolled back from set %d to set %d\r\n\r\n", getpid(), i+1, i);
            --i;
            continue;
        }
        client->workout.total_weight = -1;
        update_shared_gym(gym);       
       
        client_lift_weights(gym, client);
        update_shared_gym(gym);

        client_get_workout(gym, client, trainer, false);
    }

    releaseWeightAllocation(getpid(), &client->workout.in_use);

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
    
    while(client == NULL) {
        delay(1*gym->unit_time);
        update_gym(gym);
        client = client_list_find_pid(trainer->client_pid, gym->workoutList);
    }


    // CREATE THE WORKOUT AND SEND TO CLIENT
    init_weight = rand();

    trainer_set_workout(gym, trainer);
    update_shared_gym(gym);

    // WAIT FOR CLIENT TO ACKNOWLEDGE WORKOUT
    while(client->workout.total_weight <= 0) {
        delay(2*gym->unit_time);
        update_gym(gym);
        trainer = trainer_list_find_pid(getpid(), gym->trainerList);
        client = client_list_find_pid(trainer->client_pid, gym->workoutList);
    }       

    int total_weight = client->workout.total_weight;

    if(!gym->realistic)
        printf("trainer %d picked workout for client %d: num_sets %d\r\n", getpid(), trainer->client_pid, trainer->workout.total_sets);

    else
        printf("trainer %d picked workout for client %d: total_weight %d, num_sets %d\r\n", getpid(), trainer->client_pid, trainer->workout.total_weight, trainer->workout.total_sets);


    // NOW WAIT FOR CLIENT TO LEAVE THE WORKOUT
    while(client != NULL) {        
        #ifdef VERBOSE
        printf("Trainer %d waiting for client to finish set\r\n", getpid());
        #endif // VERBOSE

        trainer_set_workout(gym, trainer);
        update_shared_gym(gym);
        update_gym(gym);
        trainer = trainer_list_find_pid(getpid(), gym->trainerList);
        client = client_list_find_pid(trainer->client_pid, gym->workoutList);

        // WAIT FOR CLIENT TO GRAB WEIGHTS
        while(client != NULL && client->workout.total_weight > 0) {
            delay(2*gym->unit_time);
            update_gym(gym);
            trainer = trainer_list_find_pid(getpid(), gym->trainerList);
            client = client_list_find_pid(trainer->client_pid, gym->workoutList);
        }

        if(client != NULL)
            total_weight += client->workout.total_weight;

        delay(2*gym->unit_time);

        if(gym->realistic && client != NULL)
            printf("trainer %d picked %d weight for client %d\r\n", getpid(), trainer->workout.total_weight, trainer->client_pid);
    }

    trainer->workout.sets_left = -1;
    trainer->workout.total_sets = -1;
    trainer->workout.total_weight = -1;


    if(gym->trainer_log) {
        Emp record_entry;
        record_entry.id = trainer->client_pid;
        record_entry.weight = total_weight;

        sprintf(record_entry.name, "client %d", trainer->client_pid);

        addToRecordBook(&record_entry);
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

    static int weight_increase = 0;
    weight_increase += 5;

    int total_weight = ((init_weight) % (MAX_WEIGHT - MIN_WEIGHT + 1)/2) + weight_increase;
    total_weight = (total_weight > MAX_WEIGHT*2) ? 2*MAX_WEIGHT : total_weight;
    total_weight = 5 * (total_weight/5);

    // GENERATE TOTAL SETS
    int total_sets = (rand() % 11) + 1;;    
    int sets_left = -1;                     // Decided by the client

    trainer->workout.sets_left = sets_left;
    trainer->workout.total_sets = total_sets;
    trainer->workout.total_weight = total_weight;

    return 0;
}



int client_get_workout(Gym *gym, Client *client, Trainer *trainer, bool first_time) {
    // WAIT FOR THE TRAINER TO SEND US A WORKOUT
    update_gym(gym);
    trainer = trainer_list_find_client(getpid(), gym->trainerList);

    client->workout.total_weight = -1;

    while(trainer->workout.total_weight <= 0) {
        #ifdef VERBOSE
        printf("client %d waiting for workout\r\n", getpid());
        #endif //VERBOSE
        delay(1*gym->unit_time);

        update_gym(gym);
        client = client_list_find_pid(getpid(), gym->workoutList);
        trainer = trainer_list_find_pid(client->current_trainer.pid, gym->trainerList);
    }

    // TRAINER SENT WORKOUT. COPY TO CLIENT
    if(first_time) {
        client->workout.total_sets = trainer->workout.total_sets;
        client->workout.sets_left = client->workout.total_sets;
    }

    client->workout.total_weight = trainer->workout.total_weight;

    if(gym->realistic)
        printf("client %d updated workout to weight %d from trainer %d\r\n", getpid(), client->workout.total_weight, trainer->pid);

    return 0;
}


int client_get_weights(Gym *gym, Client *client) {
    // FIGURE OUT HOW MANY PLATES WE NEED WHILE UTILIZING THE SMALLEST NUMBER
    int weight_left = client->workout.total_weight;

    int weights[NUMBER_WEIGHTS];
    
    if(gym->realistic) {
        weights[FORTY_FIVE] = 2*(weight_left/45/2);
        weights[FORTY_FIVE] = (weights[FORTY_FIVE] > 4) ? 4 : weights[FORTY_FIVE];
        weight_left -= 45*weights[FORTY_FIVE];

        weights[THIRTY_FIVE] = 2*(weight_left/35/2);
        weights[THIRTY_FIVE] = (weights[THIRTY_FIVE] > 4) ? 4 : weights[THIRTY_FIVE];
        weight_left -= 35*weights[THIRTY_FIVE];

        weights[TWENTY_FIVE] = 2*(weight_left/25/2);
        weights[TWENTY_FIVE] = (weights[TWENTY_FIVE] > 4) ? 4 : weights[TWENTY_FIVE];
        weight_left -= 25*weights[TWENTY_FIVE];

        weights[TWENTY] = 2*(weight_left/20/2);
        weights[TWENTY] = (weights[TWENTY] > 4) ? 4 : weights[TWENTY];
        weight_left -= 20*weights[TWENTY];

        weights[FIFTEEN] = 2*(weight_left/15/2);
        weights[FIFTEEN] = (weights[FIFTEEN] > 4) ? 4 : weights[FIFTEEN];
        weight_left -= 15*weights[FIFTEEN];

        weights[TEN] = 2*(weight_left/10/2);
        weights[TEN] = (weights[TEN] > 4) ? 4 : weights[TEN];
        weight_left -= 10*weights[TEN];

        weights[FIVE] = 2*(weight_left/5/2);
        weights[FIVE] = (weights[FIVE] > 4) ? 4 : weights[FIVE];
        weight_left -= 5*weights[FIVE];

        weights[TWO_HALF] = 2*(int)(weight_left/2.5/2);
        weights[TWO_HALF] = (weights[TWO_HALF] > 4) ? 4 : weights[TWO_HALF];
        weight_left -= (int)(2.5*weights[TWO_HALF]);

        if(weight_left != 0) {
            perror("client_workout_event weight_left nonzero");
            printf("%d\r\n", weight_left);
            return -1;
        }
    }
    else {
        #ifdef VERBOSE
        printf("Client %d has %d sets left\r\n", getpid(), client->workout.sets_left);
        #endif // VERBOSE
        
        switch(client->workout.sets_left) {
            case 11:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 1;
                break;        
            case 10:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 3;
                break;
            case 9:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 8;
                break;
            case 8:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 9;
                break;
            case 7:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 10;
                break;
            case 6:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 6;
                break;        
            case 5:
                for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) weights[i] = 8;
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
    }

    Weight *request = weight_init(weights);
    vector_subtract(request->num_plates, client->workout.in_use.num_plates, NUMBER_WEIGHTS);

    Weight req = *request;

    #ifdef VERBOSE
    char buffer[BUFFER_SIZE] = "\0";
    weight_to_string(&client->workout.in_use, buffer);
    
    
    printf("client %d in use\r\n%s\r\n", getpid(), buffer);
    weight_to_string(request, buffer);
    printf("client %d making weight request\r\n%s\r\n", getpid(), buffer);
    #endif // VERBOSE

    int ret;
    while((ret = writeWeightRequest(getpid(), request)) < 0) {
        #ifdef VERBOSE
        printf("client %d request denied: %d\r\n", getpid(), ret);
        #endif // VERBOSE
        delay(1*gym->unit_time);
    }

    #ifdef VERBOSE
    printf("client %d successfully requested weights\r\n", getpid());
    #endif // VERBOSE
    delay(2*gym->unit_time);



    //! THIS IS WHERE WE MIGHT DEADLOCK
    bool success = false;
    do {
        success = client_request_weight_allocation(gym, client, &req);

        // CHECK IF WE'RE SET AS THE DEADLOCK VICTIM
        if(gym->deadlock_victim == getpid()) {
            printf("\r\nClient %d targeted as deadlock victim -> releasing weight allocation and requests\r\n\r\n", getpid());


            releaseWeightAllocation(getpid(), &client->workout.in_use); 
            vector_subtract(client->workout.in_use.num_plates, client->workout.in_use.num_plates, NUMBER_WEIGHTS);

            Weight *tmp_req = weight_init(req.num_plates);
            removeWeightRequest(getpid(), tmp_req);

            #ifdef VERBOSE
            weight_to_string(&client->workout.in_use, buffer);
            printf("Client %d released allocation\r\n%s\r\n", getpid(), buffer);

            weight_to_string(&req, buffer);
            printf("Client %d release request\r\n%s\r\n", getpid(), buffer);
            #endif // VERBOSE

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
        #ifdef VERBOSE
        printf("Client %d allocation denied\r\n", getpid());
        #endif // VERBOSE

        delay(1*gym->unit_time);
        update_gym(gym);
    }
    return (ret == 0) ? true : false;
}

void client_lift_weights(Gym *gym, Client *client) {
    delay(2*gym->unit_time);
    --client->workout.sets_left;
}

void test_workout_room() {
    test_resource_manager();
    return;
}