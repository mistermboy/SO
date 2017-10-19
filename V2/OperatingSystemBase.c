#include "OperatingSystemBase.h"
#include "OperatingSystem.h"
#include "Processor.h"
#include "Buses.h"
#include "Clock.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Code that students should NOT touch

// Functions prototype
int OperatingSystem_lineBeginsWithANumber(char *);
void OperatingSystem_PrintSleepingProcessQueue();
void OperatingSystem_PrintExecutingProcessInformation();

extern int executingProcessID;

// Search for a free entry in the process table. The index of the selected entry
// will be used as the process identifier
int OperatingSystem_ObtainAnEntryInTheProcessTable() {

	int orig=INITIALPID?rand()%PROCESSTABLEMAXSIZE:INITIALPID;
	int index=0;
	int entry;

	while (index<PROCESSTABLEMAXSIZE) {
		entry = (orig+index)%PROCESSTABLEMAXSIZE;
		if (processTable[entry].busy==0)
			return entry;
		else
			index++;
	}
	return NOFREEENTRY;
}

// Returns the size of the program, stored in the program file
int OperatingSystem_ObtainProgramSize(FILE **programFile, char *program) {

	char lineRead[MAXLINELENGTH];
	int isComment=1;
	int programSize;
	
	*programFile= fopen(program, "r");
	
	// Check if programFile exists
	if (*programFile==NULL)
		return PROGRAMDOESNOTEXIST;

	// Read the first number as the size of the program. Skip all comments.
	while (isComment==1) {
		if (fgets(lineRead, MAXLINELENGTH, *programFile) == NULL)
		    return PROGRAMNOTVALID;
		else
		    if (lineRead[0]!='/' && lineRead[0]!='\n') { // Line IS NOT a comment
			    isComment=0;
			    if (OperatingSystem_lineBeginsWithANumber(lineRead))
					programSize=atoi(strtok(lineRead," "));
			    else
					return PROGRAMNOTVALID;
		    }
	}
	// Only sizes above 0 are allowed
	if (programSize<=0)
	    return PROGRAMNOTVALID;
	else
	    return programSize;
}


// Returns the priority of the program, stored in the program file
int OperatingSystem_ObtainPriority(FILE *programFile) {

	char lineRead[MAXLINELENGTH];
	int isComment=1;
	int processPriority;
	
	// Read the second number as the priority of the program. Skip all comments.
	while (isComment==1) {
		if (fgets(lineRead, MAXLINELENGTH, programFile) == NULL)
			return PROGRAMNOTVALID;
		else
		    if (lineRead[0]!='/' && lineRead[0]!='\n') { // Line IS NOT a comment
			    isComment=0;
			    if (OperatingSystem_lineBeginsWithANumber(lineRead))
					processPriority=atoi(strtok(lineRead," "));
			    else
					return PROGRAMNOTVALID;
		      }
	}
	return processPriority;
}


// Function that processes the contents of the file named by the first argument
// in order to load it in main memory from the address given as the second
// argument
// IT IS NOT NECESSARY TO COMPLETELY UNDERSTAND THIS FUNCTION

int OperatingSystem_LoadProgram(FILE *programFile, int initialAddress, int size) {

	char lineRead[MAXLINELENGTH];
	char *token0, *token1, *token2;
	MEMORYCELL data;
	int nbInstructions = 0;

	Processor_SetMAR(initialAddress);
	while (fgets(lineRead, MAXLINELENGTH, programFile) != NULL) {
		// REMARK: if lineRead is greater than MAXLINELENGTH in number of characters, the program
		// loading does not work
		data.operationCode=' ';data.operand1=data.operand2=0;
		token0=strtok(lineRead," ");
		if (token0!=NULL && token0[0]!='/' && token0[0]!='\n') {
			// I have an instruction with, at least, an operation code
			data.operationCode=tolower(token0[0]);
			token1=strtok(NULL," ");
			if (token1!=NULL && token1[0]!='/') {
				// I have an instruction with, at least, an operand
				data.operand1=atoi(token1);
				token2=strtok(NULL," ");
				if (token2!=NULL && token2[0]!='/') {
					// The read line is similar to 'sum 2 3 //coment'
					// I have an instruction with two operands
					data.operand2=atoi(token2);
				}
			}
			
			// More instructions than size...
			if (++nbInstructions > size){
				return TOOBIGPROCESS;
			}

			Processor_SetMBR(&data);
			// Send data to main memory using the system buses
			Buses_write_DataBus_From_To(CPU, MAINMEMORY);
			Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
			// Tell the main memory controller to write
			MainMemory_writeMemory();
			Processor_SetMAR(Processor_GetMAR()+1);
		}
	}
	return SUCCESS;
}


// Auxiliar for check that line begins with positive number
int OperatingSystem_lineBeginsWithANumber(char * line) {
	int i;
	
	for (i=0; i<strlen(line) && line[i]==' '; i++); // Don't consider blank spaces
	// If is there a digit number...
	if (i<strlen(line) && isdigit(line[i]))
		// It's a positive number
		return 1;
	else
		return 0;
}


void OperatingSystem_ReadyToShutdown(){
		// Simulation must finish (done by modifying the PC of the System Idle Process so it points to its 'halt' instruction,
		// located at the last memory position used by that process, and dispatching sipId (next ShortTermSheduled)
		processTable[sipID].copyOfPCRegister=processTable[sipID].initialPhysicalAddress+processTable[sipID].processSize-1;

}


// Show time messages
void OperatingSystem_ShowTime(char section) {
	ComputerSystem_DebugMessage(6,section,Processor_PSW_BitState(EXECUTION_MODE_BIT)?"\t":"");
	ComputerSystem_DebugMessage(Processor_PSW_BitState(EXECUTION_MODE_BIT)?5:4,section,Clock_GetTime());
}

// Show general status
void OperatingSystem_PrintStatus(){ 
  OperatingSystem_PrintReadyToRunQueue();
  OperatingSystem_PrintSleepingProcessQueue();
  OperatingSystem_PrintExecutingProcessInformation();
 }

 // Show Executing process information
void OperatingSystem_PrintExecutingProcessInformation(){ 
#ifdef SLEEPINGQUEUE

  OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
  // Show message "Running Process Information: [PID: executingProcessID, Priority: priority, WakeUp: whenToWakeUp, Queue: queueID]\n"
  ComputerSystem_DebugMessage(28,SHORTTERMSCHEDULE,
	executingProcessID,processTable[executingProcessID].priority,processTable[executingProcessID].whenToWakeUp
	,processTable[executingProcessID].queueID?"DAEMONS":"USER");

#endif
}

// Show SleepingProcessQueue 
void OperatingSystem_PrintSleepingProcessQueue(){ 
#ifdef SLEEPINGQUEUE

  int i;
  OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
  //  Show message "SLEEPING Queue: ");
  ComputerSystem_DebugMessage(26,SHORTTERMSCHEDULE);
  for (i=0; i< numberOfSleepingProcesses; i++) {
	// Show message [PID, priority, whenToWakeUp]
	ComputerSystem_DebugMessage(27,SHORTTERMSCHEDULE,
		sleepingProcessesQueue[i],processTable[sleepingProcessesQueue[i]].priority,processTable[sleepingProcessesQueue[i]].whenToWakeUp);
	if (i<numberOfSleepingProcesses-1)
	  ComputerSystem_DebugMessage(6,SHORTTERMSCHEDULE,", ");
  }
  ComputerSystem_DebugMessage(6,SHORTTERMSCHEDULE,"\n");
  
#endif
}
