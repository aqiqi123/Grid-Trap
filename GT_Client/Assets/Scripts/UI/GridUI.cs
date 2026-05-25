using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class GridUI : MonoBehaviour
{
    [SerializeField] private GameObject _cellPrefab;

    public event Action<int> OnCellClicked;

    private void Start()
    {
        GameManager.Instance.OnGameStarted += GameManager_OnGameStarted;
        GameManager.Instance.OnActionPhase += GameManager_OnActionPhase;
        GameManager.Instance.OnCellRevealed += GameManager_OnCellRevealed;
        GameManager.Instance.OnScoutResult += GameManager_OnScoutResult;
    }

    private void OnDestroy()
    {
        GameManager.Instance.OnGameStarted -= GameManager_OnGameStarted;
        GameManager.Instance.OnActionPhase -= GameManager_OnActionPhase;
        GameManager.Instance.OnCellRevealed -= GameManager_OnCellRevealed;
        GameManager.Instance.OnScoutResult -= GameManager_OnScoutResult;
    }

    private void GameManager_OnActionPhase()
    {
        for (int i = 0; i < 35; i++)
        {
            var img = transform.GetChild(i).GetComponent<Image>();
            img.color = Color.white;
        }
    }

    private void GameManager_OnCellRevealed(int index, string result, int playerId)
    {
        var cell = transform.GetChild(index);
        var img = cell.GetComponent<Image>();
        img.color = (result == "trap") ? Color.red : Color.gray;
        img.raycastTarget = false;
        StartCoroutine(FadeOut(img));
    }

    private void GameManager_OnGameStarted(List<PlayerData> obj)
    {
        for (int i = transform.childCount - 1; i >= 0; i--)
            Destroy(transform.GetChild(i).gameObject);

        for (int i = 0; i < 35; i++)
        {
            var cell = Instantiate(_cellPrefab, transform);

            int index = i;
            cell.GetComponent<Button>().onClick.AddListener(() => OnCellClicked?.Invoke(index));
        }
    }

    private void GameManager_OnScoutResult(List<int> cells, int trapCount, int playerId)
    {
        foreach (int index in cells)
        {
            if (index < 0 || index >= transform.childCount)
                continue;

            var img = transform.GetChild(index).GetComponent<Image>();
            if (img.raycastTarget)
                StartCoroutine(ScoutHighlight(img));
        }
    }

    private IEnumerator ScoutHighlight(Image img)
    {
        Color original = img.color;
        img.color = new Color(0.3f, 0.5f, 1f, 1f);

        yield return new WaitForSeconds(0.8f);

        float duration = 0.6f;
        float elapsed = 0f;
        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            img.color = Color.Lerp(new Color(0.3f, 0.5f, 1f, 1f), original, elapsed / duration);
            yield return null;
        }

        img.color = original;
    }

    private IEnumerator FadeOut(Image img)
    {
        yield return new WaitForSeconds(0.4f);

        float duration = 0.6f;
        float elapsed = 0f;
        Color c = img.color;
        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            c.a = Mathf.Lerp(1f, 0f, elapsed / duration);
            img.color = c;
            yield return null;
        }
    }
}
