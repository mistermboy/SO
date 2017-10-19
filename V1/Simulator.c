#include <stdio.h>
#include <stdlib.h>
#include "Simulator.h"
#include "ComputerSystem.h"


int main(int argc, char *argv[]) {
  
	// We now have a multiprogrammed computer system
	// No more than USERPROGRAMSMAXNUMBER in the command line
	if ((argc < 3) || (argc>USERPROGRAMSMAXNUMBER+2)) {
		printf("USE: Simulator <sections to be debugged> <program1> [<program2> <program3>  ....] \n");
		exit(-1);
	}

	// The simulation starts
	ComputerSystem_PowerOn(argc, argv);
	// The simulation ends
	ComputerSystem_PowerOff();
	return 0;
}
