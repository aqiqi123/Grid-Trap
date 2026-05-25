#include "../include/Room.h"
#include <iostream>
#include <algorithm>

#include "../third_party/json.hpp"

using json = nlohmann::json;

int Room::AddPlayer(const std::string& name)
{
	if (_players.size() >= MAX_PLAYERS)
	{
		return -1;
	}

	Player player(static_cast<int>(_players.size()), name, GetAvailablePosition());

	_players.push_back(player);

	if (_players.size() == MAX_PLAYERS)
	{
		_currentState = GameState::TrapPlacement;
	}

	return player.Id;
}

void Room::StartActionPhase()
{
	_eventSystem.Initialize();

	_alivePlayerCount = MAX_PLAYERS;
	_currentTurnPlayerId = 0;

	if (_broadcastCallback)
		_broadcastCallback("{\"type\":\"action_phase\"}");

	json msg;
	msg["type"] = "your_turn";
	msg["playerId"] = _currentTurnPlayerId;
	if (_broadcastCallback)
		_broadcastCallback(msg.dump());
}

CellState Room::SelectCell(int playerId, int index)
{
	if (playerId != _currentTurnPlayerId)
		return CellState::Destroyed;

	CellState result = _grid.Reveal(index);

	json msg;
	msg["type"] = "cell_revealed";
	msg["index"] = index;
	msg["result"] = (result == CellState::Trap) ? "trap" : "empty";
	msg["playerId"] = playerId;

	if (_broadcastCallback) _broadcastCallback(msg.dump());

		if (result == CellState::Trap)
		{
			if (HandleTrapHit(playerId)) return result;
			_multiSelectRemaining = 0;
		}

	if (_multiSelectRemaining > 0)
	{
		_multiSelectRemaining--;

		if (_multiSelectRemaining == 0)
		{
			_currentTurnPlayerId = GetNextAlivePlayerId();
			json turnMsg;
			turnMsg["type"] = "your_turn";
			turnMsg["playerId"] = _currentTurnPlayerId;
			if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
		}
		else
		{
			json multiMsg;
			multiMsg["type"] = "multi_select";
			multiMsg["playerId"] = _currentTurnPlayerId;
			multiMsg["count"] = _multiSelectRemaining;
			if (_broadcastCallback) _broadcastCallback(multiMsg.dump());
		}
	}
	else
	{
		_currentTurnPlayerId = GetNextAlivePlayerId();
		json turnMsg;
		turnMsg["type"] = "your_turn";
		turnMsg["playerId"] = _currentTurnPlayerId;
		if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
	}

	return result;
}

int Room::GetNextAlivePlayerId()
{
	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		int nextId = (_currentTurnPlayerId + (_isReversed ? -i : i) + MAX_PLAYERS) % MAX_PLAYERS;
		if (_players[nextId].IsAlive)
			return nextId;
	}
	return _currentTurnPlayerId;
}

bool Room::CheckWinCondition()
{
	if (_alivePlayerCount == 1)
	{
		_currentState = GameState::GameOver;

		json msg;
		msg["type"] = "game_over";
		msg["winnerId"] = GetNextAlivePlayerId();
		if (_broadcastCallback) _broadcastCallback(msg.dump());
		return true;
	}
	return false;
}

bool Room::HandleTrapHit(int playerId)
{
	_players[playerId].Hp--;

	json hpMsg;
	hpMsg["type"] = "hp_update";
	hpMsg["playerId"] = playerId;
	hpMsg["hp"] = _players[playerId].Hp;
	if (_broadcastCallback) _broadcastCallback(hpMsg.dump());

	if (_players[playerId].Hp <= 0)
	{
		_players[playerId].IsAlive = false;
		_alivePlayerCount--;

		json elimMsg;
		elimMsg["type"] = "player_eliminated";
		elimMsg["playerId"] = playerId;
		if (_broadcastCallback) _broadcastCallback(elimMsg.dump());

		return CheckWinCondition();
	}
	return false;
}

void Room::SubmitTraps(int playerId, const std::vector<int>& indices)
{
	if (_submittedPlayerIds.count(playerId))
		return;

	for (int idx : indices)
		_grid.AddTrap(idx);

	_submittedPlayerIds.insert(playerId);

	if (_submittedPlayerIds.size() == MAX_PLAYERS)
	{
		_grid.ApplyTraps(MAX_PLAYERS * 3);
		_currentState = GameState::ActionPhase;

		if (_broadcastCallback)
			StartActionPhase();
	}
}

void Room::TriggerEvent(int playerId)
{
	const RandomEvent& event = _eventSystem.Roll();

	std::cout << "[Event] Player " << playerId << " triggered: " << event.Name << '\n';

	json msg;
	msg["type"] = "event_result";
	msg["playerId"] = playerId;
	msg["eventName"] = event.Name;
	if (_broadcastCallback) _broadcastCallback(msg.dump());

	if (event.Name == "reverse")
	{
		_isReversed = !_isReversed;

		json reverseMsg;
		reverseMsg["type"] = "reverse_turn";
		reverseMsg["isReversed"] = _isReversed;
		if (_broadcastCallback) _broadcastCallback(reverseMsg.dump());
	}
	else if (event.Name == "select 2 cell step by step")
	{
		_multiSelectRemaining = 2;

		json multiMsg;
		multiMsg["type"] = "multi_select";
		multiMsg["playerId"] = playerId;
		multiMsg["count"] = 2;
		if (_broadcastCallback) _broadcastCallback(multiMsg.dump());
		return; 
	}
	else if (event.Name == "select 3 cell step by step")
	{
		_multiSelectRemaining = 3;

		json multiMsg;
		multiMsg["type"] = "multi_select";
		multiMsg["playerId"] = playerId;
		multiMsg["count"] = 3;
		if (_broadcastCallback) _broadcastCallback(multiMsg.dump());
		return; 
	}
	else if (event.Name == "select 2*2 area")
	{
		_areaMode = "2x2";
		json areaMsg;
		areaMsg["type"] = "area_select";
		areaMsg["area"] = "2x2";
		areaMsg["playerId"] = playerId;
		if (_broadcastCallback) _broadcastCallback(areaMsg.dump());
		return;
	}
	else if (event.Name == "show 3*3 area trap num")
	{
		_areaMode = "3x3_scout";
		json areaMsg;
		areaMsg["type"] = "area_select";
		areaMsg["area"] = "3x3_scout";
		areaMsg["playerId"] = playerId;
		if (_broadcastCallback) _broadcastCallback(areaMsg.dump());
		return;
	}
	else if (event.Name == "select whole row")
	{
		_areaMode = "row";
		json areaMsg;
		areaMsg["type"] = "area_select";
		areaMsg["area"] = "row";
		areaMsg["playerId"] = playerId;
		if (_broadcastCallback) _broadcastCallback(areaMsg.dump());
		return;
	}
	else if (event.Name == "select whole col")
	{
		_areaMode = "col";
		json areaMsg;
		areaMsg["type"] = "area_select";
		areaMsg["area"] = "col";
		areaMsg["playerId"] = playerId;
		if (_broadcastCallback) _broadcastCallback(areaMsg.dump());
		return;
	}
	else if (event.Name == "skip")
	{
		json skipMsg;
		skipMsg["type"] = "event_result";
		skipMsg["playerId"] = playerId;
		skipMsg["eventName"] = "skip";
		if (_broadcastCallback) _broadcastCallback(skipMsg.dump());
	}

	_currentTurnPlayerId = GetNextAlivePlayerId();
	json turnMsg;
	turnMsg["type"] = "your_turn";
	turnMsg["playerId"] = _currentTurnPlayerId;
	if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
}

std::string Room::BuildGameStartJson()
{
	json j;
	j["type"] = "game_start";
	j["players"] = json::array();
	for (const auto& p : _players)
	{
		json pj;
		pj["id"] = p.Id;
		pj["name"] = p.Name;
		pj["hp"] = p.Hp;
		pj["position"] = static_cast<int>(p.Pos);
		j["players"].push_back(pj);
	}
	return j.dump();
}

void Room::Reset()
{
	_grid.Clear();

	std::vector<std::string> names;
	for (const auto& p : _players)
		names.push_back(p.Name);

	_players.clear();
	_submittedPlayerIds.clear();
	_currentState = GameState::Waiting;
	_currentTurnPlayerId = 0;
	_alivePlayerCount = 0;

	for (const auto& name : names)
		AddPlayer(name);

	if (_broadcastCallback)
	{
		_broadcastCallback(BuildGameStartJson());
		_broadcastCallback("{\"type\":\"trap_phase\"}");
	}
}

void Room::ProcessArea(int playerId, int index)
{
	std::cout << "[ProcessArea] playerId=" << playerId << " currentTurn=" << _currentTurnPlayerId << " areaMode=" << _areaMode << " index=" << index << '\n';
	if (playerId != _currentTurnPlayerId) return;

	if (_areaMode == "2x2")
	{
		int row = index / 7;
		int col = index % 7;
		int anchorRow = (row == 4) ? 3 : row;
		int anchorCol = (col == 6) ? 5 : col;
		int anchor = anchorRow * 7 + anchorCol;

		int cells[4] = { anchor, anchor + 1, anchor + 7, anchor + 8 };
		for (int c : cells)
		{
			CellState result = _grid.Reveal(c);

			json cellMsg;
			cellMsg["type"] = "cell_revealed";
			cellMsg["index"] = c;
			cellMsg["result"] = (result == CellState::Trap) ? "trap" : "empty";
			cellMsg["playerId"] = playerId;

			if (_broadcastCallback) _broadcastCallback(cellMsg.dump());

			if (result == CellState::Trap && _players[playerId].IsAlive)
			{
				if (HandleTrapHit(playerId))
				{
					_areaMode.clear();
					return;
				}
			}
		}

		_areaMode.clear();
		_currentTurnPlayerId = GetNextAlivePlayerId();

		json turnMsg;
		turnMsg["type"] = "your_turn";
		turnMsg["playerId"] = _currentTurnPlayerId;
		if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
	}
	else if (_areaMode == "3x3_scout")
	{
		int row = index / 7;
		int col = index % 7;
		int rowStart = std::max(0, row - 1);
		int rowEnd = std::min(4, row + 1);
		int colStart = std::max(0, col - 1);
		int colEnd = std::min(6, col + 1);

		int trapCount = 0;
		json cellsArray = json::array();
		for (int r = rowStart; r <= rowEnd; r++)
			for (int c = colStart; c <= colEnd; c++)
			{
				int idx = r * 7 + c;
				cellsArray.push_back(idx);
				if (_grid.GetCellState(idx) == CellState::Trap)
					trapCount++;
			}

		json scoutMsg;
		scoutMsg["type"] = "scout_result";
		scoutMsg["trapCount"] = trapCount;
		scoutMsg["playerId"] = playerId;
		scoutMsg["cells"] = cellsArray;
		if (_broadcastCallback) _broadcastCallback(scoutMsg.dump());

		_areaMode.clear();
		_currentTurnPlayerId = GetNextAlivePlayerId();

		json turnMsg;
		turnMsg["type"] = "your_turn";
		turnMsg["playerId"] = _currentTurnPlayerId;
		if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
	}
	else if (_areaMode == "row")
	{
		int row = index / 7;

		for (int c = 0; c < 7; c++)
		{
			int idx = row * 7 + c;
			if (_grid.GetCellState(idx) == CellState::Destroyed)
				continue;

			CellState result = _grid.Reveal(idx);

			json cellMsg;
			cellMsg["type"] = "cell_revealed";
			cellMsg["index"] = idx;
			cellMsg["result"] = (result == CellState::Trap) ? "trap" : "empty";
			cellMsg["playerId"] = playerId;

			if (_broadcastCallback) _broadcastCallback(cellMsg.dump());

			if (result == CellState::Trap && _players[playerId].IsAlive)
			{
				if (HandleTrapHit(playerId))
				{
					_areaMode.clear();
					return;
				}
			}
		}

		_areaMode.clear();
		_currentTurnPlayerId = GetNextAlivePlayerId();

		json turnMsg;
		turnMsg["type"] = "your_turn";
		turnMsg["playerId"] = _currentTurnPlayerId;
		if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
	}
	else if (_areaMode == "col")
	{
		int col = index % 7;

		for (int r = 0; r < 5; r++)
		{
			int idx = r * 7 + col;
			if (_grid.GetCellState(idx) == CellState::Destroyed)
				continue;

			CellState result = _grid.Reveal(idx);

			json cellMsg;
			cellMsg["type"] = "cell_revealed";
			cellMsg["index"] = idx;
			cellMsg["result"] = (result == CellState::Trap) ? "trap" : "empty";
			cellMsg["playerId"] = playerId;

			if (_broadcastCallback) _broadcastCallback(cellMsg.dump());

			if (result == CellState::Trap && _players[playerId].IsAlive)
			{
				if (HandleTrapHit(playerId))
				{
					_areaMode.clear();
					return;
				}
			}
		}

		_areaMode.clear();
		_currentTurnPlayerId = GetNextAlivePlayerId();

		json turnMsg;
		turnMsg["type"] = "your_turn";
		turnMsg["playerId"] = _currentTurnPlayerId;
		if (_broadcastCallback) _broadcastCallback(turnMsg.dump());
	}
}
void Room::VoteRestart(int playerId)
{
	_restartVote.insert(playerId);
	
	if (_restartVote.size() == MAX_PLAYERS)
	{
		_restartVote.clear();
		Reset();
	}
}

Position Room::GetAvailablePosition()
{
	switch (_players.size())
	{
	case 0:
		return Position::Up;
	case 1:
		return Position::Down;
	case 2:
		return Position::Left;
	case 3:
		return Position::Right;
	default:
		return Position::Up;
	}
}