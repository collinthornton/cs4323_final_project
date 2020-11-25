
gcc -I$1/include/ $1/src/vector.c $1/src/gym.c $1/src/resource_manager.c $1/src/workout_room.c -o $1/build/workout_room -g