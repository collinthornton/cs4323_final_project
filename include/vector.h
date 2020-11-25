// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project vector include
//   Date    -   11-24-20
//
// ########################################## 

#ifndef VECTOR_H
#define VECTOR_H


/**
 * @brief test if v1 <= v2
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements in vectors. assumed to be same for both
 * @return (bool) true if v1 <= v2. else false
 */
bool vector_less_than_equal(int *v1, int *v2, int size);

/**
 * @brief test if v1 < v2
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements in vectors. assumed to be same for both
 * @return (bool) true if v1 < v2. else false
 */
bool vector_less_than(int *v1, int *v2, int size);

/**
 * @brief test if v1 == v2
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements in vectors. assumed to be same for both
 * @return (bool) true if v1 == v2. else false
 */
bool vector_equal(int *v1, int *v2, int size);


/**
 * @brief test if v1[i] == 0 for all i < size
 * @param v1 (int*) vector to check
 * @param size (int) number of elements to check
 * @return (bool) true if v1[i] == 0 for all i < size. else false
 */
bool vector_zero(int *v1, int size);

/**
 * @brief check if v1[i] < 0 for any i < size
 * @param v1 (int*) vector to check
 * @param size (int) number of elements to check
 * @return (bool) true of any v1[i] < 0 
 */
bool vector_negative(int *v1, int size);



/**
 * @brief perform v1[i] += v2[i] for all i < size
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements to add
 * @return (int*) v1 = v1 + v2
 */
int* vector_add(int *v1, int *v2, int size);


/** @brief perform v1[i] -= v2[i] for all i < size
 * @param v1 (int*) left vector
 * @param v2 (int*) right vector
 * @param size (int) number of elements to add
 * @return (int*) v1 = v1 - v2
 */
int* vector_subtract(int *v1, int *v2, int size);



#endif // VECTOR_H