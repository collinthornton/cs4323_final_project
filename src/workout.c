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


Workout* workout_init(int total_sets, int sets_left, int total_weight, Weight *weight) {
    Workout *workout = (Workout*)malloc(sizeof(Workout));

    if(workout == NULL) {
        perror("workout_init malloc");
        return NULL;
    }

    workout->total_weight = total_sets;
    workout->sets_left = sets_left;
    workout->total_weight = total_weight;
    workout->in_use = weight;

    return workout;
}


int workout_del(Workout *workout) {
    if(workout == NULL) return -1;
    weight_del(workout->in_use);
    free(workout);
    return 0;
}