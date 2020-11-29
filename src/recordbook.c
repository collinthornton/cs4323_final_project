
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "recordbook.h"

/*Semaphore for the ensuring mutual exclusion*/
sem_t mutual_exclusion;

/* Default file name */
#define DEFAULT_RECORD_FILE "recordbook.txt"

/*Pointer to store name of the file.*/
char *RecordFileName = NULL;


/*Function to add record to the file.
 * File is opened in the append mode, so, that records are added to the end of the file */
void addToRecordBook(struct emp *empValue)
{
    FILE *fp = NULL;

    sem_wait(&mutual_exclusion);
    /*
     * Open the file in the append mode.
     * */
    fp = fopen(RecordFileName, "a");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }
    fwrite( empValue, sizeof( *empValue ), 1, fp );
    fclose(fp);
    sem_post(&mutual_exclusion);
}

/* Display the records stored in the file.
 * */
void displayRecordBook()
{
    FILE *fp = NULL;
    struct emp empVal;
    sem_wait(&mutual_exclusion);
    /*
     * Open the file in the append mode.
     * */
    fp = fopen(RecordFileName, "r");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }

    while (fread(&empVal, sizeof(empVal), 1, fp) ==  1)
    {
        printf("Name = %s \t Id = %d \t Weight = %d \n", empVal.name, empVal.id, empVal.weight);
        memset(&empVal, 0x00, sizeof(empVal));
    }
    fclose(fp);
    sem_post(&mutual_exclusion);
}

/* Opens the file and truncates it, implying all the records are cleared.
 * */
void clearRecordBook()
{
    FILE *fp = NULL;
    sem_wait(&mutual_exclusion);
    fp = fopen(RecordFileName, "w");
    if (fp == NULL)
    {
        printf("Failed to open the file. \n");
    }
    fclose(fp);
    sem_post(&mutual_exclusion);
}

/* The function MUST be called before invoking any of the record logging operation.
 * */
void initRecordBook(char *filename)
{
    sem_init(&mutual_exclusion,0,1);
    if (filename)
    {
        RecordFileName = filename;
    }
    else
    {
        RecordFileName = DEFAULT_RECORD_FILE;
    }
}
