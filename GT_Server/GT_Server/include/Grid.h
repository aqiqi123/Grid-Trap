#pragma once
#include <array>
#include <set>

enum class CellState
{
	Empty,
	Trap,
	Destroyed
};

class Grid
{
public:
	void AddTrap(int index);
	CellState Reveal(int index);
	void ApplyTraps(int minTraps);
	void Clear();
	CellState GetCellState(int index) const;

private:
	static constexpr int COLS = 7, ROWS = 5, TOTAL = 35;
	std::array<CellState, 35> _cells;
	std::set<int> _trapIndices;
};
