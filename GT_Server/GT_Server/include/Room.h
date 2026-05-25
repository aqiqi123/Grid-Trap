#pragma once
#include "Player.h"
#include "Grid.h"
#include <vector>
#include <functional>
#include "EventSystem.h"

enum class GameState
{
	Waiting,
	TrapPlacement,
	ActionPhase,
	GameOver
};

class Room
{
public:
	static constexpr int MAX_PLAYERS = 4;
	Room() : _currentState(GameState::Waiting),
		_currentTurnPlayerId(0),
		_alivePlayerCount(0),
		_isReversed(false){};
	bool IsGameStarting() const { return _currentState == GameState::TrapPlacement; }
	void SetBroadcastCallback(std::function<void(const std::string&)> callback)
	{
		_broadcastCallback = std::move(callback);
	}
	int AddPlayer(const std::string& name);
	void SubmitTraps(int playerId, const std::vector<int>& indices);
	void StartActionPhase();
	CellState SelectCell(int playerId, int index);
	int GetNextAlivePlayerId();
	bool CheckWinCondition();
	void TriggerEvent(int playerId);
	std::string BuildGameStartJson();
	void VoteRestart(int playerId);
	void Reset();
	void ProcessArea(int playerId, int index);

private:
	std::vector<Player> _players;
	GameState _currentState;

	Grid _grid;
	std::set<int> _submittedPlayerIds;

	std::function<void(const std::string&)> _broadcastCallback;

	int _currentTurnPlayerId;
	int _alivePlayerCount;
	bool _isReversed;

	EventSystem _eventSystem;

	std::set<int> _restartVote;

	int _multiSelectRemaining = 0;

	std::string _areaMode;

	Position GetAvailablePosition();
	bool HandleTrapHit(int playerId);
};