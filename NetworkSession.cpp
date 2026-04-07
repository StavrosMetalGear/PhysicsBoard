#include "NetworkSession.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketType = SOCKET;
    static constexpr SocketType INVALID_SOCK = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using SocketType = int;
    static constexpr SocketType INVALID_SOCK = -1;
    #define closesocket close
#endif

#include <cstring>
#include <algorithm>

// ── WSA init helper (Windows only) ──────────────────────────────────────────
static bool initWinsock() {
#ifdef _WIN32
    static bool inited = false;
    if (!inited) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
        inited = true;
    }
#endif
    return true;
}

NetworkSession::NetworkSession()
    : m_role(NetRole::None)
    , m_connected(false)
    , m_running(false)
    , m_listenSocket(static_cast<uint64_t>(INVALID_SOCK))
    , m_peerSocket(static_cast<uint64_t>(INVALID_SOCK))
    , m_allowStudentDraw(false) {}

NetworkSession::~NetworkSession() {
    disconnect();
}

bool NetworkSession::host(int port) {
    if (m_connected || m_running) disconnect();
    if (!initWinsock()) {
        m_statusMsg = "Winsock init failed";
        return false;
    }

    SocketType sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCK) {
        m_statusMsg = "Failed to create socket";
        return false;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<unsigned short>(port));

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        closesocket(sock);
        m_statusMsg = "Bind failed on port " + std::to_string(port);
        return false;
    }

    if (listen(sock, 1) != 0) {
        closesocket(sock);
        m_statusMsg = "Listen failed";
        return false;
    }

    m_listenSocket = static_cast<uint64_t>(sock);
    m_role = NetRole::Host;
    m_running = true;
    m_statusMsg = "Waiting for student on port " + std::to_string(port) + "...";

    m_listenThread = std::thread(&NetworkSession::acceptLoop, this);
    return true;
}

bool NetworkSession::join(const std::string& ip, int port) {
    if (m_connected || m_running) disconnect();
    if (!initWinsock()) {
        m_statusMsg = "Winsock init failed";
        return false;
    }

    SocketType sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCK) {
        m_statusMsg = "Failed to create socket";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<unsigned short>(port));
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        closesocket(sock);
        m_statusMsg = "Connection failed to " + ip + ":" + std::to_string(port);
        return false;
    }

    m_peerSocket = static_cast<uint64_t>(sock);
    m_role = NetRole::Client;
    m_connected = true;
    m_running = true;
    m_statusMsg = "Connected to " + ip + ":" + std::to_string(port);

    m_recvThread = std::thread(&NetworkSession::receiveLoop, this);
    return true;
}

void NetworkSession::disconnect() {
    m_running = false;
    m_connected = false;

    auto closeSock = [](uint64_t& s) {
        SocketType sock = static_cast<SocketType>(s);
        if (sock != INVALID_SOCK) {
            closesocket(sock);
            s = static_cast<uint64_t>(INVALID_SOCK);
        }
    };
    closeSock(m_peerSocket);
    closeSock(m_listenSocket);

    if (m_listenThread.joinable()) m_listenThread.join();
    if (m_recvThread.joinable()) m_recvThread.join();

    m_role = NetRole::None;
    m_statusMsg = "Disconnected";

    std::lock_guard<std::mutex> lk1(m_strokeMutex);
    m_incomingStrokes.clear();
    std::lock_guard<std::mutex> lk2(m_cmdMutex);
    m_incomingCommands.clear();
}

void NetworkSession::acceptLoop() {
    SocketType listenSock = static_cast<SocketType>(m_listenSocket);
    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);

    // Set a timeout so we can check m_running periodically
#ifdef _WIN32
    DWORD timeout = 1000;
    setsockopt(listenSock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = 1; tv.tv_usec = 0;
    setsockopt(listenSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    while (m_running) {
        SocketType client = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddr),
#ifdef _WIN32
            &addrLen
#else
            reinterpret_cast<socklen_t*>(&addrLen)
#endif
        );
        if (client != INVALID_SOCK) {
            m_peerSocket = static_cast<uint64_t>(client);
            m_connected = true;
            m_statusMsg = "Student connected!";
            receiveLoop();
            return;
        }
    }
}

void NetworkSession::receiveLoop() {
    while (m_running && m_connected) {
        // Read message: [type:1][length:4][payload:length]
        uint8_t typeByte;
        if (!recvRaw(&typeByte, 1)) break;

        uint32_t length = 0;
        if (!recvRaw(&length, 4)) break;

        std::vector<uint8_t> payload(length);
        if (length > 0 && !recvRaw(payload.data(), length)) break;

        processMessage(static_cast<NetMsgType>(typeByte), payload);
    }
    m_connected = false;
    m_statusMsg = "Connection lost";
}

void NetworkSession::sendRaw(const void* data, size_t size) {
    SocketType sock = static_cast<SocketType>(m_peerSocket);
    if (sock == INVALID_SOCK) return;
    const char* ptr = static_cast<const char*>(data);
    size_t sent = 0;
    while (sent < size) {
        int n = send(sock, ptr + sent, static_cast<int>(size - sent), 0);
        if (n <= 0) { m_connected = false; return; }
        sent += n;
    }
}

bool NetworkSession::recvRaw(void* buf, size_t size) {
    SocketType sock = static_cast<SocketType>(m_peerSocket);
    if (sock == INVALID_SOCK) return false;
    char* ptr = static_cast<char*>(buf);
    size_t received = 0;
    while (received < size) {
        int n = recv(sock, ptr + received, static_cast<int>(size - received), 0);
        if (n <= 0) return false;
        received += n;
    }
    return true;
}

void NetworkSession::sendMessage(NetMsgType type, const std::vector<uint8_t>& payload) {
    uint8_t typeByte = static_cast<uint8_t>(type);
    uint32_t length = static_cast<uint32_t>(payload.size());
    sendRaw(&typeByte, 1);
    sendRaw(&length, 4);
    if (!payload.empty()) sendRaw(payload.data(), payload.size());
}

void NetworkSession::processMessage(NetMsgType type, const std::vector<uint8_t>& payload) {
    switch (type) {
    case NetMsgType::StrokeAdd: {
        Stroke s = Board::deserializeStroke(payload.data(), payload.size());
        std::lock_guard<std::mutex> lk(m_strokeMutex);
        m_incomingStrokes.push_back(s);
        break;
    }
    case NetMsgType::BoardSync:
    case NetMsgType::BoardSwitch:
    case NetMsgType::ClearBoard:
    case NetMsgType::Undo:
    case NetMsgType::Redo: {
        NetCommand cmd;
        cmd.type = type;
        cmd.boardIndex = 0;
        if (type == NetMsgType::BoardSwitch && payload.size() >= 4) {
            cmd.boardIndex = payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24);
        }
        if (type == NetMsgType::BoardSync) {
            cmd.boardData = payload;
        }
        std::lock_guard<std::mutex> lk(m_cmdMutex);
        m_incomingCommands.push_back(cmd);
        break;
    }
    case NetMsgType::Ping:
        sendMessage(NetMsgType::Pong, {});
        break;
    case NetMsgType::DrawPermission:
        if (!payload.empty()) {
            m_allowStudentDraw = (payload[0] != 0);
        }
        break;
    default:
        break;
    }
}

void NetworkSession::sendStroke(const Stroke& stroke) {
    auto data = Board::serializeStroke(stroke);
    sendMessage(NetMsgType::StrokeAdd, data);
}

void NetworkSession::sendBoardSync(const Board& board) {
    auto data = board.serialize();
    sendMessage(NetMsgType::BoardSync, data);
}

void NetworkSession::sendCommand(NetMsgType type, int boardIndex) {
    std::vector<uint8_t> payload;
    if (type == NetMsgType::BoardSwitch) {
        uint32_t idx = static_cast<uint32_t>(boardIndex);
        payload.push_back(static_cast<uint8_t>(idx & 0xFF));
        payload.push_back(static_cast<uint8_t>((idx >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>((idx >> 16) & 0xFF));
        payload.push_back(static_cast<uint8_t>((idx >> 24) & 0xFF));
    }
    sendMessage(type, payload);
}

bool NetworkSession::hasIncomingStroke() {
    std::lock_guard<std::mutex> lk(m_strokeMutex);
    return !m_incomingStrokes.empty();
}

Stroke NetworkSession::popIncomingStroke() {
    std::lock_guard<std::mutex> lk(m_strokeMutex);
    Stroke s = m_incomingStrokes.front();
    m_incomingStrokes.erase(m_incomingStrokes.begin());
    return s;
}

bool NetworkSession::hasIncomingCommand() {
    std::lock_guard<std::mutex> lk(m_cmdMutex);
    return !m_incomingCommands.empty();
}

NetworkSession::NetCommand NetworkSession::popIncomingCommand() {
    std::lock_guard<std::mutex> lk(m_cmdMutex);
    NetCommand cmd = m_incomingCommands.front();
    m_incomingCommands.erase(m_incomingCommands.begin());
    return cmd;
}

bool NetworkSession::isConnected() const { return m_connected; }
NetRole NetworkSession::getRole() const { return m_role; }
const std::string& NetworkSession::getStatusMessage() const { return m_statusMsg; }

void NetworkSession::sendDrawPermission(bool allow) {
    m_allowStudentDraw = allow;
    std::vector<uint8_t> payload = { static_cast<uint8_t>(allow ? 1 : 0) };
    sendMessage(NetMsgType::DrawPermission, payload);
}

bool NetworkSession::studentCanDraw() const { return m_allowStudentDraw; }

void NetworkSession::setStudentCanDraw(bool allow) { m_allowStudentDraw = allow; }
