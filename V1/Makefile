#
# Makefile for the Linux Simulator
#
########################################################

PROGRAM = 	Simulator

# Compilation Details
SHELL = /bin/sh
CC = cc
STDCFLAGS = -g -c -Wall
INCLUDES =
LIBRERIAS =

${PROGRAM}: Simulator.o ComputerSystem.o Messages.o MainMemory.o OperatingSystem.o OperatingSystemBase.o Processor.o MMU.o Buses.o Heap.o
	$(CC) -o ${PROGRAM} Simulator.o ComputerSystem.o Messages.o MainMemory.o OperatingSystem.o OperatingSystemBase.o Processor.o Buses.o MMU.o Heap.o $(LIBRERIAS)

Simulator.o: Simulator.c Simulator.h
	$(CC) $(STDCFLAGS) $(INCLUDES) Simulator.c

ComputerSystem.o: ComputerSystem.c ComputerSystem.h
	$(CC) $(STDCFLAGS) $(INCLUDES) ComputerSystem.c

Messages.o: Messages.c Messages.h
	$(CC) $(STDCFLAGS) $(INCLUDES) Messages.c

MainMemory.o: MainMemory.c MainMemory.h
	$(CC) $(STDCFLAGS) $(INCLUDES) MainMemory.c

OperatingSystem.o: OperatingSystem.c OperatingSystem.h
	$(CC) $(STDCFLAGS) $(INCLUDES) OperatingSystem.c

OperatingSystemBase.o: OperatingSystemBase.c OperatingSystemBase.h
	$(CC) $(STDCFLAGS) $(INCLUDES) OperatingSystemBase.c

Processor.o: Processor.c Processor.h
	$(CC) $(STDCFLAGS) $(INCLUDES) Processor.c

Buses.o: Buses.c Buses.h
	$(CC) $(STDCFLAGS) $(INCLUDES) Buses.c

MMU.o: MMU.c MMU.h
	$(CC) $(STDCFLAGS) $(INCLUDES) MMU.c

Heap.o: Heap.c Heap.h
	$(CC) $(STDCFLAGS) $(INCLUDES) Heap.c

clean:
	rm -f $(PROGRAM) *.o *~ core 
