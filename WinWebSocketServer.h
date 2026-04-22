#pragma once

#include <atomic>
#include <functional>

#include "Board.h"
#include "IWebSocketServer.h"

class WinWebSocketServer : public IWebSocketServer {
public:
    WinWebSocketServer() = default;
    ~WinWebSocketServer() override = default;

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
    std::atomic<bool> m_running{ false };
};
