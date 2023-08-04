#pragma once
#include <Windows.h>
#include <iostream>

namespace Utils
{
	void Error(const char* szMessage, ...);
	bool CheckIfServer(bool bRunning);
}