#include "../include/ServerNetwork.h"
#include <iostream>
#include <cstring>

#include "../third_party/json.hpp"

using json = nlohmann::json;

bool ServerNetwork::Initialize(int port)
{
	_isRunning = false;

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cout << "WSAStartup failed, error: " << result << "\n";
		return false;
	}

	_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listenSocket == INVALID_SOCKET)
	{
		std::cout << "Failed to create socket, error: " << WSAGetLastError() << "\n";
		WSACleanup();
		return false;
	}
	if (!SetNonBlocking(_listenSocket))
	{
		closesocket(_listenSocket);
		WSACleanup();
		return false;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int bindResult = bind(_listenSocket, (sockaddr*)&addr, sizeof(addr));
	if (bindResult == SOCKET_ERROR)
	{
		std::cout << "Failed to bind port, error: " << WSAGetLastError() << "\n";
		closesocket(_listenSocket);
		WSACleanup();
		return false;
	}

	int listenResult = listen(_listenSocket, SOMAXCONN);
	if (listenResult == SOCKET_ERROR)
	{
		std::cout << "Failed to listen, error: " << WSAGetLastError() << "\n";
		closesocket(_listenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

void ServerNetwork::Run()
{
	_isRunning = true;

	std::cout << "Server started, listening on port... Max players: " << Room::MAX_PLAYERS << '\n';

	while (_isRunning)
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(_listenSocket, &readfds);

		for (int i = 0; i < _clientConnections.size(); i++)
		{
			FD_SET(_clientConnections[i].Socket, &readfds);
		}

		timeval tv = { 0,100000 };
		int activity = select(0, &readfds, nullptr, nullptr, &tv);

		if (activity == SOCKET_ERROR)
		{
			std::cout << "select error: " << WSAGetLastError() << '\n';
			break;
		}

		if (activity == 0)
		{
			continue;
		}

		if (activity > 0)
		{
			if (FD_ISSET(_listenSocket, &readfds))
			{
				sockaddr_in clientAddr;
				int addrLen = sizeof(clientAddr);
				SOCKET clientSocket = accept(_listenSocket, (sockaddr*)&clientAddr, &addrLen);

				if (clientSocket != INVALID_SOCKET)
				{
					if (_clientConnections.size() >= Room::MAX_PLAYERS)
					{
						std::cout << "Room full (" << Room::MAX_PLAYERS << "/" << Room::MAX_PLAYERS << "), rejecting connection.\n";
						closesocket(clientSocket);
					}
					else
					{
						if (!SetNonBlocking(clientSocket))
						{
							closesocket(clientSocket);
							continue;
						}
						_clientConnections.push_back({ clientSocket,std::vector<char>() });
						std::cout << "New connection! Players: " << _clientConnections.size() << "/" << Room::MAX_PLAYERS << '\n';
					}
				}
			}

			auto it = _clientConnections.begin();
			while (it != _clientConnections.end())
			{
				SOCKET client = it->Socket;
				bool playerRemoved = false;

				if (FD_ISSET(client, &readfds))
				{
					char tempBuffer[1024];
					int bytesReceived = recv(client, tempBuffer, sizeof(tempBuffer), 0);

					if (bytesReceived > 0)
					{
						it->RecvBuffer.insert(it->RecvBuffer.end(), tempBuffer, tempBuffer + bytesReceived);

						while (it->RecvBuffer.size() >= 4)
						{
							int packetLen = 0;
							std::memcpy(&packetLen, it->RecvBuffer.data(), 4);

							if (it->RecvBuffer.size() >= 4 + packetLen)
							{
								std::string jsonStr(it->RecvBuffer.begin() + 4, it->RecvBuffer.begin() + 4 + packetLen);
								std::cout << "Received: " << jsonStr << '\n';

								json j = json::parse(jsonStr);
								std::string type = j["type"];

								if (type == "join" && _room != nullptr)
								{
									std::string name = j.value("name", "Unknown");
									int playerId = _room->AddPlayer(name);
									if (playerId >= 0)
									{
										it->PlayerId = playerId;

										json welcome;
										welcome["type"] = "welcome";
										welcome["playerId"] = playerId;
										Send(client, welcome.dump());

										if (_room->IsGameStarting())
										{
											BroadcastMessage(_room->BuildGameStartJson());
											BroadcastMessage("{\"type\":\"trap_phase\"}");
										}
									}
								}
								else if (type == "place_trap" && _room != nullptr)
								{
									int playerId = it->PlayerId;
									if (playerId >= 0)
									{
										std::vector<int> indices = j["indices"].get<std::vector<int>>();
										_room->SubmitTraps(playerId, indices);
									}
								}
								else if (type == "select_cell" && _room != nullptr)
								{
									int playerId = it->PlayerId;
									int index = j["index"].get<int>();
									_room->SelectCell(playerId, index);
								}
								else if (type == "trigger_event" && _room != nullptr)
								{
									int playerId = it->PlayerId;
									_room->TriggerEvent(playerId);
								}
								else if (type == "restart" && _room != nullptr)
								{
									_room->VoteRestart(it->PlayerId);
								}
								else if (type == "select_area" && _room != nullptr)
								{
									int playerId = it->PlayerId;
									int index = j["index"].get<int>();
									_room->ProcessArea(playerId, index);
								}

								json echo;
								echo["type"] = "echo";
								echo["original"] = j;
								Send(client, echo.dump());

								it->RecvBuffer.erase(it->RecvBuffer.begin(), it->RecvBuffer.begin() + 4 + packetLen);
							}
							else
							{
								break;
							}
						}
					}
					else if (bytesReceived == 0)
					{
						std::cout << "Client disconnected.\n";
						closesocket(client);
						it = _clientConnections.erase(it);
						playerRemoved = true;
					}
					else
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							std::cout << "Client connection error.\n";
							closesocket(client);
							it = _clientConnections.erase(it);
							playerRemoved = true;
						}
					}
				}

				if (!playerRemoved)
				{
					++it;
				}
			}
		}
	}
}

bool ServerNetwork::Send(SOCKET clientSocket, const std::string& json)
{
	if (clientSocket == INVALID_SOCKET) return false;

	int len = static_cast<int>(json.size());
	std::vector<char> sendBuffer(4 + len);

	std::memcpy(sendBuffer.data(), &len, 4);

	std::memcpy(sendBuffer.data() + 4, json.data(), len);

	int totalBytes = 4 + len;
	int totalSent = 0;
	while (totalSent < totalBytes)
	{
		int sent = send(clientSocket, sendBuffer.data() + totalSent, totalBytes - totalSent, 0);
		if (sent == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Sleep(10);
				continue;
			}
			return false;
		}
		totalSent += sent;
	}

	return true;
}

void ServerNetwork::Shutdown()
{
	_isRunning = false;

	for (int i = 0; i < _clientConnections.size(); i++)
	{
		closesocket(_clientConnections[i].Socket);
	}
	_clientConnections.clear();

	closesocket(_listenSocket);
	WSACleanup();
}

bool ServerNetwork::SetNonBlocking(SOCKET socket)
{
	unsigned long mode = 1;
	int result = ioctlsocket(socket, FIONBIO, &mode);

	if (result == SOCKET_ERROR)
	{
		std::cout << "Failed to set non-blocking mode, error: " << WSAGetLastError() << "\n";
		return false;
	}

	return true;
}
