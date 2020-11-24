Course: CS4323
Name: Robert Cook
CWID: A10073520
Assignment: Group Final Project
Execution Instructions: Run the following command line in the directory where all submitted files are extracted:

gcc simulator.c -o simulator -lrt

Sample Input: No iput

Sample Output:
-----------------Scenario 1: Deadlock----------------
CUSTOMER 1: Customer has shared gym. Here are the contents:
TRAINER: Trainer has shared gym. Here are the contents:
CUSTOMER 1: NumberCustomersArriving: 0
CUSTOMER 1: NumberTrainersAvailable: 0
CUSTOMER 1: NumberCustomersWaiting: 0
CUSTOMER 1: No trainers available, customer traveling to waiting room
TRAINER: NumberCustomersArriving: 0
TRAINER: NumberTrainersAvailable: 0
TRAINER: NumberCustomersWaiting: 0
TRAINER: No customers in the waiting room
TRAINER: Trainer now available, waiting for customers
CUSTOMER 1: Customer now in waiting room