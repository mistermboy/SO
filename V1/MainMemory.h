#ifndef MAINMEMORY_H
#define MAINMEMORY_H

// Main memory size (number of memory cells)
#define MAINMEMORYSIZE 300

// A memory cell is capable of storing a structure of the 
// MEMORYCELL TYPE
typedef struct {
    char operationCode;
    int operand1;
    int operand2;
} MEMORYCELL;

// Function prototypes
void MainMemory_readMemory();
void MainMemory_writeMemory();

int MainMemory_GetMAR();
void MainMemory_SetMAR(int);
void MainMemory_GetMBR(MEMORYCELL *);
void MainMemory_SetMBR(MEMORYCELL *);

#endif