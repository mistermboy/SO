#include "Clock.h"

before(): execution(void Processor_FetchInstruction()) {
	Clock_Update();
}

before(): execution(void OperatingSystem_InterruptLogic(int)){
	Clock_Update();
}

