#include "../include/EventSystem.h"

void EventSystem::Initialize()
{
	_events.push_back({ "show 3*3 area trap num",10 });
	_events.push_back({ "select 2*2 area",18 });
	_events.push_back({ "select whole row",7 });
	_events.push_back({ "select whole col",7 });
	_events.push_back({ "select 2 cell step by step",14 });
	_events.push_back({ "select 3 cell step by step",12 });
	_events.push_back({ "reverse",10 });
	_events.push_back({ "skip",10 });

	_totalWeight = 0;
	for (const auto& e : _events)
		_totalWeight += e.Weight;

	_rng.seed(std::random_device{}());
}

const RandomEvent& EventSystem::Roll()
{
	std::uniform_int_distribution<int> dist(1, _totalWeight);
	int roll = dist(_rng);

	int cumulative = 0;
	for (const auto& e : _events)
	{
		cumulative += e.Weight;
		if (roll <= cumulative)
			return e;
	}

	return _events.back();
}