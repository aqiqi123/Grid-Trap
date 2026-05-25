#pragma once
#include <winsock2.h>
#include <vector>
#include <string>
#include "Room.h"

class ServerNetwork
{
public:
	bool Initialize(int port);
	void SetRoom(Room* room) { _room = room; }
	void BroadcastMessage(const std::string& json)
	{
		for (auto& conn : _clientConnections)
		{
			Send(conn.Socket, json);
		}
	}
	void Run();
	bool Send(SOCKET clientSocket,const std::string& json);
	void Shutdown();

private:
	struct ClientConnection
	{
		SOCKET Socket = INVALID_SOCKET;
		std::vector<char> RecvBuffer;
		int PlayerId = -1;
	};

	Room* _room = nullptr;

	SOCKET _listenSocket = INVALID_SOCKET;
	std::vector<ClientConnection> _clientConnections;
	bool _isRunning = false;

	bool SetNonBlocking(SOCKET socket);
};
