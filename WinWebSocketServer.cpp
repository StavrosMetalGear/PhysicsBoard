#include "WinWebSocketServer.h"

#include <iostream>

void WinWebSocketServer::start(
    int port,
    std::function<const Board& ()> getBoard,
    std::function<void(const Stroke&)> onStroke,
    std::function<bool()> getDrawPermission)
{
    (void)port;
    (void)getBoard;
    (void)onStroke;
    (void)getDrawPermission;

    std::cerr << "[WinWebSocketServer] Not implemented yet on Windows." << std::endl;
    m_running = false;
}

void WinWebSocketServer::stop() {
    m_running = false;
}

void WinWebSocketServer::broadcastStroke(const Stroke& stroke) {
    (void)stroke;
}

void WinWebSocketServer::broadcastBoard(const Board& board) {
    (void)board;
}

void WinWebSocketServer::broadcastPermission(bool allowed) {
    (void)allowed;
}