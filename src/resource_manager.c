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
#include <unistd.h>


#include <sys/stat.h>

#define MAX_LINE_SIZE 1024

static sem_t *resource_manager_sem;


// #define SUB_DELETE_ROW // UNCOMMENT TO DELETE MATRIX ROW WHEN SUBTRACTION = 0

static const char* FILENAME = "data/weight_allocation.txt";
static const char* TMP_FILENAME = "data/weight_allocation.tmp";


static char RESOURCE_MANAGER_SEM_NAME[] = "/sem_resource_manager";



int init_resource_manager() {
    sem_unlink(RESOURCE_MANAGER_SEM_NAME);
    resource_manager_sem = sem_open(RESOURCE_MANAGER_SEM_NAME, O_CREAT, 0644, 1);
    if(resource_manager_sem == SEM_FAILED) {
        perror("init_resource_manager sem failed to open");
        exit(1);
    }

    mkdir("./data", 0644);

    const char *INITIAL_WRITE = "\n"
        "# WEIGHT ALLOCATION MATRICES FOR GYM\n"
        "# SPECIAL CHARACTERS:\n"
        "#   #  -> Comment. Program ignores line\n"
        "#   ,  -> Delimeter\n"
        "#  --- -> Section divider\n"
        "# SECTION 1 -> AVAILABLE\n"
        "# SECTION 2 -> ALLOCATION\n"
        "# SECTION 3 -> REQUEST\n"
        "# pid,2.5,5,10,15,20,25,35,45\n"
        "\n"
        "# AVAILABLE\n"
        "# 2.5,5,10,15,20,25,35,45\n"
        "\n"
        "12,12,12,12,12,12,12,12\n"
        "---\n"
        "---\n"
        "---\n"
        "\n";

    // TAKE THE SEMAPHORE
    sem_wait(resource_manager_sem);


    // initalize the file
    FILE *file = fopen(FILENAME, "w");
    fputs(INITIAL_WRITE, file);
    fclose(file);

    sem_post(resource_manager_sem);

    return 0;
}

int open_resource_manager() {
    resource_manager_sem = sem_open(RESOURCE_MANAGER_SEM_NAME, O_CREAT, 0644, 1);
    if(resource_manager_sem == SEM_FAILED) {
        perror("init_resource_manager sem failed to open");
        exit(1);
    }    
    return 0;
}

void close_resource_manager() {
    sem_close(resource_manager_sem);
    return;
}

void destroy_resource_manager() {
    sem_unlink(RESOURCE_MANAGER_SEM_NAME);
    return;
}

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
    if(matrix == NULL) {
        perror("weight_matrix_add_req invalid_argument matrix");
        return -1;
    }
    if(weight == NULL) {
        perror("weight_matrix_add_req invalid_argument weight");
        return -1;
    }

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

WeightMatrixRow* weight_matrix_search(pid_t pid, WeightMatrix *matrix, int *row_number) {
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
    sem_wait(resource_manager_sem);
    Weight *total = getGymResources();
    WeightMatrix *allocated = getWeightAllocation();
    
    Weight *total_used = weight_init(NULL);
    for(int i=0; i<allocated->num_rows; ++i) {
        vector_add(total_used->num_plates, allocated->rows[i].weight->num_plates, NUMBER_WEIGHTS);
    }

    vector_subtract(total->num_plates, total_used->num_plates, NUMBER_WEIGHTS);

    weight_matrix_del(allocated);
    weight_del(total_used);
    sem_post(resource_manager_sem);
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
    sem_wait(resource_manager_sem);
    WeightMatrix* ret = getWeightMatrixFromFile(1);
    sem_post(resource_manager_sem);

   return ret;
}

WeightMatrix* getWeightRequest() {
    sem_wait(resource_manager_sem);
    WeightMatrix* ret = getWeightMatrixFromFile(2);
    sem_post(resource_manager_sem);

    return ret;    
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

            errno = 0;
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
    sem_wait(resource_manager_sem);
    WeightMatrix *tot_request = getWeightRequest();
    WeightMatrix *tot_allocation = getWeightAllocation();
    Weight *currently_availble = getAvailableWeights();

    Weight *tmp = weight_init(NULL);
    weight_matrix_add_req(pid, tmp, tot_allocation);

    int req_row, alloc_row;
    WeightMatrixRow *request = weight_matrix_search(pid, tot_request, &req_row);
    WeightMatrixRow *allocation = weight_matrix_search(pid, tot_allocation, &alloc_row);
    
    if(request == NULL) {
        perror("grantWeightRequest() pid doesn't exit");
        weight_matrix_del(tot_allocation);
        weight_matrix_del(tot_request);
        weight_del(currently_availble);
        sem_post(resource_manager_sem);
        return -1;
    }


    vector_add(allocation->weight->num_plates, request->weight->num_plates, NUMBER_WEIGHTS);

    if(vector_less_than_equal(request->weight->num_plates, currently_availble->num_plates, NUMBER_WEIGHTS) == false) {
        //perror("grantWeightRequest() allocation out of bounds");
        weight_matrix_del(tot_allocation);
        weight_matrix_del(tot_request);
        weight_del(currently_availble);
        sem_post(resource_manager_sem);
        return -2;
    }
    vector_subtract(request->weight->num_plates, request->weight->num_plates, NUMBER_WEIGHTS);
    
    writeWeightMatrixToFile(tot_allocation, 1);
    writeWeightMatrixToFile(tot_request, 2);

    weight_matrix_del(tot_allocation);
    weight_matrix_del(tot_request);
    weight_del(currently_availble);

    sem_post(resource_manager_sem);
    return 0;
}



int writeWeightAllocation(pid_t pid, Weight *weight) {
    sem_wait(resource_manager_sem);
    WeightMatrix *alloc_matrix = getWeightAllocation();
    WeightMatrix *req_matrix = getWeightRequest();
    Weight *tmp_weight = weight_init(NULL);

    weight_matrix_add_req(pid, weight, alloc_matrix);
    weight_matrix_add_req(pid, tmp_weight, req_matrix);

    int ret = writeWeightMatrixToFile(alloc_matrix, 1);
    writeWeightMatrixToFile(req_matrix, 2);
    weight_matrix_del(alloc_matrix);
    weight_matrix_del(req_matrix);
    sem_post(resource_manager_sem);
    return ret;
}
int writeWeightRequest(pid_t pid, Weight *weight) {
    sem_wait(resource_manager_sem);
    WeightMatrix *req_matrix = getWeightRequest();
    WeightMatrix *alloc_matrix = getWeightAllocation();
    Weight *tmp_weight = weight_init(NULL);

    weight_matrix_add_req(pid, weight, req_matrix);
    weight_matrix_add_req(pid, tmp_weight, alloc_matrix);

    int ret = writeWeightMatrixToFile(req_matrix, 2);
    writeWeightMatrixToFile(alloc_matrix, 1);

    weight_matrix_del(req_matrix);
    weight_matrix_del(alloc_matrix);
    sem_post(resource_manager_sem);
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
        perror("writeWeightMatrixToFile file");
        file_flag = 1;
    }
    if(new_file == NULL) {
        perror("writeWeightMatrixToFile new_file");
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




int releaseWeightAllocation(pid_t pid, Weight *weight) {
    sem_wait(resource_manager_sem);
    WeightMatrix *matrix = getWeightAllocation();

    WeightMatrixRow *row = weight_matrix_search(pid, matrix, NULL);
    if (row == NULL) {
        weight_matrix_del(matrix);
        sem_post(resource_manager_sem);
       return -1;
    }

    vector_subtract(row->weight->num_plates, weight->num_plates, NUMBER_WEIGHTS);
    if(vector_negative(row->weight->num_plates, NUMBER_WEIGHTS)) {
        perror("removeWeightAllocation() invalid argument");
        weight_matrix_del(matrix);
        sem_post(resource_manager_sem);
        return -1;
    }

    int ret = writeWeightMatrixToFile(matrix, 1);
    weight_matrix_del(matrix);
    sem_post(resource_manager_sem);
    return ret;
}
int removeWeightRequest(pid_t pid, Weight *weight) {
    sem_wait(resource_manager_sem);
    WeightMatrix *matrix = getWeightRequest();
    weight_matrix_sub_req(pid, weight, matrix);
    int ret = writeWeightMatrixToFile(matrix, 2);
    weight_matrix_del(matrix);
    sem_post(resource_manager_sem);
    return ret;
}

int clearWeightFile() {
    WeightMatrix *matrix = weight_matrix_init();
    int ret1 = writeWeightMatrixToFile(matrix, 1);
    int ret2 = writeWeightMatrixToFile(matrix, 2);
    weight_matrix_del(matrix);

    remove(TMP_FILENAME);

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



void test_resource_manager(void) {   
    char line[1024];

    //////////////////////////
    //
    // TEST CLEAR FUNCTION
    //

    clearWeightFile();


    //////////////////////////
    //
    // TEST WRITING FUNCITONS
    //

    pid_t pid = 2;
    int weights[8] = {2, 2, 2, 2, 2, 2, 2, 2};

    printf("Process ID: %d\r\n\r\n", pid);
    

    char in[5];

    // TEST writeWeigthAllocation (section 2 of input file). Deletes weight
    //writeWeightAllocation(pid+4, weight);



    printf("Press enter to write a weight request\r\n");
    fflush(stdin);
    fgets(in, 2, stdin);

    // TEST writeWeightRequest (section 3 of input file). Deletes weight
    Weight *weight = weight_init(weights);
    writeWeightRequest(pid, weight);

    weight = getAvailableWeights();
    weight_to_string(weight, line);
    weight_del(weight);
    printf("currently availble: %s\r\n\r\n\r\n", line);



    printf("Press enter to grant the weight request\r\n");
    fflush(stdin);
    fgets(in, 2, stdin);

    // TEST removeWeightAllocation (section 2 of input file). Deletes weight
    //weight = weight_init(weights);
    grantWeightRequest(pid);

    weight = getAvailableWeights();
    weight_to_string(weight, line);
    weight_del(weight);
    printf("currently availble: %s\r\n\r\n\r\n", line);


    printf("Press enter to remove the weight allocation\r\n");
    fflush(stdin);
    fgets(in, 2, stdin);
    
    
    // TEST removeWeightRequest (section 3 of input file). Deletes weight
    pid_t request_pid = pid;
    weight = weight_init(weights);
    releaseWeightAllocation(request_pid, weight);
    weight_del(weight);

    weight = getAvailableWeights();
    weight_to_string(weight, line);
    weight_del(weight);
    printf("currently availble: %s\r\n\r\n\r\n", line);


    //////////////////////////
    //
    // TEST READING FUNCTIONS
    //

    // TEST getGymResources (section 1 of input file)
    Weight *database = getGymResources();
    if(database == NULL) exit(1);
    weight_to_string(database, line);
    weight_del(database);

    printf("\r\nGYM RESOURCES\r\n\r\n");
    printf("%s\r\n", line);    


    // TEST getWeightAllocation (section 2 of input file)
    WeightMatrix *allocationMatrix = getWeightAllocation();
    if(allocationMatrix == NULL) exit(1);
    weight_matrix_to_string(allocationMatrix, line);
    weight_matrix_del(allocationMatrix);

    printf("\r\n----------\r\n\r\n");
    printf("WEIGHT ALLOCATION\r\n\r\n");
    printf("%s\r\n", line);

    
    // TEST getWeightRequest (section 3 of input file)
    WeightMatrix *requestMatrix = getWeightRequest();
    if(requestMatrix == NULL) exit(1);
    weight_matrix_to_string(requestMatrix, line);
    weight_matrix_del(requestMatrix);

    printf("\r\n----------\r\n\r\n");
    printf("WEIGHT REQUEST\r\n\r\n");
    printf("%s\r\n", line);

    printf("\r\n----------\r\n\r\n");    
}
