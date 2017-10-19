#include "MainMemory.h"
#include "Processor.h"
#include "Buses.h"
#include <string.h>

// Main memory can be simulated by a memory cell array
MEMORYCELL mainMemory[MAINMEMORYSIZE];

// Main memory has a MAR register whose value identifies where
// the next read/write operation will take place 
int registerMAR_MainMemory;

// It also has a register that plays the rol of a buffer for the mentioned operations
MEMORYCELL registerMBR_MainMemory;

// To read the contents of a memory cell, the MAR register must point (index) it
// The result of the operation is sent to the processor MBR register by using the
// data bus
void MainMemory_readMemory() {

	memcpy((void *) (&registerMBR_MainMemory), (void *) (&mainMemory[registerMAR_MainMemory]), sizeof(MEMORYCELL));
	Buses_write_DataBus_From_To(MAINMEMORY, CPU);
}

// To write in a memory cell, the MAR and MBR registers are used, set by the processor,
// as described previously 
void MainMemory_writeMemory() {

	memcpy((void *) (&mainMemory[registerMAR_MainMemory]), (void *) (&registerMBR_MainMemory), sizeof(MEMORYCELL));
}

// Getter for the registerMAR_MainMemory
int MainMemory_GetMAR() {
  return registerMAR_MainMemory;
}

// Setter for registerMAR_MainMemory
void MainMemory_SetMAR(int addr) {
  registerMAR_MainMemory=addr;
}

// pseudo-getter for the registerMBR_MainMemory
void MainMemory_GetMBR(MEMORYCELL *toRegister) {
  memcpy((void*) toRegister, (void *) (&registerMBR_MainMemory), sizeof(MEMORYCELL));
}

// pseudo-setter for the registerMBR_MainMemory
void MainMemory_SetMBR(MEMORYCELL *fromRegister) {
  memcpy((void*) (&registerMBR_MainMemory), (void *) fromRegister, sizeof(MEMORYCELL));
}

