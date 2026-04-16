#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include "Board.h"
#include <App.h>

// Forward declaration for Board and Stroke
class Board;
struct Stroke;

// Required by uWebSockets as the 3rd template parameter
struct PerSocketData {
};

class WebSocketServer {
public:
    using ClientSocket = uWS::WebSocket<false, true, PerSocketData>;

    WebSocketServer();
    ~WebSocketServer();
    bool isRunning() const { return m_running.load(); }
    // Start server on given port, bind to board and permission state
    void start(int port,
        std::function<const Board& ()> getBoard,
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
    std::atomic<bool> m_running{ false };
    int m_port{ 0 };

    // Track connected clients
    std::vector<ClientSocket*> m_clients;
    std::mutex m_clientsMutex;

    // Store callbacks
    std::function<const Board& ()> m_getBoard;
    std::function<void(const Stroke&)> m_onStroke;
    std::function<bool()> m_getDrawPermission;
};
