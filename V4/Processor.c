#include "Processor.h"
#include "OperatingSystem.h"
#include "Buses.h"
#include "MMU.h"
#include <stdio.h>
#include <string.h>
#include "Clock.h"

// Internals Functions prototypes

void Processor_FetchInstruction();
void Processor_DecodeAndExecuteInstruction();
void Processor_ManageInterrupts();
void Processor_RaiseInterrupt(const unsigned int);
void Processor_ACKInterrupt(const unsigned int);
unsigned int Processor_GetInterruptLineStatus(const unsigned int);
void Processor_ActivatePSW_Bit(const unsigned int);
void Processor_DeactivatePSW_Bit(const unsigned int);
void Processor_UpdatePSW();
void Processor_CheckOverflow(int,int);
void Processor_SetRegisterA(int);
int Processor_GetAccumulator();
void Processor_SetAccumulator(int);
int Processor_GetAccumulator();
void Processor_Show(char);
void OperatingSystem_HandleClockInterrupt();
void OperatingSystem_PrintStatus();
int OperatingSystem_GetExecutingProcessID();
void Processor_RaiseException(int);



// void Processor_SetRegisterA(int);

// Processor registers
int registerPC_CPU; // Program counter
int registerAccumulator_CPU; // Accumulator
MEMORYCELL registerIR_CPU; // Instruction register
unsigned int registerPSW_CPU = 128; // Processor state word, initially protected mode
int registerMAR_CPU; // Memory Address Register
MEMORYCELL registerMBR_CPU; // Memory Buffer Register

int registerA_CPU; // General purpose register
int registerB_CPU;

int interruptLines_CPU; // Processor interrupt lines

// interrupt vector table: an array of handle interrupt memory addresses routines  
int interruptVectorTable[INTERRUPTTYPES];

// For PSW show "--X---FNZS"
char pswmask []="----------"; 

char * PSW_BITS_names [INTERRUPTTYPES]={"POWEROFF_BIT","ZERO_BIT","NEGATIVE_BIT","OVERFLOW_BIT","UNUSED","UNUSED","UNUSED","EXECUTION_MODE_BIT","UNUSED","UNUSED"};

// Initialization of the interrupt vector table
void Processor_InitializeInterruptVectorTable(int interruptVectorInitialAddress) {
	int i;
	for (i=0; i< INTERRUPTTYPES;i++)
		interruptVectorTable[i]=interruptVectorInitialAddress+1+2*i;

}


// This is the instruction cycle loop (fetch, decoding, execution, etc.).
// The processor stops working when an POWEROFF signal is stored in its
// PSW register
void Processor_InstructionCycleLoop() {

	while (!Processor_PSW_BitState(POWEROFF_BIT)) {
		Processor_FetchInstruction();
		Processor_DecodeAndExecuteInstruction();
		if (interruptLines_CPU && !Processor_PSW_BitState(INTERRUPT_MASKED_BIT) )
			Processor_ManageInterrupts();
	}
}

// Fetch an instruction from main memory and put it in the IR register
void Processor_FetchInstruction() {

	// The instruction must be located at the logical memory address pointed by the PC register
	registerMAR_CPU=registerPC_CPU;
	// Send to the MMU the address in which the reading has to take place: use the address bus for this
	Buses_write_AddressBus_From_To(CPU, MMU);
	// Tell the main memory controller to read
	if (MMU_readMemory()==MMU_SUCCESS) {
	  // All the read data is stored in the MBR register. Because it is an instruction
	  // we have to copy it to the IR register
	  memcpy((void *) (&registerIR_CPU), (void *) (&registerMBR_CPU), sizeof(MEMORYCELL));
	// Show initial part of HARDWARE message with Operation Code and operands
	  // Show message: operationCode operand1 operand2
		Processor_Show(HARDWARE);
		ComputerSystem_DebugMessage(1, HARDWARE, registerIR_CPU.operationCode, registerIR_CPU.operand1, registerIR_CPU.operand2);
	}
	else 
	  // Show message: "_ _ _ "
	  ComputerSystem_DebugMessage(20,HARDWARE);
}


// Decode and execute the instruction in the IR register
void Processor_DecodeAndExecuteInstruction() {
	int tempAcc; // for save accumulator if necesary

	Processor_DeactivatePSW_Bit(OVERFLOW_BIT);

	switch (registerIR_CPU.operationCode) {
	  
		// Instruction ADD
		case 'a':
			registerAccumulator_CPU= registerIR_CPU.operand1 + registerIR_CPU.operand2;	
			Processor_CheckOverflow(registerIR_CPU.operand1,registerIR_CPU.operand2);
			registerPC_CPU++;
			break;
		
		// Instruction MEMADD
		case 'm':
			registerMAR_CPU=registerIR_CPU.operand2;
			Buses_write_AddressBus_From_To(CPU,MMU);
			MMU_readMemory();
			registerAccumulator_CPU=registerIR_CPU.operand1 + registerMBR_CPU.operand1;
			registerPC_CPU++;
			break;
		
		// Instruction SUB
		case 's':
			registerAccumulator_CPU= registerIR_CPU.operand1 - registerIR_CPU.operand2;
			Processor_CheckOverflow(registerIR_CPU.operand1,-registerIR_CPU.operand2);
			registerPC_CPU++;
			break;
		
		// Instruction DIV
		case 'd':
			if (registerIR_CPU.operand2 == 0)
				Processor_RaiseException(DIVISIONBYZERO);
			else {
				registerAccumulator_CPU=registerIR_CPU.operand1 / registerIR_CPU.operand2;
				registerPC_CPU++;
			}
			break;
			  
		// Instruction TRAP
		case 't':
			Processor_RaiseInterrupt(SYSCALL_BIT);
			registerA_CPU=registerIR_CPU.operand1;
			registerPC_CPU++;
			break;
		
		// Instruction NOP
		case 'n':
			registerPC_CPU++;
			break;
			  
		// Instruction JUMP
		case 'j':
			registerPC_CPU+= registerIR_CPU.operand1;
			break;
			  
		// Instruction ZJUMP
		case 'z':  // Jump if ZERO_BIT on
			if (Processor_PSW_BitState(ZERO_BIT))
				registerPC_CPU+= registerIR_CPU.operand1;
			else
				registerPC_CPU++;
			break;

		// Instruction WRITE
		case 'w':
			registerMBR_CPU.operationCode= registerMBR_CPU.operand1= registerMBR_CPU.operand2= registerAccumulator_CPU;
			registerMAR_CPU=registerIR_CPU.operand1;
			// Send to the main memory controller the data to be written: use the data bus for this
			Buses_write_DataBus_From_To(CPU, MAINMEMORY);
			// Send to the main memory controller the address in which the writing has to take place: use the address bus for this
			Buses_write_AddressBus_From_To(CPU, MMU);
			// Tell the main memory controller to write
			MMU_writeMemory();
			registerPC_CPU++;
			break;

		// Instruction READ
		case 'r':
			registerMAR_CPU=registerIR_CPU.operand1;
			// Send to the main memory controller the address in which the reading has to take place: use the address bus for this
			Buses_write_AddressBus_From_To(CPU, MMU);
			// Tell the main memory controller to read
			MMU_readMemory();
			// Copy the read data to the accumulator register
			registerAccumulator_CPU= registerMBR_CPU.operand1;
			registerPC_CPU++;
			break;

		// Instruction INC
		case 'i':
			tempAcc=registerAccumulator_CPU;
			registerAccumulator_CPU+= registerIR_CPU.operand1;
			Processor_CheckOverflow(tempAcc,registerIR_CPU.operand1);
			registerPC_CPU++;
			break;

		// Instruction HALT
		case 'h':
			if(Processor_PSW_BitState(EXECUTION_MODE_BIT)){
			Processor_ActivatePSW_Bit(POWEROFF_BIT);
			}else{
				Processor_RaiseException(INVALIDPROCESSORMODE);
			}
			break;
			  
		// Instruction OS
		case 'o': // Make a operating system routine in entry point indicated by operand1
			if(Processor_PSW_BitState(EXECUTION_MODE_BIT)){
			// Show final part of HARDWARE message with CPU registers
			// Show message: " (PC: registerPC_CPU, Accumulator: registerAccumulator_CPU, PSW: registerPSW_CPU [Processor_ShowPSW()]\n
			ComputerSystem_DebugMessage(130, HARDWARE,OperatingSystem_GetExecutingProcessID(),registerPC_CPU,registerAccumulator_CPU,registerPSW_CPU,Processor_ShowPSW());
			// Not all operating system code is executed in simulated processor, but really must do it... 
			OperatingSystem_InterruptLogic(registerIR_CPU.operand1);
			registerPC_CPU++;
			// Update PSW bits (ZERO_BIT, NEGATIVE_BIT, ...)
			Processor_UpdatePSW();
			return; // Note: message show before... for operating system messages after...
			}else{
				Processor_RaiseException(INVALIDPROCESSORMODE);
			}
			break;
		// Instruction IRET
		case 'y': // Return from a interrupt handle manager call
			if(Processor_PSW_BitState(EXECUTION_MODE_BIT)){
				registerPC_CPU=Processor_CopyFromSystemStack(MAINMEMORYSIZE-1);
				registerPSW_CPU=Processor_CopyFromSystemStack(MAINMEMORYSIZE-2);
			}else{
				Processor_RaiseException(INVALIDPROCESSORMODE);
			}
			break;		

		// Unknown instruction
		default : 
			Processor_RaiseException(INVALIDINSTRUCTION);
			registerA_CPU++;
			break;
	}
	
	// Update PSW bits (ZERO_BIT, NEGATIVE_BIT, ...)
	Processor_UpdatePSW();
	
	// Show final part of HARDWARE message with	CPU registers
	// Show message: " (PC: registerPC_CPU, Accumulator: registerAccumulator_CPU, PSW: registerPSW_CPU [Processor_ShowPSW()]\n
	ComputerSystem_DebugMessage(130,HARDWARE,OperatingSystem_GetExecutingProcessID(),registerPC_CPU,registerAccumulator_CPU,registerPSW_CPU,Processor_ShowPSW());
}
	
	
// Hardware interrupt processing
void Processor_ManageInterrupts() {
  
	int i;

		for (i=0;i<INTERRUPTTYPES;i++)
			// If an 'i'-type interrupt is pending
			if (Processor_GetInterruptLineStatus(i)) {
				// Deactivate interrupt
				Processor_ACKInterrupt(i);
				// Copy PC and PSW registers in the system stack
				Processor_CopyInSystemStack(MAINMEMORYSIZE-1, registerPC_CPU);
				Processor_CopyInSystemStack(MAINMEMORYSIZE-2, registerPSW_CPU);	
				// Activate protected excution mode
				Processor_ActivatePSW_Bit(INTERRUPT_MASKED_BIT);
				Processor_ActivatePSW_Bit(EXECUTION_MODE_BIT);
				
				// Call the appropriate OS interrupt-handling routine setting PC register
				registerPC_CPU=interruptVectorTable[i];
				break; // Don't process another interrupt
			}
}

// Update PSW state
void Processor_UpdatePSW(){
	// Update ZERO_BIT
	if (registerAccumulator_CPU==0){
		if (!Processor_PSW_BitState(ZERO_BIT))
			Processor_ActivatePSW_Bit(ZERO_BIT);
	}
	else {
		if (Processor_PSW_BitState(ZERO_BIT))
			Processor_DeactivatePSW_Bit(ZERO_BIT);
	}
	
	// Update NEGATIVE_BIT
	if (registerAccumulator_CPU<0) {
		if (!Processor_PSW_BitState(NEGATIVE_BIT))
			Processor_ActivatePSW_Bit(NEGATIVE_BIT);
	}
	else {
		if (Processor_PSW_BitState(NEGATIVE_BIT))
			Processor_DeactivatePSW_Bit(NEGATIVE_BIT);
	}
	
}

// Check overflow, receive operands for add (if sub, change operand2 sign)
void Processor_CheckOverflow(int op1, int op2) {
			if ((op1>0 && op2>0 && registerAccumulator_CPU<0)
				|| (op1<0 && op2<0 && registerAccumulator_CPU>0))
				Processor_ActivatePSW_Bit(OVERFLOW_BIT);
}

// Save in the system stack a given value
void Processor_CopyInSystemStack(int physicalMemoryAddress, int data) {

	registerMBR_CPU.operationCode=registerMBR_CPU.operand1=registerMBR_CPU.operand2=data;
	registerMAR_CPU=physicalMemoryAddress;
	Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
	Buses_write_DataBus_From_To(CPU, MAINMEMORY);	
	MainMemory_writeMemory();
}

// Get value from system stack
int Processor_CopyFromSystemStack(int physicalMemoryAddress) {

	registerMAR_CPU=physicalMemoryAddress;
	Buses_write_AddressBus_From_To(CPU, MAINMEMORY);
	MainMemory_readMemory();
	return registerMBR_CPU.operand1;
}


// Put the specified interrupt line to a high level 
void Processor_RaiseInterrupt(const unsigned int interruptNumber) {
	unsigned int mask = 1;

	mask = mask << interruptNumber;
	interruptLines_CPU = interruptLines_CPU | mask;
}

// Put the specified interrupt line to a low level 
void Processor_ACKInterrupt(const unsigned int interruptNumber) {
	unsigned int mask = 1;

	mask = mask << interruptNumber;
	mask = ~mask;

	interruptLines_CPU = interruptLines_CPU & mask;
}

// Returns the state of a given interrupt line (1=high level, 0=low level)
unsigned int Processor_GetInterruptLineStatus(const unsigned int interruptNumber) {
	unsigned int mask = 1;

	mask = mask << interruptNumber;
	return (interruptLines_CPU & mask) >> interruptNumber;
}

// Set a given bit position in the PSW register
void Processor_ActivatePSW_Bit(const unsigned int nbit) {
	unsigned int mask = 1;

	mask = mask << nbit;
	
	registerPSW_CPU = registerPSW_CPU | mask;
}

// Unset a given bit position in the PSW register
void Processor_DeactivatePSW_Bit(const unsigned int nbit) {
	unsigned int mask = 1;

	mask = mask << nbit;
	mask = ~mask;

	registerPSW_CPU = registerPSW_CPU & mask;
}

// Returns the state of a given bit position in the PSW register
unsigned int Processor_PSW_BitState(const unsigned int nbit) {
	unsigned int mask = 1;

	mask = mask << nbit;
	return (registerPSW_CPU & mask) >> nbit;
}

// Getter for the registerMAR_CPU
int Processor_GetMAR() {
  return registerMAR_CPU;
}

// Setter for the registerMAR_CPU
void Processor_SetMAR(int data) {
  registerMAR_CPU=data;
}

// pseudo-getter for the registerMBR_CPU
void Processor_GetMBR(MEMORYCELL *toRegister) {
  memcpy((void*) toRegister, (void *) (&registerMBR_CPU), sizeof(MEMORYCELL));
}

// pseudo-setter for the registerMBR_CPU
void Processor_SetMBR(MEMORYCELL *fromRegister) {
  memcpy((void*) (&registerMBR_CPU), (void *) fromRegister, sizeof(MEMORYCELL));
}

// pseudo-getter for the registerMBR_CPU value
int Processor_GetMBR_Value(){
  return registerMBR_CPU.operationCode;
}

//Setter for the Accumulator
 void Processor_SetAccumulator(int acc){
   registerAccumulator_CPU=acc;
 }

// Setter for the PC
void Processor_SetPC(int pc){
  registerPC_CPU= pc;
}

// Setter for the RegisterA
//void Processor_SetRegisterA(int rA) {
//   registerA_CPU=rA;
 //}

// Getter for the Accumulator
int Processor_GetAccumulator() {
   return registerAccumulator_CPU;
 }

// int Processor_GetPC() {
//   return registerPC_CPU;
// }

int Processor_GetRegisterA() {
  return registerA_CPU;
}

// Setter for the PSW
void Processor_SetPSW(unsigned int psw){
	registerPSW_CPU=psw;
}

// Getter for the PSW
unsigned int Processor_GetPSW(){
	return registerPSW_CPU;
}

char * Processor_ShowPSW(){
	strcpy(pswmask,"----------");
	int tam=strlen(pswmask)-1;
	if (Processor_PSW_BitState(EXECUTION_MODE_BIT))
		pswmask[tam-EXECUTION_MODE_BIT]='X';
	if (Processor_PSW_BitState(OVERFLOW_BIT))
		pswmask[tam-OVERFLOW_BIT]='F';
	if (Processor_PSW_BitState(NEGATIVE_BIT))
		pswmask[tam-NEGATIVE_BIT]='N';
	if (Processor_PSW_BitState(ZERO_BIT))
		pswmask[tam-ZERO_BIT]='Z';
	if (Processor_PSW_BitState(POWEROFF_BIT))
		pswmask[tam-POWEROFF_BIT]='S';
	if (Processor_PSW_BitState(INTERRUPT_MASKED_BIT))
		pswmask[tam-INTERRUPT_MASKED_BIT]='M';
	return pswmask;
}

/////////////////////////////////////////////////////////
//  New functions below this line  //////////////////////


void Processor_Show(char section) {
	ComputerSystem_DebugMessage(Processor_PSW_BitState(EXECUTION_MODE_BIT)?5:4,section,Clock_GetTime());
}

//Se invoca cada vez que se produce una excepción
void Processor_RaiseException(int typeOfException) {
Processor_RaiseInterrupt(EXCEPTION_BIT);
registerB_CPU=typeOfException;
}

int Processor_GetExceptionType(){
	return registerB_CPU;
}
