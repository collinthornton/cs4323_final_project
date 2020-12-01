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
} Couch;                    //TODO Remove this. It's unused.


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


/**
 * @brief Organize the grip plates
 */
typedef struct {
    int num_plates[NUMBER_WEIGHTS];                    // Use PlateIndex as index for the array (will help w/ keeping track)
    float total_weight;                                // Summation of plate weights
} Weight;




/**
 * @brief Allocate Weight struct on heap
 * @param plate_array (int[NUMBER_WEIGHTS]) Array with each indice a number of grip plates of corresponding PlateIndice. Set to NULL if not yet known
 * @return (Weight*) struct allocated on heap
 */
Weight* weight_init(int plate_array[NUMBER_WEIGHTS]);


/**
 * @brief free a Weight struct from the heap
 * @param weight (Weight*) struct to be freed
 * @return (int) return code. negative on failure
 */
int weight_del(Weight *weight);


/**
 * @brief Calculate the total weight represented by the num_plates array
 * @param weight (Weight*) pointer to a Weight struct
 * @return (float) total weight represented by that struct
 */
float weight_calc_total_weight(Weight *weight);


/**
 * @brief stringify a weight struct
 * @param weight (Weight*) struct to be stringified
 * @param buffer (char[]) buffer to store string
 * @return (const char*) same as buffer
 */
const char* weight_to_string(Weight *weight, char buffer[]);

#endif // GYM_RESOURCES_H