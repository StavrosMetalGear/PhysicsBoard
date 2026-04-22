#pragma once

#include <functional>

#include "Board.h"

class IWebSocketServer {
public:
    virtual ~IWebSocketServer() = default;

    virtual void start(
        int port,
        std::function<const Board& ()> getBoard,
        std::function<void(const Stroke&)> onStroke,
        std::function<bool()> getDrawPermission) = 0;

    virtual void stop() = 0;
    virtual bool isRunning() const = 0;

    virtual void broadcastStroke(const Stroke& stroke) = 0;
    virtual void broadcastBoard(const Board& board) = 0;
    virtual void broadcastPermission(bool allowed) = 0;
};
