#include "Cycle.hpp"

#include <SDL3/SDL_timer.h>
#include <stdexcept>
#include <unistd.h>

Cycle::Cycle(const char* name) : _clock(name), _counter(SDL_GetPerformanceCounter()), _freq(SDL_GetPerformanceFrequency())
{
}

Cycle::~Cycle()
{
	Complete();
}

void Cycle::Complete()
{
    if (_completed)
        return;

    _clock.Stop();
    _completed = true;

	uint64_t currentCounter = SDL_GetPerformanceCounter();

	_deltaTime = static_cast<double>(currentCounter - _counter) / _freq;
}

void Cycle::Throttle()
{
	if (_completed)
	{
		throw std::runtime_error("Cycle already completed");
	}

    usleep(1000); // throttling
}

double Cycle::GetDeltaTime() const
{
	if (!_completed)
	{
		throw std::runtime_error("Cycle not completed yet");
	}

	return _deltaTime;
}