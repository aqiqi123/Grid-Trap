using Newtonsoft.Json.Linq;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class GameOverUI : MonoBehaviour
{
    [SerializeField] private GameObject _panel;
    [SerializeField] private TextMeshProUGUI _winnerText;
    [SerializeField] private Button _restartButton;

    void Start()
    {
        _panel.SetActive(false);
        GameManager.Instance.OnGameOver += OnGameOver;
        _restartButton.onClick.AddListener(OnRestartClicked);
    }

    private void OnDestroy()
    {
        _restartButton.onClick.RemoveListener(OnRestartClicked);
        GameManager.Instance.OnGameOver -= OnGameOver;
    }

    private void OnGameOver(int winnerId)
    {
        _panel.SetActive(true);
        _winnerText.text = (winnerId == GameManager.Instance.MyPlayerId)
            ? "You Win"
            : $"{GameManager.Instance.GetPlayerName(winnerId)} Wins";
    }

    public void OnRestartClicked()
    {
        var msg = new JObject();
        msg["type"] = "restart";
        NetworkManager.Instance.Send(msg.ToString());
        _panel.SetActive(false);
    }
}
