using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;

public class NetworkManager : MonoBehaviour
{
    public static NetworkManager Instance { get; private set; }

    public event Action<string> OnMessageReceived;

    private TcpClient _tcpClient;
    private NetworkStream _stream;
    private Thread _receiveThread;
    volatile bool _isConnected;

    private ConcurrentQueue<string> _messageQueue;

    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            _messageQueue = new ConcurrentQueue<string>();
        }
        else
        {
            Destroy(gameObject);
        }
    }

    private void Update()
    {
        while(_messageQueue.TryDequeue(out string msg))
        {
            OnMessageReceived?.Invoke(msg);
        }
    }

    private void OnDestroy()
    {
        Disconnect();
    }

    private void ResetConnectionState()
    {
        _isConnected = false;
        _stream?.Close();
        _stream = null;
        _tcpClient?.Close();
        _tcpClient = null;
    }

    private void ReceiveLoop()
    {
        List<byte> recvBuffer = new List<byte>();
        byte[] tempBuffer = new byte[1024];

        while (_isConnected)
        {
            try
            {
                if (_stream == null || !_stream.CanRead)
                    break;

                int bytesReceived = _stream.Read(tempBuffer, 0, tempBuffer.Length);
                if (bytesReceived <= 0)
                {
                    Debug.Log("[Network] 服务器主动断开了连接。");
                    break;
                }

                byte[] actualBytes = new byte[bytesReceived];
                Array.Copy(tempBuffer, actualBytes, bytesReceived);
                recvBuffer.AddRange(actualBytes);

                while (recvBuffer.Count >= 4)
                {
                    byte[] lenBytes = recvBuffer.GetRange(0, 4).ToArray();
                    int packetLen = BitConverter.ToInt32(lenBytes, 0);

                    if (recvBuffer.Count >= 4 + packetLen)
                    {
                        byte[] jsonBytes = recvBuffer.GetRange(4, packetLen).ToArray();
                        string jsonStr = System.Text.Encoding.UTF8.GetString(jsonBytes);

                        _messageQueue.Enqueue(jsonStr);

                        recvBuffer.RemoveRange(0, 4 + packetLen);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            catch (ThreadAbortException)
            {
                break;
            }
            catch (Exception e)
            {
                Debug.LogError($"[Network] 接收线程发生异常: {e.Message}");
                break;
            }
        }
    }

    public void Connect(string ip,int port)
    {
        if (_isConnected)
            return;

        try
        {
            _tcpClient = new TcpClient();
            _tcpClient.Connect(ip, port);

            _stream = _tcpClient.GetStream();
            _isConnected = true;

            _receiveThread = new Thread(ReceiveLoop);
            _receiveThread.IsBackground = true;
            _receiveThread.Start();
        }
        catch (SocketException e)
        {
            Debug.LogError($"[Network] 连接服务器失败 (Socket异常): {e.Message}，错误码: {e.SocketErrorCode}");
            ResetConnectionState();
        }
        catch (Exception e)
        {
            Debug.LogError($"[Network] 连接引发了未知异常: {e.Message}");
            ResetConnectionState();
        }
    }

    public void Send(string json)
    {
        if (!_isConnected || _stream == null)
        {
            Debug.LogWarning("[Network] 未建立连接，无法发送消息。");
            return;
        }

        try
        {
            byte[] jsonBytes = System.Text.Encoding.UTF8.GetBytes(json);

            byte[] lenBytes = BitConverter.GetBytes(jsonBytes.Length);

            _stream.Write(lenBytes, 0, lenBytes.Length);
            _stream.Write(jsonBytes, 0, jsonBytes.Length);
            _stream.Flush(); 
        }
        catch (Exception e)
        {
            Debug.LogError($"[Network] 发送消息失败: {e.Message}");
        }
    }

    public void Disconnect()
    {
        if (!_isConnected)
            return;

        _isConnected = false;

        try
        {
            if (_stream != null)
            {
                _stream.Close();
                _stream = null;
            }

            if (_tcpClient != null)
            {
                _tcpClient.Close();
                _tcpClient = null;
            }

            _receiveThread?.Join(1000);
            _receiveThread = null;

            Debug.Log("[Network] 已成功断开与服务器的连接，资源清理完毕。");
        }
        catch (Exception e)
        {
            Debug.LogError($"[Network] 断开连接清理资源时发生异常: {e.Message}");
        }
    }
}
