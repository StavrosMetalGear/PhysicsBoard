#include "WebSocketServer.h"
#include "Board.h"

#include <App.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <functional>

using json = nlohmann::json;

WebSocketServer::WebSocketServer()
    : m_running(false), m_port(0) {
}

WebSocketServer::~WebSocketServer() {
    stop();
}

void WebSocketServer::start(int port,
    std::function<const Board& ()> getBoard,
    std::function<void(const Stroke&)> onStroke,
    std::function<bool()> getDrawPermission)
{
    if (m_running) {
        return;
    }

    m_port = port;
    m_getBoard = std::move(getBoard);
    m_onStroke = std::move(onStroke);
    m_getDrawPermission = std::move(getDrawPermission);
    m_running = true;

    m_thread = std::thread([this]() {
        uWS::App()
            .ws<PerSocketData>("/ws", {
                .open = [this](ClientSocket* ws) {
                    {
                        std::lock_guard<std::mutex> lock(m_clientsMutex);
                        m_clients.push_back(ws);
                    }

                    if (m_getBoard) {
                        broadcastBoard(m_getBoard());
                    }
                    if (m_getDrawPermission) {
                        broadcastPermission(m_getDrawPermission());
                    }
                },

                .message = [this](ClientSocket* ws, std::string_view msg, uWS::OpCode opCode) {
                    (void)ws;
                    (void)opCode;

                    try {
                        json j = json::parse(msg);

                        if (j.contains("type") &&
                            j["type"] == "stroke" &&
                            j.contains("stroke"))
                        {
                            Stroke s = j["stroke"].get<Stroke>();

                            if (m_onStroke) {
                                m_onStroke(s);
                            }

                            broadcastStroke(s);
                        }
                    }
                    catch (const std::exception& e) {
                        std::cerr << "WebSocketServer: Failed to parse message: "
                                  << e.what() << std::endl;
                    }
                },

                .close = [this](ClientSocket* ws, int code, std::string_view message) {
                    (void)code;
                    (void)message;

                    std::lock_guard<std::mutex> lock(m_clientsMutex);
                    m_clients.erase(
                        std::remove(m_clients.begin(), m_clients.end(), ws),
                        m_clients.end()
                    );
                }
                })

            .get("/", [](auto* res, auto* req) {
            (void)req;

            std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>PhysicsBoard Student</title>
</head>
<body>
<canvas id="board" width="1200" height="800" style="border:1px solid #888"></canvas>
<script>
let ws = new WebSocket('ws://' + location.host + '/ws');
ws.onopen = () => { console.log('Connected'); };
ws.onmessage = (ev) => { /* handle board updates */ };
function sendStroke(stroke) {
    ws.send(JSON.stringify({ type: 'stroke', stroke: stroke }));
}
</script>
</body>
</html>
)";
            res->writeHeader("Content-Type", "text/html");
            res->end(html);
                })

            .listen(m_port, [this](auto* token) {
            if (!token) {
                std::cout << "[WebSocketServer] Failed to listen on port "
                    << m_port << std::endl;
                m_running = false;
            }
            else {
                std::cout << "[WebSocketServer] Listening on port "
                    << m_port << std::endl;
            }
                })

            .run();
        });
}

void WebSocketServer::stop() {
    m_running = false;

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void WebSocketServer::broadcastStroke(const Stroke& stroke) {
    json j;
    j["type"] = "stroke";
    j["stroke"] = stroke;

    std::string msg = j.dump();

    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (ClientSocket* ws : m_clients) {
        if (ws) {
            ws->send(msg, uWS::OpCode::TEXT);
        }
    }
}

void WebSocketServer::broadcastBoard(const Board& board) {
    json j;
    j["type"] = "board";
    j["strokes"] = board.getStrokes();

    std::string msg = j.dump();

    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (ClientSocket* ws : m_clients) {
        if (ws) {
            ws->send(msg, uWS::OpCode::TEXT);
        }
    }
}

void WebSocketServer::broadcastPermission(bool allowed) {
    json j;
    j["type"] = "permission";
    j["allowed"] = allowed;

    std::string msg = j.dump();

    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (ClientSocket* ws : m_clients) {
        if (ws) {
            ws->send(msg, uWS::OpCode::TEXT);
        }
    }
}