using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class LobbyUI : MonoBehaviour
{
    [SerializeField] private TMP_InputField _nameInput;
    [SerializeField] private TMP_InputField _ipInput;
    [SerializeField] private TMP_InputField _portInput;
    [SerializeField] private Button _connectButton;
    [SerializeField] private GameObject _lobbyPanel;

    void Start()
    {
        _lobbyPanel.SetActive(true);
        _ipInput.text = "127.0.0.1";
        _portInput.text = "8888";
        _connectButton.onClick.AddListener(OnConnectClicked);
    }

    private void OnDestroy()
    {
        _connectButton.onClick.RemoveListener(OnConnectClicked);
    }

    private void OnConnectClicked()
    {
        string playerName = _nameInput.text.Trim();
        string ip = _ipInput.text.Trim();
        string portStr = _portInput.text.Trim();

        if (string.IsNullOrEmpty(playerName))
        {
            Debug.LogWarning("[Lobby] Player name is empty");
            return;
        }

        if (string.IsNullOrEmpty(ip))
            ip = "127.0.0.1";

        if (!int.TryParse(portStr, out int port) || port <= 0)
            port = 8888;

        NetworkManager.Instance.Connect(ip, port);

        string joinJson = $"{{\"type\":\"join\",\"name\":\"{playerName}\"}}";
        NetworkManager.Instance.Send(joinJson);

        _lobbyPanel.SetActive(false);
    }
}
