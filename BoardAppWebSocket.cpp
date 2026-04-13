#include "BoardApp.h"
#include "WebSocketServer.h"
#include <memory>

// Integration: Add a WebSocketServer to BoardApp
namespace {
    std::unique_ptr<WebSocketServer> g_wsServer;
}

void BoardApp::startWebSocketServer(int port) {
    if (!g_wsServer) {
        g_wsServer = std::make_unique<WebSocketServer>();
        g_wsServer->start(
            port,
            [this]() -> const Board& { return m_boards[m_activeBoard]; },
            [this](const Stroke& s) { m_boards[m_activeBoard].addStroke(s); },
            []() { return true; } // Allow all for now
        );
    }
}

void BoardApp::stopWebSocketServer() {
    if (g_wsServer) {
        g_wsServer->stop();
        g_wsServer.reset();
    }
}

bool BoardApp::isWebSocketServerRunning() const {
    return g_wsServer && g_wsServer->isRunning();
}
