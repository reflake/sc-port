#pragma once

#include <SDL_timer.h>
#include <cstdint>

void ShowClockReports();

class ClockReport
{
public:

	ClockReport() {}
	ClockReport(double time) : averageTime(time) 
		{};

	void AddTime(double time);

	double GetAverageTime();

private:

	double averageTime = 0.0;
};

class Clock
{
public:

	Clock(const char* name) : name(name), freq(SDL_GetPerformanceFrequency()), start(SDL_GetPerformanceCounter())
		{}

	~Clock()
	{
		Stop();
	}

	void Stop();

	void Report(double elapsedTime);

private:

	const char* name;
	bool released = false;
	uint64_t start, freq;
};