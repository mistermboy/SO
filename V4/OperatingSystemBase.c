#include "OperatingSystemBase.h"
#include "OperatingSystem.h"
#include "Processor.h"
#include "Buses.h"
#include "ComputerSystemBase.h"
#include "Clock.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Code that students should NOT touch

// Functions prototype
int OperatingSystem_lineBeginsWithANumber(char *);
void OperatingSystem_PrintSleepingProcessQueue();
void OperatingSystem_PrintExecutingProcessInformation();
int OperatingSystem_getBiggestPart();


//Mayor particion
int biggestPart=0;
//Particiones Libres
int freePart=0;


#ifdef MEMCONFIG
PARTITIONDATA partitionsTable[PARTITIONTABLEMAXSIZE];
#endif

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
	int sipIdPCtoShutdown=processTable[sipID].initialPhysicalAddress+processTable[sipID].processSize-1;
		// Simulation must finish (done by modifying the PC of the System Idle Process so it points to its 'halt' instruction,
		// located at the last memory position used by that process, and dispatching sipId (next ShortTermSheduled)
		if (executingProcessID==sipID)
			Processor_CopyInSystemStack(MAINMEMORYSIZE-1, sipIdPCtoShutdown);
		else
			processTable[sipID].copyOfPCRegister=sipIdPCtoShutdown;
}


// Show time messages
void OperatingSystem_ShowTime(char section) {
	ComputerSystem_DebugMessage(6,section,Processor_PSW_BitState(EXECUTION_MODE_BIT)?"\t":"");
	ComputerSystem_DebugMessage(Processor_PSW_BitState(EXECUTION_MODE_BIT)?5:4,section,Clock_GetTime());
}

// Show general status
void OperatingSystem_PrintStatus(){ 
	OperatingSystem_PrintExecutingProcessInformation(); // Show executing process information
	OperatingSystem_PrintReadyToRunQueue();  // Show Ready to run queues implemented for students
	OperatingSystem_PrintSleepingProcessQueue(); // Show Sleeping process queue
	ComputerSystem_PrintArrivalTimeQueue(); // Show arrival queue of user programs
}

 // Show Executing process information
void OperatingSystem_PrintExecutingProcessInformation(){ 
#ifdef SLEEPINGQUEUE

	OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
	if (executingProcessID>=0)
		// Show message "Running Process Information:\n\t\t[PID: executingProcessID, Priority: priority, WakeUp: whenToWakeUp, Queue: queueID]\n"
		ComputerSystem_DebugMessage(28,SHORTTERMSCHEDULE,
			executingProcessID,processTable[executingProcessID].priority,processTable[executingProcessID].whenToWakeUp
			,processTable[executingProcessID].queueID?"DAEMONS":"USER");
	else
		ComputerSystem_DebugMessage(6,SHORTTERMSCHEDULE,"Running Process Information:\n\t\t[--- No running process ---]\n");

#endif
}

// Show SleepingProcessQueue 
void OperatingSystem_PrintSleepingProcessQueue(){ 
#ifdef SLEEPINGQUEUE

	int i;
	OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
	//  Show message "SLEEPING Queue:\n\t\t");
	ComputerSystem_DebugMessage(26,SHORTTERMSCHEDULE);
	if (numberOfSleepingProcesses>0)
		for (i=0; i< numberOfSleepingProcesses; i++) {
			// Show message [PID, priority, whenToWakeUp]
			ComputerSystem_DebugMessage(27,SHORTTERMSCHEDULE,
			sleepingProcessesQueue[i],processTable[sleepingProcessesQueue[i]].priority,processTable[sleepingProcessesQueue[i]].whenToWakeUp);
			if (i<numberOfSleepingProcesses-1)
	  			ComputerSystem_DebugMessage(6,SHORTTERMSCHEDULE,", ");
  		}
  	else 
	  	ComputerSystem_DebugMessage(6,SHORTTERMSCHEDULE,"[--- empty queue ---]");
  ComputerSystem_DebugMessage(6,SHORTTERMSCHEDULE,"\n");
  
#endif
}

// This function returns:
// 		-1 if no programs in arrivalTimeQueue
//		1 if any program arrivalTime is now
//		0 else
// considered by the LTS to create processes at the current time
int OperatingSystem_IsThereANewProgram() {
#ifdef ARRIVALQUEUE
        int currentTime;
		int programArrivalTime;

		if (numberOfProgramsInArrivalTimeQueue <= 0)
		  return -1;  // No new programs in command line list of programs
		
		// Get the current simulation time
        currentTime = Clock_GetTime();
		
		// Get arrivalTime of next program
		programArrivalTime = userProgramsList[arrivalTimeQueue[0]]->arrivalTime; 

		if (programArrivalTime <= currentTime)
		  return 1;  //  There'is new program to start
#endif		 
		return 0;  //  No program in current time
}

// Function to initialize the partition table
// Return number of partitions readed
int OperatingSystem_InitializePartitionTable() {
#ifdef MEMCONFIG
	char lineRead[MAXLINELENGTH];
	FILE *fileMemConfig;
	
	fileMemConfig= fopen(MEMCONFIG, "r");
	if (fileMemConfig==NULL)
		return 0;
	int number = 0;
	// The initial physical address of the first partition is 0
	int initAddress=0;
	int currentPartition=0;
	
	// The file is processed line by line
	while (fgets(lineRead, MAXLINELENGTH, fileMemConfig) != NULL) {
		number=atoi(lineRead);
		// "number" is the size of a just read partition
		partitionsTable[currentPartition].initAddress=initAddress;
		partitionsTable[currentPartition].size=number;
		partitionsTable[currentPartition].occupied=0;
		
		//Le asignamos a biggestPar la particion más grande
		if(partitionsTable[currentPartition].size >= biggestPart){
			biggestPart=partitionsTable[currentPartition].size;
		}
		
		// Next partition will begin at the updated "initAdress"
		initAddress+=number;
		// There is now one more partition
		currentPartition++;
		
		if (currentPartition==PARTITIONTABLEMAXSIZE)
			break;  // No more lines than partitions
	}

	int numOfPartitions = currentPartition;
	for (;currentPartition< PARTITIONTABLEMAXSIZE;currentPartition++)
			partitionsTable[currentPartition].initAddress=-1;

	return numOfPartitions;
#else
	return 0;
#endif
}

// Show partition table
void OperatingSystem_ShowPartitionTable(char *mensaje) {
#ifdef MEMCONFIG
  	int i;
	
	OperatingSystem_ShowTime(SYSMEM);
	ComputerSystem_DebugMessage(40,SYSMEM, mensaje);
	for (i=0;i<PARTITIONTABLEMAXSIZE && partitionsTable[i].initAddress>=0;i++) {
		ComputerSystem_DebugMessage(41,SYSMEM,i,partitionsTable[i].initAddress,partitionsTable[i].size);
		if (partitionsTable[i].occupied)
			ComputerSystem_DebugMessage(42,SYSMEM,partitionsTable[i].PID);
		else
			ComputerSystem_DebugMessage(43,SYSMEM,"AVAILABLE");
	}
#endif
}


int OperatingSystem_getBiggestPart(){
	return biggestPart;
}

