#include <stdio.h>
#include <string.h>
#include "recordbook.h"

int main() {
    initRecordBook("recordbook.txt");
    struct emp empVal;
    memset(&empVal, 0x00, sizeof(empVal));
    memcpy(&empVal.name,"testName", 10);
    empVal.id  = 1;
    empVal.weight = 50;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    addToRecordBook(&empVal);
    empVal.weight++;
    displayRecordBook();
    clearRecordBook();
    displayRecordBook();
}
