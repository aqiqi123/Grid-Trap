using Newtonsoft.Json.Linq;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class TrapUI : MonoBehaviour
{
    [SerializeField] private GridUI _gridUI;
    [SerializeField] private TextMeshProUGUI tips;
    [SerializeField] private Button confirmButton;

    private List<int> _selectedIndices = new List<int>();
    private bool _isActive = false;

    private void Start()
    {
        _gridUI.OnCellClicked += GridUI_OnCellClicked;
        GameManager.Instance.OnTrapPhase += GameManager_OnTrapPhase;
        GameManager.Instance.OnActionPhase += GameManager_OnActionPhase;
        confirmButton.onClick.AddListener(OnConfirmClicked);
        tips.gameObject.SetActive(false);
        confirmButton.gameObject.SetActive(false);
    }

    private void OnDestroy()
    {
        _gridUI.OnCellClicked -= GridUI_OnCellClicked;
        GameManager.Instance.OnTrapPhase -= GameManager_OnTrapPhase;
        GameManager.Instance.OnActionPhase -= GameManager_OnActionPhase;
        confirmButton.onClick.RemoveListener(OnConfirmClicked);
    }

    private void OnConfirmClicked()
    {
        if (!_isActive || _selectedIndices.Count != 3)
            return;

        var msg = new JObject();
        msg["type"] = "place_trap";
        msg["indices"] = new JArray(_selectedIndices);

        NetworkManager.Instance.Send(msg.ToString());
        _isActive = false;
    }

    private void GameManager_OnActionPhase()
    {
        _isActive = false;
        tips.gameObject.SetActive(false);
        confirmButton.gameObject.SetActive(false);
    }

    private void GameManager_OnTrapPhase()
    {
        _isActive = true;
        _selectedIndices.Clear();
        tips.gameObject.SetActive(true);
        for (int i = 0; i < 35; i++)
            SetCellHighlight(i, Color.white);
    }

    private void GridUI_OnCellClicked(int index)
    {
        if (!_isActive)
            return;

        if (_selectedIndices.Contains(index))
        {
            _selectedIndices.Remove(index);
            SetCellHighlight(index, Color.white);
        }
        else if (_selectedIndices.Count < 3)
        {
            _selectedIndices.Add(index);
            SetCellHighlight(index, Color.red);
        }

        confirmButton.gameObject.SetActive(_selectedIndices.Count == 3);
    }

    private void SetCellHighlight(int index, Color color)
    {
        var cell = _gridUI.transform.GetChild(index);
        cell.GetComponent<Image>().color = color;
    }
}
