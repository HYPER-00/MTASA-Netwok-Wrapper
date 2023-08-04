#include "Utils.h"

void Utils::Error(const char* szMessage, ...)
{
	printf("\033[31m[ERROR]: %s\033[0m.\n", szMessage);
	exit(0);
}

bool Utils::CheckIfServer(bool bRunning)
{
	if (bRunning)
	{
		return true;
	}
	else
	{
		Error("Server Is Not Running Or Not Initialized");
		return false;
	}
}