#include <iostream>
#include "../include/ServerNetwork.h"

int main()
{
    ServerNetwork serverNetwork;
    if (!serverNetwork.Initialize(8888))
    {
        std::cout << "服务端初始化失败" << std::endl;
        return 1;
    }

    Room room;
    serverNetwork.SetRoom(&room);

    room.SetBroadcastCallback([&serverNetwork](const std::string& msg)
    {
        serverNetwork.BroadcastMessage(msg);
    });

    serverNetwork.Run();
}
