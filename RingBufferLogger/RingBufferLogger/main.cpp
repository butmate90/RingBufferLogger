#include <iostream>
#include "RingBufferLogger.h"
#include <thread>
#include <chrono>


using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
	RingBufferLogger* RBL = new RingBufferLogger((char*)".\\", (char*)"LOG", 128000, 256, MAX_NUM_OF_LOGFILES, LOG_FMT_XML);
	char* test = "This is a test!";
	while (1)
	{
		RBL->LOG(LOG_LEVEL_ALWAYS, TEST, 0/*TODO*/, "%s", test);
		std::this_thread::sleep_for(10ms);
	}
	delete RBL;

	system("PAUSE");
	return 0;
}