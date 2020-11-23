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

#include "gym.h"


typedef struct {
    pid_t pid;
    Weight *weight;
} WeightMatrixRow;

typedef struct{
    WeightMatrixRow* rows;
    int num_rows;
} WeightMatrix;




WeightMatrix* weight_matrix_init();
int weight_matrix_del(WeightMatrix *matrix);

// Search matrix for pid. If found, adds weight to current row. Else, creates new row.
int weight_matrix_add_req(pid_t pid, Weight *weight, WeightMatrix *matrix);

// Search matrix for pid. If found, subtract weight from current row. Else, return 0
int weight_matrix_sub_req(pid_t pid, Weight *weight, WeightMatrix *matrix);

const char* weight_matrix_to_string(WeightMatrix *matrix, char buffer[]);

WeightMatrixRow* weight_matrix_search(pid_t pid, WeightMatrix *matrix, int *row_number);

Weight* getGymResources();
WeightMatrix* getWeightRequest();
WeightMatrix* getWeightAllocation();
WeightMatrix* getWeightMatrixFromFile(unsigned int section);


int writeWeightRequest(pid_t pid, Weight *weight);
int writeWeightAllocation(pid_t pid, Weight *weight);
int writeWeightMatrixToFile(WeightMatrix *matrix, int section);

int removeWeightRequest(pid_t pid, Weight *weight);
int removeWeightAllocation(pid_t pid, Weight *weight);

char* removeWhiteSpace(char* str);




#endif // RESOURCE_MANAGER_H