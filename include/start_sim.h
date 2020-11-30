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


void start_sim(int num_trainers, int num_couches, bool realistic, bool detect_deadlock, bool fix_deadlock, bool trainer_log);



#endif // PART2_H