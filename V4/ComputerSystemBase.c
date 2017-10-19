#include "ComputerSystem.h"
#include "ComputerSystemBase.h"
#include "Heap.h"
#include "OperatingSystemBase.h"

// Functions prototypes


// Fill ArrivalTimeQueue heap with user program from parameters
void ComputerSystem_FillInArrivalTimeQueue() {
#ifdef ARRIVALQUEUE
	int i=0;

	while (userProgramsList[i]!=NULL && i<USERPROGRAMSMAXNUMBER) {
	  Heap_add(i,arrivalTimeQueue,QUEUE_ARRIVAL,&i,USERPROGRAMSMAXNUMBER);
	}
	numberOfProgramsInArrivalTimeQueue=i;
#endif
}

// Print arrivalTiemQueue program information
void ComputerSystem_PrintArrivalTimeQueue(){
#ifdef ARRIVALQUEUE
	int i;

	OperatingSystem_ShowTime(LONGTERMSCHEDULE);
	// Show message "Arrival Time Queue:\n"
	ComputerSystem_DebugMessage(29,LONGTERMSCHEDULE);
	if (numberOfProgramsInArrivalTimeQueue>0) 
		for (i=0; i< numberOfProgramsInArrivalTimeQueue; i++) {
			// Show message: \t\t[executableName,arrivalTime]\n
			ComputerSystem_DebugMessage(30,LONGTERMSCHEDULE,userProgramsList[arrivalTimeQueue[i]]->executableName,userProgramsList[arrivalTimeQueue[i]]->arrivalTime);
		}
	else 
 		ComputerSystem_DebugMessage(6,LONGTERMSCHEDULE,"\t\t[--- empty queue ---]\n");
#endif
}
  
