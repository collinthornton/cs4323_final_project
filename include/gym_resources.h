// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project Gym include
//   Date    -   11-15-20
//
// ########################################## 

#ifndef GYM_RESOURCES_H
#define GYM_RESOURCES_H

#include <semaphore.h>

#define NUMBER_WEIGHTS 8

#define MIN_COUCHES 3
#define MAX_COUCHES 6


typedef struct {
    sem_t couch_mutex;      //! MAY BE EASIER TO JUST MAKE A SEMAPHORE
} Couch;


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
    int num_plates[NUMBER_WEIGHTS];                    // Use PlateIndex as index for the array (will help w/ keeping track)
    float total_weight;                                // Summation of plate weights
} Weight;




// allocate on heap. Set params as NULL if not available
Weight* weight_init(int plate_array[NUMBER_WEIGHTS]);
int weight_del(Weight *weight);
float weight_calc_total_weight(Weight *weight);
const char* weight_to_string(Weight *weight, char buffer[]);

#endif // GYM_RESOURCES_H