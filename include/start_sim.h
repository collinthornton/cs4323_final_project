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

#include "entrance.h"
#include "workout_room.h"
#include "deadlock.h"
#include "recordbook.h"


void start_sim(int num_trainers, int num_couches, const bool boundary_case, const bool realistic, 
    const bool detect_deadlock, const bool fix_deadlock, const bool trainer_log);



#endif // PART2_H