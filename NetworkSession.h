#pragma once

#include "Board.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <cstdint>

// ── NetworkSession ──────────────────────────────────────────────────────────
// Simple TCP-based collaborative whiteboard session.
// Host mode:  listens on a port, accepts one client (student).
// Join mode:  connects to a host IP:port.
// Protocol:   length-prefixed messages for stroke sync.
// ─────────────────────────────────────────────────────────────────────────────

enum class NetRole { None, Host, Client };

// Message types
enum class NetMsgType : uint8_t {
    StrokeAdd       = 1,   // sender adds a stroke
    BoardSync       = 2,   // full board state sync
    BoardSwitch     = 3,   // switch to board index
    ClearBoard      = 4,   // clear current board
    Undo            = 5,
    Redo            = 6,
    Ping            = 10,
    Pong            = 11,
    DrawPermission  = 12,  // host grants/revokes student drawing
};

class NetworkSession {
public:
    NetworkSession();
    ~NetworkSession();

    // Start hosting on a port
    bool host(int port);

    // Connect to a host
    bool join(const std::string& ip, int port);

    // Disconnect
    void disconnect();

    // Send a stroke to the remote peer
    void sendStroke(const Stroke& stroke);

    // Send full board state
    void sendBoardSync(const Board& board);

    // Send a command (clear, undo, redo, board switch)
    void sendCommand(NetMsgType type, int boardIndex = 0);

    // Poll received strokes (call each frame, thread-safe)
    bool hasIncomingStroke();
    Stroke popIncomingStroke();

    // Poll received commands
    struct NetCommand {
        NetMsgType type;
        int        boardIndex;
        std::vector<uint8_t> boardData;  // for BoardSync
    };
    bool hasIncomingCommand();
    NetCommand popIncomingCommand();

    // Student drawing permission (host grants/revokes)
    void sendDrawPermission(bool allow);
    bool studentCanDraw() const;
    void setStudentCanDraw(bool allow);  // for host-side state tracking

    // Status
    bool isConnected() const;
    NetRole getRole() const;
    const std::string& getStatusMessage() const;

private:
    NetRole              m_role;
    std::atomic<bool>    m_connected;
    std::atomic<bool>    m_running;
    std::string          m_statusMsg;

    // Socket handles (platform-specific, stored as uint64_t)
    uint64_t             m_listenSocket;
    uint64_t             m_peerSocket;

    std::thread          m_listenThread;
    std::thread          m_recvThread;

    std::mutex           m_strokeMutex;
    std::vector<Stroke>  m_incomingStrokes;

    std::mutex           m_cmdMutex;
    std::vector<NetCommand> m_incomingCommands;

    std::atomic<bool>    m_allowStudentDraw;  // permission state

    void acceptLoop();
    void receiveLoop();
    void sendRaw(const void* data, size_t size);
    bool recvRaw(void* buf, size_t size);
    void sendMessage(NetMsgType type, const std::vector<uint8_t>& payload);
    void processMessage(NetMsgType type, const std::vector<uint8_t>& payload);
    void cleanup();
};
