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

// Store data for a single workout
typedef struct {
    int total_sets;
    int sets_left;
    
    int total_weight;
    Weight in_use;
} Workout;



//////////////////////////////
//
// Workout functions
//


/**
 * @brief Allocate a workout on the heap
 * @param total_sets (int) Total sets in workout
 * @param sets_left (int) Sets left in workout
 * @param total_weight (int) Total weight of workout
 * @param in_use (Weight*) Points to weights currently being used
 */
Workout* workout_init(int total_sets, int sets_left, int total_weight, Weight *in_use);


/**
 * @brief Free a workout from the heap
 * @param workout (Worktout*) workout to be deleted
 * @return (int) return code. negative on failure
 */
int workout_del(Workout *workout);

#endif // WORKOUT_H