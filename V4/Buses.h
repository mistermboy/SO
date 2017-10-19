#ifndef BUSES_H
#define BUSES_H

enum BusConnection { MAINMEMORY, MMU, CPU, INPUTDEVICE, OUTPUTDEVICE };

#define Bus_SUCCESS 1
#define Bus_FAIL -1

// Functions prototypes
int Buses_write_AddressBus_From_To(int, int);
int Buses_write_DataBus_From_To(int, int);

#endif