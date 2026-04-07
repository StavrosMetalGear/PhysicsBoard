#include "WebSocketServer.h"
#include "Board.h"
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <functional>

using json = nlohmann::json;

WebSocketServer::WebSocketServer() : m_running(false), m_port(0) {}
WebSocketServer::~WebSocketServer() { stop(); }

void WebSocketServer::start(int port,
    std::function<const Board&()> getBoard,
    std::function<void(const Stroke&)> onStroke,
    std::function<bool()> getDrawPermission)
{
    if (m_running) return;
    m_running = true;
    m_port = port;
    m_thread = std::thread([=]() {
        uWS::App()
        .ws<false>("/ws", {
            .open = [](auto* ws) {
                // On connect: send full board and permission
            },
            .message = [=](auto* ws, std::string_view msg, uWS::OpCode) {
                // On message: parse JSON, handle stroke or permission
            },
            .close = [](auto*, int, std::string_view) {
            }
        })
        .get("/", [](auto* res, auto*) {
            // Serve a minimal HTML/JS client
            std::string html = R"(
<!DOCTYPE html>
<html><head><meta charset='utf-8'><title>PhysicsBoard Student</title></head>
<body>
<canvas id='board' width='1200' height='800' style='border:1px solid #888'></canvas>
<script>
let ws = new WebSocket('ws://' + location.host + '/ws');
ws.onopen = () => { console.log('Connected'); };
ws.onmessage = (ev) => { /* handle board updates */ };
function sendStroke(stroke) { ws.send(JSON.stringify({type:'stroke', stroke})); }
// ... drawing logic ...
</script>
</body></html>
)";
            res->writeHeader("Content-Type", "text/html")->end(html);
        })
        .listen(port, [=](auto* token) {
            if (!token) {
                printf("[WebSocketServer] Failed to listen on port %d\n", port);
            } else {
                printf("[WebSocketServer] Listening on port %d\n", port);
            }
        })
        .run();
    });
}

void WebSocketServer::stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void WebSocketServer::broadcastStroke(const Stroke&) {}
void WebSocketServer::broadcastBoard(const Board&) {}
void WebSocketServer::broadcastPermission(bool) {}
