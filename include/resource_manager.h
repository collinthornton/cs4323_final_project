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

Weight* getGymResources();

int writeWeightRequest(const char* req);
int writeWeightCheckout(const char* req);

int removeWeightRequest(const char* req);
int removeWeightCheckout(const char* req);

char* removeWhiteSpace(char* str, int n);




#endif // RESOURCE_MANAGER_H