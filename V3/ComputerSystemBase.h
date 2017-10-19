#ifndef COMPUTERSYSTEMBASE_H
#define COMPUTERSYSTEMBASE_H

#include "ComputerSystem.h"

// Functions prototypes
void ComputerSystem_FillInArrivalTimeQueue();
void ComputerSystem_PrintArrivalTimeQueue();


// This "extern" declarations enables other source code files to gain access to the variables 
extern int numberOfProgramsInArrivalTimeQueue;
extern int arrivalTimeQueue[USERPROGRAMSMAXNUMBER];

#endif
