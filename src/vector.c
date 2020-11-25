// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project vecotr src
//   Date    -   11-24-20
//
// ########################################## 


#include <stdbool.h>
#include <stdlib.h>

#include "vector.h"



/**
 * @brief test if v1 <= v2
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements in vectors. assumed to be same for both
 * @return (bool) true if v1 <= v2. else false
 */
 bool vector_less_than_equal(int *v1, int *v2, int size) {
    if(v1 == NULL || v2 == NULL) return false;
    for(int i=0; i<size; ++i) {
        if(v1[i] > v2[i]) return false;
    }
    return true;
}

/**
 * @brief test if v1 < v2
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements in vectors. assumed to be same for both
 * @return (bool) true if v1 < v2. else false
 */
 bool vector_less_than(int *v1, int *v2, int size) {
    if(v1 == NULL || v2 == NULL) return false;
    for(int i=0; i<size; ++i) {
        if(v2[i] >= v1[i]) return false;
    }
    return true;
}

/**
 * @brief test if v1 == v2
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements in vectors. assumed to be same for both
 * @return (bool) true if v1 == v2. else false
 */
 bool vector_equal(int *v1, int *v2, int size) {
    if(v1 == NULL || v2 == NULL) return false;
    for(int i=0; i<size; ++i) {
        if(v1[i] != v2[i]) return false;
    }
    return true;
}


/**
 * @brief test if v1[i] == 0 for all i < size
 * @param v1 (int*) vector to check
 * @param size (int) number of elements to check
 * @return (bool) true if v1[i] == 0 for all i < size. else false
 */
bool vector_zero(int *v1, int size) {
    if(v1 == NULL) return false;
    for(int i=0; i<size; ++i) if(v1[i] != 0) return false;
    return true;
}


/**
 * @brief check if v1[i] < 0 for any i < size
 * @param v1 (int*) vector to check
 * @param size (int) number of elements to check
 * @return (bool) true of any v1[i] < 0 
 */
bool vector_negative(int *v1, int size) {
    if(v1 == NULL) return false;
    for(int i=0; i<size; ++i) if(v1[i] < 0) return true;
    return false;
}

/**
 * @brief perform v1[i] += v2[i] for all i < size
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements to add
 * @return (int*) v1 = v1 + v2
 */
 int* vector_add(int *v1, int *v2, int size) {
    if(v1 == NULL || v2 == NULL) return false;
    for(int i=0; i<size; ++i) v1[i] += v2[i];
    return v1;
}


/**
 * @brief perform v1[i] -= v2[i] for all i < size
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements to add
 * @return (int*) v1 = v1 - v2
 */
 int* vector_subtract(int *v1, int *v2, int size) {
    if(v1 == NULL || v2 == NULL) return false;
    for(int i=0; i<size; ++i) v1[i] -= v2[i];
    return v1;
}