#ifndef HEAP_H
#define HEAP_H

#define QUEUE_WAKEUP 0
#define QUEUE_PRIORITY 1
#define QUEUE_ARRIVAL 2

// Implements the insertion operation in a binary heap. 
// Parameters are:
//    info: PID to be inserted or position in userProgramList
//    heap: the corresponding queue: readyToRun, UserProgramList or sleepingQueue
//    queueType: if sleeping queue, QUEUE_WAKEUP; if ready to run queue, QUEUE_PRIORITY; if userProgramList, QUEUE_ARRIVAL
//    numElem: number of current elements inside the queue, if successfull is increased by one
//    limit: maximum capacity of the queue
// return 0/-1  ok/fail
int Heap_add(int, int[], int , int*, int);

// Implements the extraction operation (the element with the highest priority).
// Parameters are:
//    heap: the corresponding queue: readyToRun, UserProgramList or sleepingQueue
//    queueType: if sleeping queue, QUEUE_WAKEUP; if ready to run queue, QUEUE_PRIORITY; if userProgramList, QUEUE_ARRIVAL
//    numElem: number of current elements inside the queue, if successfull is decremented by one
// Returns: the item with the highest priority in the queue, if everything went ok
int Heap_poll(int[], int, int*);

// Auxiliary function to make comparisons
// Parameters are:
// 	Position one
// 	Position two
// 	queueType: if sleeping queue, QUEUE_WAKEUP; if ready queue, QUEUE_PRIORITY; if arrival, QUEUE_ARRIVAL
int Heap_compare(int, int, int); //

#endif