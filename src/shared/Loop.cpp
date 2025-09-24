#include "Loop.hpp"

Loop::Loop(const char* name) : _name(name)
{
}

Loop::~Loop()
{
    Stop();
}

void Loop::Start()
{
    _running = true;
}

void Loop::Stop()
{
    _running = false;
}

bool Loop::IsRunning() const
{
    return _running;
}

Cycle Loop::GetNewCycle()
{
    return Cycle(_name);
}

void Loop::Complete(Cycle& cycle)
{
    cycle.Throttle();
    cycle.Complete();

    _deltaTime = cycle.GetDeltaTime();
}

double Loop::GetDeltaTime() const
{
    return _deltaTime;
}