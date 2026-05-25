using System.Collections;
using System.Collections.Generic;
using System.Text;
using TMPro;
using UnityEngine;

public class GameHUD : MonoBehaviour
{
    [SerializeField] private TMP_Text _phaseText;
    [SerializeField] private TMP_Text _turnText;
    [SerializeField] private TMP_Text _playerListText;
    [SerializeField] private TMP_Text _logText;

    private List<PlayerData> _players;

    void Start()
    {
        GameManager.Instance.OnGameStarted += OnGameStarted;
        GameManager.Instance.OnTrapPhase += () => _phaseText.text = "Trap Phase";
        GameManager.Instance.OnActionPhase += () => _phaseText.text = "Action Phase";
        GameManager.Instance.OnMyTurn += OnMyTurn;
        GameManager.Instance.OnHpUpdate += OnHpUpdate;
        GameManager.Instance.OnCellRevealed += OnCellRevealed;
        GameManager.Instance.OnTurnReversed += isRev =>
            AppendLog(isRev ? "Turn Order Reversed" : "Turn Order Restore");
        GameManager.Instance.OnScoutResult += OnScoutResult;
        GameManager.Instance.OnEventResult += OnEventResult;
    }

    private void OnDestroy()
    {
        GameManager.Instance.OnGameStarted -= OnGameStarted;
        GameManager.Instance.OnTrapPhase -= () => _phaseText.text = "Trap Phase";
        GameManager.Instance.OnActionPhase -= () => _phaseText.text = "Action Phase";
        GameManager.Instance.OnMyTurn -= OnMyTurn;
        GameManager.Instance.OnHpUpdate -= OnHpUpdate;
        GameManager.Instance.OnCellRevealed -= OnCellRevealed;
        GameManager.Instance.OnTurnReversed -= isRev =>
            AppendLog(isRev ? "Turn Order Reversed" : "Turn Order Restore");
        GameManager.Instance.OnScoutResult -= OnScoutResult;
        GameManager.Instance.OnEventResult -= OnEventResult;
    }

    private void OnGameStarted(List<PlayerData> players)
    {
        _players = players;
        RefreshPlayerList();
    }

    private string PN(int playerId) => GameManager.Instance.GetPlayerName(playerId);

    private void OnMyTurn(int playerId)
    {
        _turnText.text = (playerId == GameManager.Instance.MyPlayerId)
            ? "Your Turn"
            : $"Waiting for {PN(playerId)}...";
    }

    private void OnHpUpdate(int playerId, int hp)
    {
        _players.Find(p => p.id == playerId).hp = hp;
        RefreshPlayerList();
    }

    private void OnScoutResult(List<int> cells, int trapCount, int playerId)
    {
        AppendLog($"{PN(playerId)} scouted 3x3 area: {trapCount} trap(s)");
    }

    private void OnEventResult(int playerId, string eventName)
    {
        AppendLog($"{PN(playerId)}: {eventName}");
    }

    private void OnCellRevealed(int index, string result, int playerId)
    {
        string msg = result == "trap"
            ? $"{PN(playerId)}: Cell {index} Is Trap"
            : $"{PN(playerId)}: Cell {index} Is Safe";
        AppendLog(msg);
    }

    private void RefreshPlayerList()
    {
        var sb = new StringBuilder();
        foreach (var p in _players)
        {
            if (p.hp <= 0)
                sb.AppendLine($"<color=#808080>{p.name} : HP {p.hp}</color>");
            else if (p.id == GameManager.Instance.MyPlayerId)
                sb.AppendLine($"<color=#FFD700>{p.name} : HP {p.hp}</color>");
            else
                sb.AppendLine($"{p.name} : HP {p.hp}");
        }
        _playerListText.text = sb.ToString();
    }

    private void AppendLog(string msg)
    {
        _logText.text = msg;
    }
}
