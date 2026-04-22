#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "Board.h"
#include "IWebSocketServer.h"

#include <App.h>

struct PerSocketData {
};

class UwsWebSocketServer : public IWebSocketServer {
public:
    using ClientSocket = uWS::WebSocket<false, true, PerSocketData>;

    UwsWebSocketServer();
    ~UwsWebSocketServer() override;

    bool isRunning() const override { return m_running.load(); }

    void start(
        int port,
        std::function<const Board& ()> getBoard,
        std::function<void(const Stroke&)> onStroke,
        std::function<bool()> getDrawPermission) override;

    void stop() override;

    void broadcastStroke(const Stroke& stroke) override;
    void broadcastBoard(const Board& board) override;
    void broadcastPermission(bool allowed) override;

private:
    std::thread m_thread;
    std::atomic<bool> m_running{ false };
    int m_port{ 0 };

    std::vector<ClientSocket*> m_clients;
    std::mutex m_clientsMutex;

    std::function<const Board& ()> m_getBoard;
    std::function<void(const Stroke&)> m_onStroke;
    std::function<bool()> m_getDrawPermission;
};