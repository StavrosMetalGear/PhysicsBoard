#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <functional>

// Forward declaration for Board and Stroke
class Board;
struct Stroke;

// WebSocketServer: runs in a background thread, serves HTTP/WS for student client
class WebSocketServer {
public:
    WebSocketServer();
    ~WebSocketServer();

    // Start server on given port, bind to board and permission state
    void start(int port,
               std::function<const Board&()> getBoard,
               std::function<void(const Stroke&)> onStroke,
               std::function<bool()> getDrawPermission);
    void stop();

    // Broadcast a new stroke to all clients
    void broadcastStroke(const Stroke& stroke);
    // Broadcast full board state
    void broadcastBoard(const Board& board);
    // Broadcast permission state
    void broadcastPermission(bool allowed);

private:
    std::thread m_thread;
    std::atomic<bool> m_running;
    int m_port;
    // Track connected clients
    std::vector<uWS::WebSocket<false, true>*> m_clients;
    std::mutex m_clientsMutex;
    // Store callbacks
    std::function<const Board&()> m_getBoard;
    std::function<void(const Stroke&)> m_onStroke;
    std::function<bool()> m_getDrawPermission;
};
