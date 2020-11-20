// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project Gym include
//   Date    -   11-15-20
//
// ########################################## 


#ifndef GYM_H
#define GYM_H

#include <stdbool.h>
#include <semaphore.h>

#include "trainer.h"
#include "client.h"

typedef enum {
    TWO_HALF,
    FIVE,
    TEN,
    FIFTEEN,
    TWENTY,
    TWENTY_FIVE,
    THIRTY_FIVE,
    FORTY_FIVE
} PlateIndex;


typedef struct {
    unsigned short num_plates[8] = {0, 0, 0, 0, 0, 0, 0, 0};    // Use PlateIndex as index for the array (will help w/ keeping track)
    unsigned int total_weight;                                  // Summation of plate weights
    bool in_use;                                                // Whether not plates are begin used (may help w/ deadlock detection)
} Weight;


typedef struct {
    sem_t couch_mutex;      //! MAY BE EASIER TO JUST MAKE A SEMAPHORE
} Couch;



// MAINTAINS THE GYM DATABASE OF RESOURCES


#endif // GYM_H