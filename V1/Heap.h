#ifndef HEAP_H
#define HEAP_H

#include "OperatingSystem.h"

#define QUEUE_WAKEUP 0
#define QUEUE_PRIORITY 1
#define QUEUE_ARRIVAL 2


int Heap_poll(int[], int, int*);
int Heap_add(int, int[], int , int*, int);
int Heap_compare(int, int, int);

#endif