using Newtonsoft.Json.Linq;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class ActionUI : MonoBehaviour
{
    [SerializeField] private GridUI _gridUI;
    [SerializeField] private Button _eventButton;

    private bool _isMyTurn = false;
    private string _areaMode;

    void Start()
    {
        _gridUI.OnCellClicked += OnCellClicked;
        _eventButton.onClick.AddListener(OnEventClicked);
        _eventButton.gameObject.SetActive(false);
        GameManager.Instance.OnMyTurn += OnMyTurn;
        GameManager.Instance.OnMultiSelect += OnMultiSelect;
        GameManager.Instance.OnActionPhase += () => { };
        GameManager.Instance.OnAreaSelect += OnAreaSelect;
    }

    private void OnDestroy()
    {
        _gridUI.OnCellClicked -= OnCellClicked;
        _eventButton.onClick.RemoveListener(OnEventClicked);
        GameManager.Instance.OnMyTurn -= OnMyTurn;
        GameManager.Instance.OnMultiSelect -= OnMultiSelect;
        GameManager.Instance.OnActionPhase -= () => { };
        GameManager.Instance.OnAreaSelect -= OnAreaSelect;
    }

    private void OnMultiSelect(int count, int playerId)
    {
        _isMyTurn = (count > 0 && playerId == GameManager.Instance.MyPlayerId);
        _eventButton.gameObject.SetActive(false);
    }

    private void OnAreaSelect(string area, int playerId)
    {
        if (playerId == GameManager.Instance.MyPlayerId)
        {
            _areaMode = area;
            _isMyTurn = true;
            _eventButton.gameObject.SetActive(false);
        }
    }

    public void OnEventClicked()
    {
        if (!_isMyTurn)
            return;

        var msg = new JObject();
        msg["type"] = "trigger_event";
        NetworkManager.Instance.Send(msg.ToString());

        _isMyTurn = false;
        _eventButton.gameObject.SetActive(false);
    }

    private void OnMyTurn(int playerId)
    {
        _isMyTurn = (playerId == GameManager.Instance.MyPlayerId);
        _eventButton.gameObject.SetActive(_isMyTurn);
    }

    private void OnCellClicked(int index)
    {
        if (!_isMyTurn)
            return;

        if (!string.IsNullOrEmpty(_areaMode))
        {
            var areaMsg = new JObject();
            areaMsg["type"] = "select_area";
            areaMsg["area"] = _areaMode;
            areaMsg["index"] = index;
            Debug.Log($"[ActionUI] Sending select_area: area={_areaMode} index={index}");
            NetworkManager.Instance.Send(areaMsg.ToString());
            _areaMode = null;
            _isMyTurn = false;
            return;
        }

        var msg = new JObject();
        msg["type"] = "select_cell";
        msg["index"] = index;
        Debug.Log($"[ActionUI] Sending select_cell: index={index}");
        NetworkManager.Instance.Send(msg.ToString());

        _isMyTurn = false;
        _eventButton.gameObject.SetActive(false);
    }
}
