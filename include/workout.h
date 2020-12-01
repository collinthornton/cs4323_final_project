// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project workout include
//   Date    -   11-26-20
//
// ########################################## 



#ifndef WORKOUT_H
#define WORKOUT_H

#include "gym_resources.h"

#define MAX_WEIGHT 500
#define MIN_WEIGHT 100

typedef struct {
    int total_sets;
    int sets_left;
    
    int total_weight;
    Weight in_use;
} Workout;


// allocate workout on heap
Workout* workout_init(int total_sets, int sets_left, int total_weight, Weight *in_use);

// free workout 
int workout_del(Workout *workout);





#endif // WORKOUT_H