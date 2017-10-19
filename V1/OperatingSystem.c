#include "OperatingSystem.h"
#include "OperatingSystemBase.h"
#include "MMU.h"
#include "Processor.h"
#include "Buses.h"
#include "Heap.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// Functions prototypes
void OperatingSystem_CreateDaemons();
void OperatingSystem_PCBInitialization(int, int, int, int, int);
void OperatingSystem_MoveToTheREADYState(int);
void OperatingSystem_Dispatch(int);
void OperatingSystem_RestoreContext(int);
void OperatingSystem_SaveContext(int);
void OperatingSystem_TerminateProcess();
int OperatingSystem_LongTermScheduler();
void OperatingSystem_PreemptRunningProcess();
int OperatingSystem_CreateProcess(USER_PROGRAMS_DATA);
int OperatingSystem_ObtainMainMemory(int, int);
int OperatingSystem_ShortTermScheduler();
int OperatingSystem_ExtractFromReadyToRun();
void OperatingSystem_HandleException();
void OperatingSystem_HandleSystemCall();
void OperatingSystem_PrintReadyToRunQueue();

//Estados de los procesos
char * statesNames [5]={"NEW","READY","EXECUTING","BLOCKED","EXIT"}; 

// The process table
PCB processTable[PROCESSTABLEMAXSIZE];

// Address base for OS code
int OS_address_base = PROCESSTABLEMAXSIZE * MAINMEMORYSECTIONSIZE;
// Identifier of the current executing process
int executingProcessID=NOPROCESS;

// Identifier of the System Idle Process
int sipID;

// Array that contains the identifiers of the READY processes
int readyToRunQueue[NUMBEROFQUEUES][PROCESSTABLEMAXSIZE];
int numberOfReadyToRunProcesses[NUMBEROFQUEUES];

// Variable containing the number of not terminated user processes
int numberOfNotTerminatedUserProcesses=0;

//Variable que controla si hay que crear un proceso de usuario o de sistema
int isDemon=0;



// Initial set of tasks of the OS
void OperatingSystem_Initialize() {
	
	int i, selectedProcess;
	int numberOfSuccessfullyCreatedProcesses=0;
	FILE *programFile; // For load Operating System Code

	// Obtain the memory requirements of the program
	int processSize=OperatingSystem_ObtainProgramSize(&programFile, "OperatingSystemCode");

	// Load Operating System Code
	OperatingSystem_LoadProgram(programFile, OS_address_base, processSize);
	
	// Initialize random feed
	srand(time(NULL));

	// Process table initialization (all entries are free)
	for (i=0; i<PROCESSTABLEMAXSIZE;i++)
		processTable[i].busy=0;
	
	// Initialization of the interrupt vector table of the processor
	Processor_InitializeInterruptVectorTable(OS_address_base);
	
	
	// Create all system daemon processes
	OperatingSystem_CreateDaemons();
	
	if (sipID<0) {
		// Show message "ERROR: Missing SIP program!\n"
		ComputerSystem_DebugMessage(21,SHUTDOWN);
		exit(1);
	}
	
	// Create all user processes from the information given in the command line
	numberOfSuccessfullyCreatedProcesses=OperatingSystem_LongTermScheduler();
	if(numberOfSuccessfullyCreatedProcesses==0){
		OperatingSystem_ReadyToShutdown();
	}
	
	// At least, one user process has been created
	// Select the first process that is going to use the processor
	selectedProcess=OperatingSystem_ShortTermScheduler();
	
	// Assign the processor to the selected process
	OperatingSystem_Dispatch(selectedProcess);

	// Initial operation for Operating System
	Processor_SetPC(OS_address_base);
}


// Daemon processes are system processes, that is, they work together with the OS.
// The System Idle Process uses the CPU whenever a user process is able to use it
void OperatingSystem_CreateDaemons() {
  
	USER_PROGRAMS_DATA systemIdleProcess;
	
	systemIdleProcess.executableName="SystemIdleProcess";
	isDemon=1;
	sipID=OperatingSystem_CreateProcess(systemIdleProcess); 
	processTable[sipID].copyOfPCRegister=processTable[sipID].initialPhysicalAddress;
	processTable[sipID].copyOfPSWRegister=Processor_GetPSW();
}


// The LTS is responsible of the admission of new processes in the system.
// Initially, it creates a process from each program specified in the command line
int OperatingSystem_LongTermScheduler() {
  
	int PID, i,
		numberOfSuccessfullyCreatedProcesses=0;
	
	for (i=0 ; userProgramsList[i]!=NULL && i<USERPROGRAMSMAXNUMBER ; i++) {
		PID=OperatingSystem_CreateProcess(*userProgramsList[i]);
		numberOfSuccessfullyCreatedProcesses++;
		//Si el PID es igual a NOFREENTRY decrementamos el numberOfSuccessfullyCreatedProcesses porque 
		//el proceso no se ha creado correctamente
		if(PID==NOFREEENTRY){
			ComputerSystem_DebugMessage(103,ERROR,userProgramsList[i]->executableName);
			numberOfSuccessfullyCreatedProcesses--;
		}
		else if(PID==PROGRAMDOESNOTEXIST){
			ComputerSystem_DebugMessage(104,ERROR,userProgramsList[i]->executableName," it does not exist");
			numberOfSuccessfullyCreatedProcesses--;
		}
		else if(PID==PROGRAMNOTVALID){
			ComputerSystem_DebugMessage(104,ERROR,userProgramsList[i]->executableName,"invalid priority or size");
			numberOfSuccessfullyCreatedProcesses--;
		}
		else if(PID==TOOBIGPROCESS){
			ComputerSystem_DebugMessage(105,ERROR,userProgramsList[i]->executableName);
			numberOfSuccessfullyCreatedProcesses--;
		}
		else{
		// Show message "Process [PID] created from program [executableName]\n"
		ComputerSystem_DebugMessage(22,INIT,PID,userProgramsList[i]->executableName);
		}
	}
	numberOfNotTerminatedUserProcesses+=numberOfSuccessfullyCreatedProcesses;

	// Return the number of succesfully created processes
	return numberOfSuccessfullyCreatedProcesses;
}


// This function creates a process from an executable program
int OperatingSystem_CreateProcess(USER_PROGRAMS_DATA executableProgram) {
  
	int PID;
	int processSize;
	int loadingPhysicalAddress;
	int priority;
	int queueID=isDemon; //Cola a la que pertenece el proceso
	FILE *programFile;

	// Obtain a process ID
	PID=OperatingSystem_ObtainAnEntryInTheProcessTable();
	//Si OperatingSystem_ObtainAnEntryInTheProcessTable() fracasa devolvemos NOFREENTRY
	if(PID==NOFREEENTRY){
		return PID;
	}

	// Obtain the memory requirements of the program
	processSize=OperatingSystem_ObtainProgramSize(&programFile, executableProgram.executableName);
	//Si el tamaño del programa no es válido entra en la condición
	if(processSize==PROGRAMNOTVALID){
		return processSize;
	}
	//Si el nombre del programa no es válido entra en la condición
	if(processSize==PROGRAMDOESNOTEXIST){
		return processSize;
	}

	// Obtain the priority for the process
	priority=OperatingSystem_ObtainPriority(programFile);
	
	// Obtain enough memory space
 	int isBig=loadingPhysicalAddress=OperatingSystem_ObtainMainMemory(processSize, PID);
	if(isBig==TOOBIGPROCESS){
		return isBig;
	}
	// Load program in the allocated memory
	int big = OperatingSystem_LoadProgram(programFile, loadingPhysicalAddress, processSize);
	if(big==TOOBIGPROCESS){
		return big;
	}
	
	//Controlamos la cola a la que quiere acceder
	if(isDemon==1){
		queueID=1;
		isDemon=0;
	}
	// PCB initialization
	OperatingSystem_PCBInitialization(PID, loadingPhysicalAddress, processSize, priority, queueID);
	
	// Move process to the ready state
	OperatingSystem_MoveToTheREADYState(PID);
	
	return PID;
}


// Main memory is assigned in chunks. All chunks are the same size. A process
// always obtains the chunk whose position in memory is equal to the processor identifier
int OperatingSystem_ObtainMainMemory(int processSize, int PID) {

 	if (processSize>MAINMEMORYSECTIONSIZE)
		return TOOBIGPROCESS;
	
 	return PID*MAINMEMORYSECTIONSIZE;
}


// Assign initial values to all fields inside the PCB
void OperatingSystem_PCBInitialization(int PID, int initialPhysicalAddress, int processSize, int priority,int queue) {

	processTable[PID].busy=1;
	processTable[PID].initialPhysicalAddress=initialPhysicalAddress;
	processTable[PID].processSize=processSize;
	processTable[PID].state=NEW;
	ComputerSystem_DebugMessage(111,SYSPROC,PID,statesNames[0]);
	processTable[PID].priority=priority;
	processTable[PID].copyOfPCRegister=0;
	processTable[PID].copyOfPSWRegister=0;
	processTable[PID].queueID=queue;
	processTable[PID].copyOfAcumulator=0;
}


// Move a process to the READY state: it will be inserted, depending on its priority, in
// a queue of identifiers of READY processes
void OperatingSystem_MoveToTheREADYState(int PID) {
	int cola;
	if(processTable[PID].queueID==0){
		cola=0;
	}else if(processTable[PID].queueID==1){
		cola=1;
	}
	
	if (Heap_add(PID, readyToRunQueue[cola],QUEUE_PRIORITY ,&numberOfReadyToRunProcesses[cola] ,PROCESSTABLEMAXSIZE)>=0) {
		ComputerSystem_DebugMessage(110,SYSPROC,PID,statesNames[processTable[PID].state],statesNames[1]);
		processTable[PID].state=READY;
	} 
	OperatingSystem_PrintReadyToRunQueue();
}


// The STS is responsible of deciding which process to execute when specific events occur.
// It uses processes priorities to make the decission. Given that the READY queue is ordered
// depending on processes priority, the STS just selects the process in front of the READY queue
int OperatingSystem_ShortTermScheduler() {
	
	int selectedProcess;

	selectedProcess=OperatingSystem_ExtractFromReadyToRun();
	
	return selectedProcess;
}


// Return PID of more priority process in the READY queue
int OperatingSystem_ExtractFromReadyToRun() {
	int selectedProcess=NOPROCESS;
	int cola=1;
	int exit=0;
	int i;
	for(i=0;i <numberOfReadyToRunProcesses[0];i++){
		if(readyToRunQueue[0][i] >= 0 && exit==0){
			cola=0;
			exit=1;
			}
		}
	
	selectedProcess=Heap_poll(readyToRunQueue[cola],QUEUE_PRIORITY ,&numberOfReadyToRunProcesses[cola]);
	
	// Return most priority process or NOPROCESS if empty queue
	return selectedProcess; 
}


// Function that assigns the processor to a process
void OperatingSystem_Dispatch(int PID) {

	// The process identified by PID becomes the current executing process
	executingProcessID=PID;
	// Change the process' state
	ComputerSystem_DebugMessage(110,SYSPROC,executingProcessID,statesNames[processTable[executingProcessID].state],statesNames[2]);
	processTable[PID].state=EXECUTING;
	// Modify hardware registers with appropriate values for the process identified by PID
	OperatingSystem_RestoreContext(PID);
	
	
}


// Modify hardware registers with appropriate values for the process identified by PID
void OperatingSystem_RestoreContext(int PID) {
  
	// New values for the CPU registers are obtained from the PCB
	Processor_CopyInSystemStack(MAINMEMORYSIZE-1,processTable[PID].copyOfPCRegister);
	Processor_CopyInSystemStack(MAINMEMORYSIZE-2,processTable[PID].copyOfPSWRegister);
	Processor_SetAccumulator(processTable[PID].copyOfAcumulator);
	
	// Same thing for the MMU registers
	MMU_SetBase(processTable[PID].initialPhysicalAddress);
	MMU_SetLimit(processTable[PID].processSize);
}


// Function invoked when the executing process leaves the CPU 
void OperatingSystem_PreemptRunningProcess() {

	// Save in the process' PCB essential values stored in hardware registers and the system stack
	OperatingSystem_SaveContext(executingProcessID);
	// Change the process' state
	OperatingSystem_MoveToTheREADYState(executingProcessID);
	// The processor is not assigned until the OS selects another process
	executingProcessID=NOPROCESS;
}


// Save in the process' PCB essential values stored in hardware registers and the system stack
void OperatingSystem_SaveContext(int PID) {
	
	// Load PC saved for interrupt manager
	processTable[PID].copyOfPCRegister=Processor_CopyFromSystemStack(MAINMEMORYSIZE-1);
	
	// Load PSW saved for interrupt manager
	processTable[PID].copyOfPSWRegister=Processor_CopyFromSystemStack(MAINMEMORYSIZE-2);
	
	//Load acumulator saved for interrupt manager
	processTable[PID].copyOfAcumulator=Processor_GetAccumulator();
}


// Exception management routine
void OperatingSystem_HandleException() {
  
	// Show message "Process [executingProcessID] has generated an exception and is terminating\n"
	ComputerSystem_DebugMessage(23,SYSPROC,executingProcessID);
	
	OperatingSystem_TerminateProcess();
}


// All tasks regarding the removal of the process
void OperatingSystem_TerminateProcess() {
  
	int selectedProcess;
	ComputerSystem_DebugMessage(110,SYSPROC,executingProcessID,statesNames[processTable[executingProcessID].state],statesNames[4]);
	processTable[executingProcessID].state=EXIT;
	
	// One more process that has terminated
	numberOfNotTerminatedUserProcesses--;
	
	if (numberOfNotTerminatedUserProcesses<=0) {
		// Simulation must finish 
		OperatingSystem_ReadyToShutdown();
	}
	// Select the next process to execute (sipID if no more user processes)
	selectedProcess=OperatingSystem_ShortTermScheduler();
	// Assign the processor to that process
	OperatingSystem_Dispatch(selectedProcess);
}


// System call management routine
void OperatingSystem_HandleSystemCall() {
  
	int systemCallID;
	int queue=processTable[executingProcessID].queueID;
	int oldProcess=executingProcessID;
	int processToRun=readyToRunQueue[queue][0];

	// Register A contains the identifier of the issued system call
	systemCallID=Processor_GetRegisterA();
	
	switch (systemCallID) {
		case SYSCALL_PRINTEXECPID:
			// Show message: "Process [executingProcessID] has the processor assigned\n"
			ComputerSystem_DebugMessage(24,SYSPROC,executingProcessID);
			break;

		case SYSCALL_END:
			// Show message: "Process [executingProcessID] has requested to terminate\n"
			ComputerSystem_DebugMessage(25,SYSPROC,executingProcessID);
			OperatingSystem_TerminateProcess();
			break;
			
		case SYSCALL_YIELD:			
				if(processTable[processToRun].priority==processTable[executingProcessID].priority){					
					OperatingSystem_PreemptRunningProcess();	
					OperatingSystem_Dispatch(OperatingSystem_ShortTermScheduler());					
					ComputerSystem_DebugMessage(115,SHORTTERMSCHEDULE,oldProcess,processToRun);
				}
			break;
			
	}
}
	
//	Implement interrupt logic calling appropriate interrupt handle
void OperatingSystem_InterruptLogic(int entryPoint){
	switch (entryPoint){
		case SYSCALL_BIT: // SYSCALL_BIT=2
			OperatingSystem_HandleSystemCall();
			break;
		case EXCEPTION_BIT: // EXCEPTION_BIT=6
			OperatingSystem_HandleException();
			break;
	}

}
 //Muestra por pantalla el contenido de la tabla de listos y la prioridad de cada proceso
 void OperatingSystem_PrintReadyToRunQueue(){
	 int i;
	 int exit=1; 
	 ComputerSystem_DebugMessage(106,SHORTTERMSCHEDULE);
	 //Recorremos la cola de usuarios
	 ComputerSystem_DebugMessage(200,SHORTTERMSCHEDULE);
	 for(i=0;i <numberOfReadyToRunProcesses[0];i++){   
		 int pid = readyToRunQueue[0][i];
		 if(exit==1){
			ComputerSystem_DebugMessage(107,SHORTTERMSCHEDULE,pid,processTable[pid].priority);
			exit=0;
		 }else{
			 ComputerSystem_DebugMessage(108,SHORTTERMSCHEDULE,pid,processTable[pid].priority);
		 }	 
	 }
	 ComputerSystem_DebugMessage(109,SHORTTERMSCHEDULE);
	 //Recorremos la cola del sistema
	  ComputerSystem_DebugMessage(201,SHORTTERMSCHEDULE);
	 exit=1;
	 for(i=0;i <numberOfReadyToRunProcesses[1];i++){   
		 int pid = readyToRunQueue[1][i];
		 if(exit==1){
			ComputerSystem_DebugMessage(112,SHORTTERMSCHEDULE,pid,processTable[pid].priority);
			exit=0;
		 }else{
			 ComputerSystem_DebugMessage(113,SHORTTERMSCHEDULE,pid,processTable[pid].priority);
		 }	 
	 }
	 ComputerSystem_DebugMessage(109,SHORTTERMSCHEDULE);
 }

