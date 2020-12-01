
#ifndef RECORDKEEPING_RECORDBOOK_H
#define RECORDKEEPING_RECORDBOOK_H

#include <pthread.h>

#define MAX_NAME_LEN 100


typedef struct emp
{
    char name[MAX_NAME_LEN];
    int id;
    int weight;
} Emp;

void addToRecordBook(struct emp *empValue);
void displayRecordBook();
void clearRecordBook();
void initRecordBook();
void openRecordBook();
void closeRecordBook();
void destroyRecordBook();

#endif //RECORDKEEPING_RECORDBOOK_H
