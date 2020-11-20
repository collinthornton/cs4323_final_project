// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project gym resource source
//   Date    -   11-20-20
//
// ########################################## 

#include "resource_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char* FILENAME = "data/weights.txt";
static const int MAX_LINE_SIZE = 1024;

Weight* getGymResources() {
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL) {
        perror("getGymResources fopen()");
        return NULL;
    }

    Weight* database = (Weight*)malloc(sizeof(Weight));

    char line[MAX_LINE_SIZE];
    while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
        removeWhiteSpace(line, strlen(line));
        
        if(line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if(strstr(line, "---") != NULL) break;

        #ifdef VERBOSE
        printf("%s", line);
        #endif // VERBOSE

        char *weight = strtok(line, ",");
        char *number_plates = strtok(NULL, ",");

        if (weight == NULL || number_plates == NULL) {
            perror("getGymResources field_processing");
            free(database);
            return NULL;
        }

        char* end;
        int num_plates;
        if((num_plates = strtol(number_plates, &end, 10)) == 0 && errno != 0) {
            perror("getGymResources number_plates");
            free(database);
            return NULL;
        }

        float total_weight = 0;
        if(strcmp(weight, "2.5") == 0) {
            database->num_plates[TWO_HALF] = num_plates;
            total_weight += num_plates*2.5;
        }
        else if(strcmp(weight, "5") == 0) {
            database->num_plates[FIVE] = num_plates;
            total_weight += num_plates*5;
        }
        else if(strcmp(weight, "10") == 0) {
            database->num_plates[TEN] = num_plates;
            total_weight += num_plates*10;
        }
        else if(strcmp(weight, "15") == 0) {
            total_weight += num_plates*15;
            database->num_plates[FIFTEEN] = num_plates;
        }
        else if(strcmp(weight, "20") == 0) { 
            database->num_plates[TWENTY] = num_plates;
            total_weight += num_plates*20;
        }
        else if(strcmp(weight, "25") == 0) { 
            database->num_plates[TWENTY_FIVE] = num_plates;
            total_weight += num_plates*25;
        }
        else if(strcmp(weight, "35") == 0) { 
            database->num_plates[THIRTY_FIVE] = num_plates;
            total_weight += num_plates*35;
        }
        else if(strcmp(weight, "45") == 0) { 
            database->num_plates[FORTY_FIVE] = num_plates;
            total_weight += num_plates*45;
        }

        database->total_weight = total_weight;

    }   

    return database; 
}


char* removeWhiteSpace(char* str, int n) {
    const char* d = str;
    do {
        while(*d == ' ' || *d == '\t') {
            ++d;
        }
    } while(*str++ = *d++);
}