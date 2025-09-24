#pragma once

#include <Cycle.hpp>

class Loop
{
public:

    Loop(const char* name);
    ~Loop();

    void Start();
    void Stop();

    bool IsRunning() const;

    double GetDeltaTime() const;

    Cycle GetNewCycle();

    void Complete(Cycle& cycle);

private:

    const char* _name;
    bool _running = false;
    double _deltaTime = 0.0;
};