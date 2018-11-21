#pragma once
#include "FoundationAPI.h"
#include <thread>

void FOUNDATION_API SetThreadName(const char* threadName);
void FOUNDATION_API SetThreadName(std::thread* thread, const char* threadName);
