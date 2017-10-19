#ifndef OPERATINGSYSTEMBASE_H
#define OPERATINGSYSTEMBASE_H

#include "ComputerSystem.h"
#include "OperatingSystem.h"
#include <stdio.h>

// Prototypes of OS functions that students should not change
int OperatingSystem_ObtainAnEntryInTheProcessTable();
int OperatingSystem_ObtainProgramSize(FILE **, char *);
int OperatingSystem_ObtainPriority(FILE *);
int OperatingSystem_LoadProgram(FILE *, int, int);
void OperatingSystem_ReadyToShutdown();
void OperatingSystem_ShowTime(char);
void OperatingSystem_PrintStatus();
void OperatingSystem_PrintReadyToRunQueue();
int OperatingSystem_IsThereANewProgram();
int OperatingSystem_InitializePartitionTable();
void OperatingSystem_ShowPartitionTable(char *); 


extern int sleepingProcessesQueue[PROCESSTABLEMAXSIZE];
extern int numberOfSleepingProcesses; 

#ifdef MEMCONFIG
// Copy to OperatingSystem.h Exercise 8 of V4
typedef struct {
     int occupied;
     int initAddress; // Lowest physical address of the partition
     int size; // Size of the partition in memory positions
     int PID; // PID of the process using the partition, if occupied
} PARTITIONDATA;

#define PARTITIONTABLEMAXSIZE PROCESSTABLEMAXSIZE*2
extern PARTITIONDATA partitionsTable[PARTITIONTABLEMAXSIZE];
#endif

#endif
