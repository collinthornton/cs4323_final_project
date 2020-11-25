// ##########################################
// 
//   Author  -   Collin Thornton
//   Email   -   collin.thornton@okstate.edu
//   Brief   -   Final Project gym resource source
//   Date    -   11-20-20
//
// ########################################## 

#include "resource_manager.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_SIZE 1024

// #define SUB_DELETE_ROW // UNCOMMENT TO DELETE MATRIX ROW WHEN SUBTRACTION = 0

static const char* FILENAME = "data/weight_allocation.txt";
static const char* TMP_FILENAME = "data/weight_allocation.tmp";



static WeightMatrix* weight_matrix_init() {
    WeightMatrix* matrix = malloc(sizeof(WeightMatrix));

    if(matrix == NULL) {
        perror("weight_matrix_init malloc()");
        return NULL;
    }

    matrix->rows = NULL;
    matrix->num_rows = 0;
    return matrix;
}

int weight_matrix_del(WeightMatrix* matrix) {
    if(matrix != NULL) {
        if(matrix->rows != NULL) {
            for(int i=0; i<matrix->num_rows; ++i) {
                if(matrix->rows[i].weight != NULL) free(matrix->rows[i].weight);
            }
            free(matrix->rows);
        }
        free(matrix);
    }

    return 0;
}

static int weight_matrix_add_req(pid_t pid, Weight* weight, WeightMatrix *matrix) {
    WeightMatrixRow* row = weight_matrix_search(pid, matrix, NULL);

    if(row == NULL) {
        // ADD A ROW IF PID NOT FOUND
        if(matrix->num_rows == 0) matrix->rows = (WeightMatrixRow*)malloc(sizeof(WeightMatrixRow));
        else matrix->rows = (WeightMatrixRow*)realloc(matrix->rows, (matrix->num_rows+1)*sizeof(WeightMatrixRow));

        if(matrix->rows == NULL) {
            perror("resourceManager allocate row");
            weight_del(weight);
            return -1;
        }

        ++matrix->num_rows;

        matrix->rows[matrix->num_rows-1].pid = pid;
        matrix->rows[matrix->num_rows-1].weight = weight;

        return matrix->num_rows;
    }

    // ELSE ADD weight TO CURRENT ROW

    for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) {
        row->weight->num_plates[i] += weight->num_plates[i];

    }
    weight_del(weight);

    return matrix->num_rows;
}

static int weight_matrix_sub_req(pid_t pid, Weight *weight, WeightMatrix *matrix) {
    int row_number;
    WeightMatrixRow *row = weight_matrix_search(pid, matrix, &row_number);

    if(row == NULL) {
        weight_del(weight);
        return -1;
    }

    vector_subtract(row->weight->num_plates, weight->num_plates, NUMBER_WEIGHTS);
    if(vector_negative(row->weight->num_plates, NUMBER_WEIGHTS)) {
        perror("weight_matrix_sub_req invalid request");
        weight_del(weight);
        return -1;
    }
    row->weight->total_weight = weight_calc_total_weight(row->weight);
    
    if(row->weight->total_weight >= 0) {
        // FINISHED
        weight_del(weight);
        return matrix->num_rows;
    }

    // ELSE WE NEED TO DELETE ROW AND RESTRUCTURE MATRIX
    #ifdef SUB_DELETE_ROW
    weight_del(weight);
    weight_del(row->weight);

    for(int i=row_number+1; i<matrix->num_rows; ++i) {
        matrix->rows[i-1].pid = matrix->rows[i].pid;
        matrix->rows[i-1].weight = matrix->rows[i].weight;
    }

    --matrix->num_rows;

    if(matrix->num_rows > 0) {
        matrix->rows = (WeightMatrixRow*)realloc(matrix->rows, (matrix->num_rows)*sizeof(WeightMatrixRow));
    } else {
        free(matrix->rows);
        matrix->rows = 0;
    }
    #endif

    return matrix->num_rows;
}

static WeightMatrixRow* weight_matrix_search(pid_t pid, WeightMatrix *matrix, int *row_number) {
    if(matrix == NULL || matrix->rows == NULL) return NULL;

    for(int i=0; i<matrix->num_rows; ++i) {
        if(row_number != NULL) *row_number = i;
        if(matrix->rows[i].pid == pid) return &matrix->rows[i];
    } 

    if(row_number != NULL) *row_number = -1;
    return NULL;
}

const char* weight_matrix_to_string(WeightMatrix *matrix, char buffer[]) {
    buffer[0] = '\0';
    
    for(int i=0; i<matrix->num_rows; ++i) {
        char line[MAX_LINE_SIZE];
        sprintf(buffer+strlen(buffer), "%d,", matrix->rows[i].pid);

        weight_to_string(matrix->rows[i].weight, line);
        sprintf(buffer+strlen(buffer), "%s\n", line);
    }
    return buffer;
}


Weight* getGymResources() {
    Weight *database = getWeightFromFile(0);
    return database;
}

Weight* getAvailableWeights() {
    Weight *total = getGymResources();
    WeightMatrix *allocated = getWeightAllocation();
    
    Weight *total_used = weight_init(NULL);
    for(int i=0; i<allocated->num_rows; ++i) {
        vector_add(total_used->num_plates, allocated->rows[i].weight->num_plates, NUMBER_WEIGHTS);
    }

    vector_subtract(total->num_plates, total_used->num_plates, NUMBER_WEIGHTS);

    weight_matrix_del(allocated);
    weight_del(total_used);
    return total;
}


Weight* getWeightFromFile(unsigned int section) {
    FILE *file = fopen(FILENAME, "r");
    if(file == NULL) {
        perror("getGymResources fopen()");
        return NULL;
    }

    Weight* database = weight_init(NULL);

    char line[MAX_LINE_SIZE] = "\0";

    // THROW OUT FIRST LINE
    fgets(line, MAX_LINE_SIZE-1, file);

    for(int i=0; i<section; ++i) {
        while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
            if(line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
            if(strstr(line, "---") != NULL) break;
        }
    }


    while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
        removeWhiteSpace(line);
        
        if(line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if(strstr(line, "---") != NULL) break;

        #ifdef VERBOSE
        printf("%s", line);
        #endif // VERBOSE

        char *number_plates = strtok(line, ",");
        if(number_plates == NULL) {
            perror("getGymResources inital string token");
            weight_del;
            fclose(file);
            return NULL;
        }

        for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) {
            if(number_plates == NULL && i != FORTY_FIVE) {
                perror("getGymResources field_processing");
                printf("%d\n", i);
                weight_del(database);
                fclose(file);
                return NULL;
            }

            char *end;
            int num_plates = 0;
            float total_weight;

            if((num_plates = strtol(number_plates, &end, 10)) == 0 && errno != 0) {
                perror("getGymResources num_plates");
                weight_del(database);
                fclose(file);
                return NULL;
            }

            database->num_plates[i] = num_plates;

            number_plates = strtok(NULL, ",");
        }

    }   

    fclose(file);

    database->total_weight = weight_calc_total_weight(database);
    return database;
}


WeightMatrix* getWeightAllocation() {
   return getWeightMatrixFromFile(1);
}

WeightMatrix* getWeightRequest() {
    return getWeightMatrixFromFile(2);    
}


static WeightMatrix* getWeightMatrixFromFile(unsigned int section) {
    if(section > 2 || section == 0) {
        perror("getWeightMatrixFromFile section");
        return NULL;
    }

    FILE *file = fopen(FILENAME, "r");
    if(file == NULL) {
        perror("getWeightMatrixFromFile fopen()");
        return NULL;
    }

    WeightMatrix* database = weight_matrix_init();

    char line[MAX_LINE_SIZE];

    for(int i=0; i<section; ++i) {
        while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
            if(line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
            if(strstr(line, "---") != NULL) break;
        }
    }

    while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
        removeWhiteSpace(line);
        
        if(strstr(line, "---") != NULL) break;

        #ifdef VERBOSE
        printf("%s", line);
        #endif // VERBOSE

        Weight* weight = weight_init(NULL);
        pid_t pid;


        char *tmp = strtok(line, ",");
        char *end;

        if(tmp == NULL) {
            perror("getWeightMatrixFromFile pid token");
            weight_del(weight);
            weight_matrix_del(database);
            fclose(file);
            return NULL;
        }
        if((pid = strtol(tmp, &end, 10)) == 0 && errno != 0) {
            perror("getWeightMatrixFromFile pid");
            weight_del(weight);
            weight_matrix_del(database);
            fclose(file);
            return NULL;
        }


        for(int i=TWO_HALF; i<=FORTY_FIVE; ++i) {
            tmp = strtok(NULL, ",");

            if(tmp == NULL) {
                perror("getWeightMatrixFromFile field_processing");
                weight_del(weight);
                weight_matrix_del(database);
                fclose(file);
                return NULL;
            }

            int num_plates = 0;
            float total_weight;

            if((num_plates = strtol(tmp, &end, 10)) == 0 && errno != 0) {
                perror("getWeightMatrixFromFile num_plates");
                weight_del(weight);
                weight_matrix_del(database);
                fclose(file);
                return NULL;
            }

            weight->num_plates[i] = num_plates;
        }
        weight->total_weight = weight_calc_total_weight(weight);
        weight_matrix_add_req(pid, weight, database);

    }   

    fclose(file);
    return database;  
}


int grantWeightRequest(pid_t pid) {
    WeightMatrix *tot_request = getWeightRequest();
    WeightMatrix *tot_allocation = getWeightAllocation();
    Weight *currently_availble = getAvailableWeights();

    int req_row, alloc_row;
    WeightMatrixRow *request = weight_matrix_search(pid, tot_request, &req_row);
    WeightMatrixRow *allocation = weight_matrix_search(pid, tot_allocation, &alloc_row);
    
    if(request == NULL) {
        perror("grantWeightRequest() pid doesn't exit");
        weight_matrix_del(tot_allocation);
        weight_matrix_del(tot_request);
        weight_del(currently_availble);
        return -1;
    }

    vector_add(allocation->weight->num_plates, request->weight->num_plates, NUMBER_WEIGHTS);
    vector_subtract(request->weight->num_plates, request->weight->num_plates, NUMBER_WEIGHTS);
    
    if(vector_less_than_equal(allocation->weight->num_plates, currently_availble->num_plates, NUMBER_WEIGHTS) == false) {
        perror("grantWeightRequest() allocation out of bounds");
        weight_matrix_del(tot_allocation);
        weight_matrix_del(tot_request);
        weight_del(currently_availble);
        return -1;
    }
    
    writeWeightMatrixToFile(tot_allocation, 1);
    writeWeightMatrixToFile(tot_request, 2);

    weight_matrix_del(tot_allocation);
    weight_matrix_del(tot_request);
    weight_del(currently_availble);

    return 0;
}



int writeWeightAllocation(pid_t pid, Weight *weight) {
    WeightMatrix *alloc_matrix = getWeightAllocation();
    WeightMatrix *req_matrix = getWeightRequest();
    Weight *tmp_weight = weight_init(NULL);

    weight_matrix_add_req(pid, weight, alloc_matrix);
    weight_matrix_add_req(pid, tmp_weight, req_matrix);

    int ret = writeWeightMatrixToFile(alloc_matrix, 1);
    writeWeightMatrixToFile(req_matrix, 2);
    weight_matrix_del(alloc_matrix);
    weight_matrix_del(req_matrix);
    return ret;
}
int writeWeightRequest(pid_t pid, Weight *weight) {
    WeightMatrix *req_matrix = getWeightRequest();
    WeightMatrix *alloc_matrix = getWeightAllocation();
    Weight *tmp_weight = weight_init(NULL);

    weight_matrix_add_req(pid, weight, req_matrix);
    weight_matrix_add_req(pid, tmp_weight, alloc_matrix);

    int ret = writeWeightMatrixToFile(req_matrix, 2);
    writeWeightMatrixToFile(alloc_matrix, 1);

    weight_matrix_del(req_matrix);
    weight_matrix_del(alloc_matrix);
    return ret;
}


static int writeWeightMatrixToFile(WeightMatrix *matrix, int section) {
    if(section > 2 || section == 0) {
        perror("writeWeightToFile section");
        return 1;
    }

    FILE *file = fopen(FILENAME, "r");
    FILE *new_file = fopen(TMP_FILENAME, "w");

    short file_flag = 0;
    if(file == NULL) {
        perror("writeWeightToFile fopen()");
        file_flag = 1;
    }
    if(new_file == NULL) {
        perror("writeWeightMatrixToFile fopen()");
        file_flag += 2;
    }

    switch(file_flag) {
        case 0:
            break;
        case 1:
            fclose(new_file);
            return 1;
        case 2:
            fclose(file);
            return 1;
        case 3:
            return 1;
    }

    char line[MAX_LINE_SIZE];

    for(int i=0; i<section; ++i) {
        while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
            fputs(line, new_file);
            if(strcmp(line, "---\n") == 0) break;
        }
    }

    int buff_size = matrix->num_rows*MAX_LINE_SIZE;
    char buffer[buff_size];

    weight_matrix_to_string(matrix, buffer);
    fprintf(new_file, "%s", buffer);
    fputs("---\n", new_file);

    while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
        if(strcmp(line, "---\n") == 0) break;
    }

    while(fgets(line, MAX_LINE_SIZE-1, file) != NULL && !feof(file)) {
        fputs(line, new_file);
    }


    fclose(file);
    fclose(new_file);

    remove(FILENAME);
    rename(TMP_FILENAME, FILENAME);

    return 0;  
}




int removeWeightAllocation(pid_t pid, Weight *weight) {
    WeightMatrix *matrix = getWeightAllocation();

    WeightMatrixRow *row = weight_matrix_search(pid, matrix, NULL);
    if (row == NULL) {
        weight_matrix_del(matrix);
       return -1;
    }

    vector_subtract(row->weight->num_plates, weight->num_plates, NUMBER_WEIGHTS);
    if(vector_negative(row->weight->num_plates, NUMBER_WEIGHTS)) {
        perror("removeWeightAllocation() invalid argument");
        weight_matrix_del(matrix);
        return -1;
    }

    int ret = writeWeightMatrixToFile(matrix, 1);
    weight_matrix_del(matrix);
    return ret;
}
int removeWeightRequest(pid_t pid, Weight *weight) {
    WeightMatrix *matrix = getWeightRequest();
    weight_matrix_sub_req(pid, weight, matrix);
    int ret = writeWeightMatrixToFile(matrix, 2);
    weight_matrix_del(matrix);
    return ret;
}

int clearWeightFile() {
    WeightMatrix *matrix = weight_matrix_init();
    int ret1 = writeWeightMatrixToFile(matrix, 1);
    int ret2 = writeWeightMatrixToFile(matrix, 2);
    weight_matrix_del(matrix);
    if(ret2 < 0) return ret2;
    return ret1;
}


static char* removeWhiteSpace(char* str) {
    const char* d = str;
    do {
        while(*d == ' ' || *d == '\t') {
            ++d;
        }
    } while(*str++ = *d++);
}