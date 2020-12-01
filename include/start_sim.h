// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project start include
//   Date    -   11-30-20
//
// ########################################## 


#ifndef PART2_H
#define PART2_H

#include <stdbool.h>

#include "workout_room.h"
#include "deadlock.h"
#include "recordbook.h"


/**
 * @brief Startup the sim with flags. Used in driver files
 * @param num_trainer (int) total number of trainers in simulation
 * @param num_couches (int) max clients in waiting room
 * @param boundary_case (const bool) solve for part b
 * @param realistc (const bool) toggle realistic weight algorithm in client
 * @param detect_deadlock (const bool) solve for part c
 * @param fix_deadlock (const bool) solve for part d
 * @param trainer_log (const bool) solve for part e
 */
void start_sim(int num_trainers, int num_couches, const bool boundary_case, const bool realistic, 
    const bool detect_deadlock, const bool fix_deadlock, const bool trainer_log);



#endif // PART2_H