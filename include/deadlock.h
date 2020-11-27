// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project deadlock detection include
//   Date    -   11-23-20
//
// ########################################## 


#ifndef DEADLOCK_DETECTION_H
#define DEADLOCK_DETECTION_H

#include <stdbool.h>

#include "resource_manager.h"


/**
 * @brief Check the input file for deadlocked processes.
 * @param deadlocked_array (pid_t[]) array with length of number of processes (matrix rows)
 * @return (int) number of deadlocked processes
 */
int checkForDeadlock(pid_t deadlock_array[]);


void test_deadlock_detection(void);




#endif // DEADLOCK_DEFINITION_H