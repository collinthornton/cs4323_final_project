// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project workout src
//   Date    -   11-26-20
//
// ########################################## 


#include <stdlib.h>
#include <stdio.h>

#include "workout.h" 


/**
 * @brief Allocate a workout on the heap
 * @param total_sets (int) Total sets in workout
 * @param sets_left (int) Sets left in workout
 * @param total_weight (int) Total weight of workout
 * @param in_use (Weight*) Points to weights currently being used
 */
Workout* workout_init(int total_sets, int sets_left, int total_weight, Weight *weight) {
    Workout *workout = (Workout*)malloc(sizeof(Workout));

    if(workout == NULL) {
        perror("workout_init malloc");
        return NULL;
    }

    workout->total_weight = total_sets;
    workout->sets_left = sets_left;
    workout->total_weight = total_weight;
    if(weight != NULL) {
        workout->in_use = *weight;
    }
    else {
        Weight *tmp_weight = weight_init(NULL);
        workout->in_use = *tmp_weight;
        weight_del(tmp_weight);
    }

    return workout;
}


/**
 * @brief Free a workout from the heap
 * @param workout (Worktout*) workout to be deleted
 * @return (int) return code. negative on failure
 */
int workout_del(Workout *workout) {
    if(workout == NULL) return -1;
    //weight_del(workout->in_use);
    free(workout);
    return 0;
}