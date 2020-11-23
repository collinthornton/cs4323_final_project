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

// #include "trainer.h"
// #include "client.h"

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
    unsigned short num_plates[8];                               // Use PlateIndex as index for the array (will help w/ keeping track)
    float total_weight;                                  // Summation of plate weights
} Weight;


typedef struct {
    sem_t couch_mutex;      //! MAY BE EASIER TO JUST MAKE A SEMAPHORE
} Couch;



// MAINTAINS THE GYM DATABASE OF RESOURCES

// allocate on heap. Set params as NULL if not available
Weight* weight_init(short plate_array[8]);
int weight_del(Weight *weight);
float weight_calc_total_weight(Weight *weight);
const char* weight_to_string(Weight *weight, char buffer[]);
#endif // GYM_H