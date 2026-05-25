using System;
using System.Collections;
using System.Collections.Generic;
using Newtonsoft.Json.Linq;
using UnityEngine;

public class GameManager : MonoBehaviour
{
    public static GameManager Instance { get; private set; }

    public event Action<List<PlayerData>> OnGameStarted;
    public event Action OnTrapPhase;
    public event Action OnActionPhase;
    public event Action<int> OnMyTurn;            
    public event Action<int, string, int> OnCellRevealed;
    public event Action<int, int> OnHpUpdate;    
    public event Action<int> OnPlayerEliminated; 
    public event Action<int> OnGameOver;
    public event Action<bool> OnTurnReversed;
    public event Action<int, int> OnMultiSelect;
    public event Action<string, int> OnAreaSelect;
    public event Action<List<int>, int, int> OnScoutResult;
    public event Action<int, string> OnEventResult;

    public int MyPlayerId { get; private set; } = -1;

    private List<PlayerData> _players;

    public string GetPlayerName(int playerId)
    {
        if (_players != null)
        {
            var p = _players.Find(x => x.id == playerId);
            if (p != null)
                return p.name;
        }
        return $"Player {playerId}";
    }

    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
        }
        else
        {
            Destroy(gameObject);
        }
    }

    private void Start()
    {
        NetworkManager.Instance.OnMessageReceived += NetworkManager_OnMessageReceived;
    }

    private void OnDestroy()
    {
        NetworkManager.Instance.OnMessageReceived -= NetworkManager_OnMessageReceived;
    }

    private void NetworkManager_OnMessageReceived(string obj)
    {
        var jObj = JObject.Parse(obj);
        string type = (string)jObj["type"];

        if (type == "game_start")
        {
            var msg = jObj.ToObject<GameStartMessage>();
            _players = msg.players;
            OnGameStarted?.Invoke(_players);
        }
        else if (type == "trap_phase")
        {
            OnTrapPhase?.Invoke();
        }
        else if (type == "action_phase")
        {
            OnActionPhase?.Invoke();
        }
        else if (type == "your_turn")
        {
            int playerId = (int)jObj["playerId"];
            OnMyTurn?.Invoke(playerId);
        }
        else if (type == "cell_revealed")
        {
            int index = (int)jObj["index"];
            string result = (string)jObj["result"];
            int playerId = (int)jObj["playerId"];
            OnCellRevealed?.Invoke(index, result, playerId);
        }
        else if (type == "hp_update")
        {
            int playerId = (int)jObj["playerId"];
            int hp = (int)jObj["hp"];
            OnHpUpdate?.Invoke(playerId, hp);
        }
        else if(type== "player_eliminated")
        {
            int playerId = (int)jObj["playerId"];
            OnPlayerEliminated?.Invoke(playerId);
        }
        else if (type == "game_over")
        {
            int winnerId = (int)jObj["winnerId"];
            OnGameOver?.Invoke(winnerId);
        }
        else if (type == "welcome")
        {
            MyPlayerId = (int)jObj["playerId"];
        }
        else if (type == "event_result")
        {
            int playerId = (int)jObj["playerId"];
            string eventName = (string)jObj["eventName"];
            OnEventResult?.Invoke(playerId, eventName);
        }
        else if (type == "reverse_turn")
        {
            bool isReversed = (bool)jObj["isReversed"];
            OnTurnReversed?.Invoke(isReversed);
        }
        else if (type == "multi_select")
        {
            int count = (int)jObj["count"];
            int playerId = (int)jObj["playerId"];
            OnMultiSelect?.Invoke(count, playerId);
        }
        else if (type == "area_select")
        {
            string area = (string)jObj["area"];
            int playerId = (int)jObj["playerId"];
            OnAreaSelect?.Invoke(area, playerId);
        }
        else if (type == "scout_result")
        {
            int trapCount = (int)jObj["trapCount"];
            int playerId = (int)jObj["playerId"];
            var cells = jObj["cells"].ToObject<List<int>>();
            OnScoutResult?.Invoke(cells, trapCount, playerId);
        }
    }
}

[System.Serializable]
public class PlayerData
{
    public int id;
    public string name;
    public int hp;
    public int position;
}

[System.Serializable]
public class GameStartMessage
{
    public string type;
    public List<PlayerData> players;
}
