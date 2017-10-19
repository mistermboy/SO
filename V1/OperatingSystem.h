#ifndef OPERATINGSYSTEM_H
#define OPERATINGSYSTEM_H

#include "ComputerSystem.h"
#include <stdio.h>


#define SUCCESS 1
#define PROGRAMDOESNOTEXIST -1
#define PROGRAMNOTVALID -2

#define MAXLINELENGTH 150

#define PROCESSTABLEMAXSIZE 4

#define INITIALPID 0

// In this version, every process occupies a 60 positions main memory chunk 
// so we can use 16 positions for the system stack
#define MAINMEMORYSECTIONSIZE 60

#define NOFREEENTRY -3
#define TOOBIGPROCESS -4

#define NOPROCESS -1

#define NUMBEROFQUEUES 2 
#define USERPROCESSQUEUE 0 
#define DAEMONSQUEUE 1 

// Enumerated type containing all the possible process states
enum ProcessStates { NEW, READY, EXECUTING, BLOCKED, EXIT};

// Enumerated type containing the list of system calls and their numeric identifiers
enum SystemCallIdentifiers { SYSCALL_END=3, SYSCALL_PRINTEXECPID=5, SYSCALL_YIELD=4};

// A PCB contains all of the information about a process that is needed by the OS
typedef struct {
	int busy;
	int initialPhysicalAddress;
	int processSize;
	int state;
	int priority;
	int copyOfPCRegister;
	unsigned int copyOfPSWRegister;
	int copyOfAcumulator;
	int queueID;
} PCB;

// These "extern" declaration enables other source code files to gain access
// to the variable listed
extern PCB processTable[PROCESSTABLEMAXSIZE];
extern int OS_address_base;
extern int sipID;

// Functions prototypes
void OperatingSystem_Initialize();
void OperatingSystem_InterruptLogic(int);


#endif
