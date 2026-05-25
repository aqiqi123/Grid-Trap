#pragma once

#include <string>
#include <vector>
#include <random>

struct RandomEvent
{
    std::string Name;
    int Weight;
};

class EventSystem
{
public:
    void Initialize();
    const RandomEvent& Roll();

private:
    std::vector<RandomEvent> _events;
    std::mt19937 _rng;
    int _totalWeight = 0;
};