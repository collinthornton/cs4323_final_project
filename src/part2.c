#include <stdlib.h>
#include <stdio.h>

#include "resource_manager.h"


int main(int argc, char** argv) {
    Weight *database;
    if((database = getGymResources()) == NULL) {
        exit(1);
    }



    printf("2.5 plates -> %i\r\n", database->num_plates[TWO_HALF]);
    printf("5.0 plates -> %i\r\n", database->num_plates[FIVE]);
    printf("%i\r\n", database->num_plates[TEN]);
    printf("%i\r\n", database->num_plates[TWENTY]);
    printf("%i\r\n", database->num_plates[TWENTY_FIVE]);
    printf("%i\r\n", database->num_plates[THIRTY_FIVE]);
    printf("%i\r\n", database->num_plates[FORTY_FIVE]);

    free(database);

}