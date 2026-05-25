# CLAUDE.md - GridTrap Arena 项目开发指南

## 1. 项目概述
- **游戏名称**: GridTrap Arena
- **游戏类型**: 4人在线、回合制策略/聚会游戏
- **核心玩法**: 5×7网格，前置暗拍陷阱阶段（互相隐藏，重叠算一个），后续回合制轮询选格消除或触发随机事件。血量归零淘汰，最后存活者获胜。
- **网络架构**: 权威服务器 (Server-Authoritative) 状态同步。

## 2. 技术栈与运行环境
### 服务端 (GT_Server)
- **开发环境**: Windows 控制台应用, Visual Studio 解决方案 (`.sln`)
- **语言标准**: C++20
- **网络层**: Windows 基础 Socket API (`Winsock2`)，采用**单线程非阻塞 `select` 轮询模型**。
- **序列化**: `nlohmann/json` (位于 `GT_Server/ThirdParty/json.hpp`)。

### 客户端 (GT_Client)
- **引擎版本**: Unity 2022.3 LTS
- **语言标准**: C#
- **网络层**: 原生 `System.Net.Sockets.TcpClient`（异步接收线程 + 主线程消息队列）。
- **序列化**: `Newtonsoft.Json` (通过 Unity Package Manager 导入)。

---

## 3. 目录结构规范
```text
GridTrap/
├── CLAUDE.md
├── GT_Server/                  # C++ 服务端项目
│   ├── GT_Server.sln           # VS 解决方案
│   ├── include/                # 头文件 (.h)
│   ├── src/                    # 源文件 (.cpp)
│   └── third_party/            # 第三方库
│       └── json.hpp            # 引入路径为 "../third_party/json.hpp"
└── GT_Client/                  # Unity 客户端项目
    └── Assets/
        └── Scripts/            # 所有游戏脚本
            ├── Network/        # 网络连接与消息分发
            ├── GameLogic/      # 核心玩法状态、状态机
            ├── UI/             # 界面渲染与表现
            └── Framework/      # 单例基类及工具函数
```

---

## 4. 代码风格与命名约定 (双端通用)

* **类名 / 函数名 / 公共变量 / 枚举**: 严格使用 `PascalCase`（例如 `PlayerManager`, `GetHp()`）。
* **私有变量 / 内部成员**: 带下怀线前缀的 `pascalCase` 或 `camelCase`（例如 `_currentTurn`, `_isReady`）。
* **注释**: 核心逻辑和算法步骤必须有清晰的中文注释。

---

## 5. 架构设计与核心原则

### C++ 服务端原则

* **面向对象思维 (OOP)**: 严禁将所有逻辑塞在 `main.cpp`。必须封装清晰的类：`ServerNetwork` (负责 select 循环与字节流封装), `Room` (管理4个玩家和游戏阶段), `Grid` (管理35个格子的状态与范围爆炸算法)。
* **网络数据成帧 (Framing)**: TCP 传输必须处理粘包/半包。所有 JSON 发送前必须附加 4 字节的整数作为长度前缀：`[Length(4 bytes)][JSON String]`。

### Unity 客户端原则

* **单例模式 (Singleton)**: 全局唯一的管理器（如 `NetworkManager`, `GameSceneManager`）使用单例架构。
* **严格解耦 (Decoupling)**: 逻辑层与表现层（UI）必须完全隔离。UI 严禁直接读取或修改网络层数据。网络层收到消息并更新数据后，通过 **C# Events / Actions** 派发事件，UI 监听事件并刷新画面。
* **主线程同步 (Main Thread Dispatcher)**: 异步网络线程接收到的 JSON 数据，必须压入线程安全队列，在主线程的 `Update` 中出队并安全调用 Unity API。

---

## 6. 当前开发里程碑

* [ ] **里程碑 1**: 双端 TCP 连通，实现自定义【长度前缀】的包体拆装，跑通首条验证 JSON。
* [ ] **里程碑 2**: 服务端实现完整的房间加入逻辑（满4人触发游戏开始），客户端动态渲染 5x7 网格 UI。
* [ ] **里程碑 3**: 服务端实现 Trap 阶段的去重暗拍算法与回合状态机。
* [ ] **里程碑 4**: 接入随机事件权重池，完善淘汰与胜利结算。

---

## 7. 核心指令与开发流程

* **游戏规则参考**: 详细的玩法、网格大小、随机事件权重及胜负判定，请严格参考同级目录下的 `GAME_DESIGN.md`。
* **阶段性任务流**:
1. 任何时候要求 AI 编写业务逻辑代码前，必须先核对 `GAME_DESIGN.md` 中的设计。
2. 优先完成网络基建（4字节长度前缀 + 粘包处理），再逐步根据游戏阶段（Phase）递进开发。