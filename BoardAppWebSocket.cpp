#include "BoardApp.h"
#include "IWebSocketServer.h"

#ifdef _WIN32
#include "WinWebSocketServer.h"
#else
#include "UwsWebSocketServer.h"
#endif

#include <memory>

namespace {
    std::unique_ptr<IWebSocketServer> g_wsServer;
}

void BoardApp::startWebSocketServer(int port) {
    if (!g_wsServer) {
#ifdef _WIN32
        g_wsServer = std::make_unique<WinWebSocketServer>();
#else
        g_wsServer = std::make_unique<UwsWebSocketServer>();
#endif

        g_wsServer->start(
            port,
            [this]() -> const Board& { return m_boards[m_activeBoard]; },
            [this](const Stroke& s) { m_boards[m_activeBoard].addStroke(s); },
            []() { return true; }
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