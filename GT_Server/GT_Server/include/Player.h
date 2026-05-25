#pragma once
#include <string>

enum class Position
{
	Up,
	Down,
	Left,
	Right,
};

inline const char* PositionToString(Position pos)
{
	switch (pos)
	{
	case Position::Up:    return "ÉÏ";
	case Position::Down:  return "ÏÂ";
	case Position::Left:  return "×ó";
	case Position::Right: return "ÓÒ";
	default:              return "Î´Öª";
	}
}

struct Player
{
	int Id;
	std::string Name;
	int Hp;
	Position Pos;
	bool IsAlive;

	Player(int id,const std::string& name,Position pos)
		:Id(id)
		,Name(name)
		,Hp(3)
		,Pos(pos)
		,IsAlive(true)
	{}
};