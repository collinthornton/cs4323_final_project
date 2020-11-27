# Part 2 README

## part2.c

Test code for the resource manager, focusing on the generation of an internal weight database from the communal file. Will later be used to launch the simulator in Part2 of the assignment description.

methods
void test_resource_manager(void)
int main(int argc, char** argv)



## resource_manager.c

Manage I/O with the gym's weight databse.

methods
Weight* getGymResources(void) 
char* removeWhiteSpace(char *str)



## gym.c

Maintain struts for the gym's resources, including weights and couches.

Structs
Weight
	-Unsigned short num_plates[8] - Array of plates indexed with the PlateIndex enumeration. Values indicate the number of plates in use. Eg. num_plates[FIVE] = 2 indicates that 2 plates of weight 5 are in use.
	-Float total_weight - total weight of plates in num_plates.


methods
Weight* weight_init(short plate_array[8], float total_weight)
int weight_del(Weight* weight)

