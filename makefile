INCLUDE_DIR = ./include
BUILD_DIR = ./build
SRC_DIR = ./src

CC=gcc
CFLAGS=-I$(INCLUDE_DIR) -g

_DEPS = client.h deadlock.h gym_resources.h gym.h resource_manager.h trainer.h vector.h workout_room.h workout.h start_sim.h recordbook.h
DEPS = $(patsubset %,$(INCLUDE_DIR)/%,$(_DEPS))

_OBJ = client.o deadlock.o gym_resources.o gym.o resource_manager.o trainer.o vector.o workout_room.o workout.o start_sim.o recordbook.o
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(_OBJ))

LIBS = -lrt -pthread


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


part1: $(BUILD_DIR)/part1.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

part2: $(BUILD_DIR)/part2.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

part3: $(BUILD_DIR)/part3.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test_deadlock_rollback: $(BUILD_DIR)/test_deadlock_rollback.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

test_recordbook: $(BUILD_DIR)/test_recordbook.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) 


.PHONY: clean

clean:
	rm -f $(BUILD_DIR)/*.o part1 part2 part3 test_recordbook test_deadlock_rollback