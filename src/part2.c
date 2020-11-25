#include "entrance.h"
#include "workout_room.h"

void start_part2(){
    //First open the gym, but do not use semaphores
    //open_gym(3,3,5,0);
    test_resource_manager();
}


int main(int argc, char **argv) {
    start_part2();
}