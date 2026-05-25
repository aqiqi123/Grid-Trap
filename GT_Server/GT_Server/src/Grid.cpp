#include "../include/Grid.h"
#include <algorithm>
#include <random> 

void Grid::AddTrap(int index)
{
	if (index >= 0 && index < TOTAL)
	{
		_trapIndices.insert(index);
	}
}

CellState Grid::Reveal(int index)
{
	CellState prev = _cells[index];
	_cells[index] = CellState::Destroyed;
	return prev;
}

void Grid::ApplyTraps(int minTraps)
{
	std::fill(_cells.begin(), _cells.end(), CellState::Empty);

	for (int idx : _trapIndices)
	{
		_cells[idx] = CellState::Trap;
	}
    int currentTraps = (int)_trapIndices.size();

    if (currentTraps < minTraps)
    {
        int need = minTraps - currentTraps;

        std::vector<int> emptyIndices;
        for (int i = 0; i < TOTAL; i++)
            if (_cells[i] == CellState::Empty)
                emptyIndices.push_back(i);

        static std::mt19937 rng(std::random_device{}());
        std::shuffle(emptyIndices.begin(), emptyIndices.end(), rng);

        for (int i = 0; i < need; i++)
        {
            _cells[emptyIndices[i]] = CellState::Trap;
            _trapIndices.insert(emptyIndices[i]); 
        }
    }
}

void Grid::Clear()
{
    std::fill(_cells.begin(), _cells.end(), CellState::Empty);
    _trapIndices.clear();
}

CellState Grid::GetCellState(int index) const
{
    return _cells[index];
}