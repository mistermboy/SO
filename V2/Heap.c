#include <stdlib.h>
#include "Heap.h"
#include "OperatingSystem.h"

// Internal Functions prototypes
void Heap_swap_Up(int, int[], int);
void Heap_swap_Down(int, int[], int, int);


// Insertion of a PID into a binary heap
// info: PID to insert
// heap: Binary heap to insert: user o daemon ready queue, sleeping queue, ...
// queueType: QUEUE_PRIORITY, QUEUE_WAKEUP, QUEUE_ARRIVAL, ...
// numElem: number of elements actually into the queue, if successfull is increased by one
// limit: max size of the queue
// return 0/-1  ok/fail
int Heap_add(int info, int heap[], int queueType, int *numElem, int limit) {
	if (*numElem >= limit || info<0)
		return -1;
	heap[*numElem]=info;
	Heap_swap_Up(*numElem, heap, queueType);
	(*numElem)++;
	return 0;
}

// Extract the more priority item
// heap: Binary heap to extract: user o daemon ready queue, sleeping queue, ...
// queueType: QUEUE_PRIORITY, QUEUE_WAKEUP, QUEUE_ARRIVAL, ...
// numElem: number of elements actually into the queue, if successfull is decremented by one
// return more priority item into the queue
int Heap_poll(int heap[], int queueType, int *numElem) {
	int info = heap[0];
	if (*numElem==0)
		return -1; // no hay elementos en la cola de prioridad
	else {
		heap[0]=heap[*numElem-1];
		Heap_swap_Down(0, heap, queueType, *numElem);
		(*numElem)--;
	}
	return info;		
}

//  Auxiliar function for implementation of binary heaps
void Heap_swap_Up(int p, int heap[], int queueType) {
	if (p > 0)  { // if not at the top...
		int padre = abs(p - 1) / 2; // integer operation
		if (Heap_compare(heap[p],heap[padre],queueType)>0) { // less than father...
			int aux = heap[padre]; 
			heap[padre] = heap[p];
			heap[p] = aux;
			Heap_swap_Up(padre, heap, queueType);
		} // if not less, don't switch
	}  // at the top...
}

//  Auxiliar function for implementation of binary heaps
void Heap_swap_Down(int p, int heap[], int queueType, int numElem) {
	int izq = 2*p+1;
	int der = 2*p+2;
	int aux = heap[p];
	
	if (der < numElem) //  2 childs... 
		if ((Heap_compare(heap[izq],heap[der], queueType)>0) && (Heap_compare(heap[p],heap[izq],queueType)<0)){ // Switch with left-child if rigth-child greather
			heap[p] = heap[izq];
			heap[izq] = aux;
			Heap_swap_Down(izq, heap, queueType, numElem);
		} else { // rigth 
			if (Heap_compare(heap[p],heap[der], queueType)<0) { // Switch with rigth-child
				heap[p] = heap[der];
				heap[der] = aux;
				Heap_swap_Down(der, heap, queueType, numElem);
			}
		}
	else if (izq<numElem) { // only left-child...
		if (Heap_compare(heap[p],heap[izq], queueType)<0){ // Switch with left-child
			heap[p] = heap[izq];
			heap[izq] = aux;
			Heap_swap_Down(izq, heap, queueType, numElem);
		} // Less than left-child, don't switch
	} // leaf-node...
}

// Auxiliar for priority comparations
int Heap_compare_priority(int value1, int value2) {
  return processTable[value2].priority-processTable[value1].priority;
}

// Auxiliar for  WakeUp-time comparations
int Heap_compare_wakeup(int value1, int value2) {
#ifdef SLEEPINGQUEUE
	return processTable[value2].whenToWakeUp - processTable[value1].whenToWakeUp;
#else
	return 0;
#endif
}

// Auxiliar for arrival-time comparations
int Heap_compare_arrival(int value1, int value2) {
  return userProgramsList[value2]->arrivalTime - userProgramsList[value1]->arrivalTime;
}


// Auxiliar for generic comparations
int Heap_compare(int value1, int value2, int queueType) {
  
  switch (queueType) {
	case QUEUE_PRIORITY:
	  return Heap_compare_priority(value1, value2);
	case QUEUE_WAKEUP:
	  return Heap_compare_wakeup(value1, value2);
	case QUEUE_ARRIVAL:
	  return Heap_compare_arrival(value1, value2);
	default:
	  return 0; // 
  }
}


