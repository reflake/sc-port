
#pragma once

#include "diagnostic/Clock.hpp"

class Cycle
{
public:

    Cycle(const char* name);
    ~Cycle();

    void Complete();
    void Throttle();
    double GetDeltaTime() const;

private:

    Clock _clock;
    uint64_t _counter;
    uint64_t _freq;
    bool _completed = false;
    double _deltaTime = 0.0f;
};