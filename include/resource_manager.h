// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project gym resource include
//   Date    -   11-15-20
//
// ########################################## 



#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <semaphore.h>

#include "gym.h"


typedef struct {
    pid_t pid;
    Weight *weight;
} WeightMatrixRow;

typedef struct{
    WeightMatrixRow* rows;
    int num_rows;
} WeightMatrix;


//////////////////////////////
//
//  USER FUNCTIONS
//

int init_resource_manager();
int open_resource_manager();
void close_resource_manager();
void destroy_resource_manager();

// FUNCTIONS TO GET WEIGHT REQUESTS FROM FILE
/**
 * @brief return the gym's total resources. must be deleted with weight_del()
 * @return (Weight*) Vector of total weights
 */
Weight* getGymResources();

/**
 * @brief return currently available weights
 * @return (Weight*) vector of current weight
 */
Weight* getAvailableWeights();


/**
 * @brief return the current weight requests. deleted with weight_matrix_del
 * @return (WeightMatrix*) matrix of requests allocated on heap
 */
WeightMatrix* getWeightRequest();


/**
 * @brief return the currently allocated weights. deleted with weight_matrix_del
 * @return (WeightMatrix*) matrix of allocations allocated on heap
 */
WeightMatrix* getWeightAllocation();


/**
 * @brief removes the request, allocates weights, and adjusts the currently available weights
 * @param pid (pid_t) process to grant
 * @return (int) return code. negative on error
 */
int grantWeightRequest(pid_t pid);

/**
 * @brief removes the allocation and adjusts currently available weights
 * @param pid (pid_t) process id to adjust
 * @param weight (Weight*) amount to change
 * @return (int) return code. negative on error
 */
int releaseWeightAllocation(pid_t pid, Weight* weight);





/**
 * @brief Free a WeightMatrix*, and all internal WeightRows* and Weights*
 * @param matrix (WeightMatrix*) matrix to be deleted
 * @return (int) 0 on success
 */
int weight_matrix_del(WeightMatrix *matrix);

// FUNCTIONS TO WRITE WEIGHT REQUESTS TO FILE

/**
 * @brief write a new request to the file
 * @param pid (pid_t) pid of process
 * @param weight (Weight*) new request
 * @return (int) negative on failure
 */
int writeWeightRequest(pid_t pid, Weight *weight);


/**
 * @brief write a new allocation to the file
 * @param pid (pid_t) pid of process
 * @param weight (Weight*) new allocation
 * @return (int) negative on failure
 */
int writeWeightAllocation(pid_t pid, Weight *weight);


// FUNCTIONS TO REMOVE WEIGHT REQUEST FROM FILE

/**
 * @brief remove a request from the file. will throw error if result is negative
 * @param pid (pid_t) pid of process
 * @param weight (Weight*) weight to be subtracted
 * @return (int) negative on failure
 */
int removeWeightRequest(pid_t pid, Weight *weight);


/**
 * @brief remove an allocation from the file. will throw error if result is negative
 * @param pid (pid_t) pid of process
 * @param weight (Weight*) weight to be subtracted
 * @return (int) negative on failure
 */
int removeWeightAllocation(pid_t pid, Weight *weight);


/**
 * @brief clear allocation and request matrices from file
 * @return (int) negative on failure
 */
int clearWeightFile();


/**
 * @brief Return a string representative of matrix
 * @param matrix (WeightMatrix*) matrix to be returned as string
 * @param buffer (char[]) buffer to store string output
 * @return (const char*) pointer to string. same as buffer
 */
const char* weight_matrix_to_string(WeightMatrix *matrix, char buffer[]);



//////////////////////////////
//
//  HELPER FUNCTIONS
//


/**
 * @brief Initailize a WeightMatrix on heap. All values NULL or 0
 * @return (WeightMatrix*) pointer to new matrix
 */
static WeightMatrix* weight_matrix_init();



/**
 * @brief Search a WeightMatrix for a pid
 * @param pid (pid_t) pid for which to search
 * @param matrix (WeightMatrix*) matrix to be searched
 * @param row_number (int*) storage for the row number. negative if row not found. can be set to NULL if not neededd
 * @return (WeightMatrixRow*) pointer to the row. NULL if not found
 */
WeightMatrixRow* weight_matrix_search(pid_t pid, WeightMatrix *matrix, int *row_number);


/**
 * @brief Add a weight request to a weight matrix. Will add new row if pid is not found
 * @param pid (pid_t) pid of requesting process
 * @param weight (Weight*) weight to be added
 * @param matrix (WeightMatrix*) matrix to store summation
 * @return (int) number of rows in matrix. Negative on error
 */
static int weight_matrix_add_req(pid_t pid, Weight *weight, WeightMatrix *matrix);


/**
 * @brief Subtract a weight request from a weight matrix. Will delete a row if result is 0 vector
 * @param pid (pid_t) pid of process
 * @param weight (Weight*) weight to be subtracted
 * @param matrix (WeightMatrix*) matrix to store difference
 * @return (int) nuber of rows in matrix. Negative on error
 */
static int weight_matrix_sub_req(pid_t pid, Weight *weight, WeightMatrix *matrix);


/**
 * @brief read a weight matrix from the input file
 * @param section (unsigned int) section number from which to read
 * @return (WeightMatrix*) pointer to weight matrix on heap
 */
static WeightMatrix* getWeightMatrixFromFile(unsigned int section);


/**
 * @brief write a weight matrix to the input file. will delete the matrix
 * @param matrix (WeightMatrix*) matrix to be written
 * @param section (int) section number at which to write
 * @return (int) negative on failure
 */
static int writeWeightMatrixToFile(WeightMatrix *matrix, int section);


/**
 * @brief get a weight from the input file
 * @param section (unsigned int) section number from which to read
 * @return (Weight*) allocation on heap
 */
static Weight* getWeightFromFile(unsigned int section);

/**
 * @brief remove all whitespace from a string
 * @param str (char*) input string. will be changed
 * @return (const char*) output string. same as str
 */
static char* removeWhiteSpace(char* str);


void test_resource_manager();

#endif // RESOURCE_MANAGER_H