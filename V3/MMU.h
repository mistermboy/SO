#ifndef MMU_H
#define MMU_H

#define MMU_SUCCESS 1
#define MMU_FAIL -1

// Functions prototypes
int MMU_readMemory();
int MMU_writeMemory();

int MMU_GetMAR();
void MMU_SetMAR(int);
void MMU_SetBase(int);
void MMU_SetLimit(int);

#endif