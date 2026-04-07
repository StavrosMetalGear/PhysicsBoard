#include "Board.h"
#include <cstring>

Board::Board()
    : m_name("Board"), m_bgColor(0xFF2B2B2B), m_bgStyle(BackgroundStyle::DarkGrid) {}

void Board::setName(const std::string& name) { m_name = name; }
const std::string& Board::getName() const { return m_name; }

void Board::addStroke(const Stroke& stroke) {
    saveUndoState();
    m_strokes.push_back(stroke);
    m_redoStack.clear();
}

const std::vector<Stroke>& Board::getStrokes() const { return m_strokes; }

void Board::undo() {
    if (m_undoStack.empty()) return;
    m_redoStack.push_back(m_strokes);
    m_strokes = m_undoStack.back();
    m_undoStack.pop_back();
}

void Board::redo() {
    if (m_redoStack.empty()) return;
    m_undoStack.push_back(m_strokes);
    m_strokes = m_redoStack.back();
    m_redoStack.pop_back();
}

bool Board::canUndo() const { return !m_undoStack.empty(); }
bool Board::canRedo() const { return !m_redoStack.empty(); }

void Board::clear() {
    saveUndoState();
    m_strokes.clear();
    m_redoStack.clear();
}

void Board::setBackgroundColor(uint32_t color) { m_bgColor = color; }
uint32_t Board::getBackgroundColor() const { return m_bgColor; }

void Board::setBackgroundStyle(BackgroundStyle style) { m_bgStyle = style; }
BackgroundStyle Board::getBackgroundStyle() const { return m_bgStyle; }

void Board::saveUndoState() {
    m_undoStack.push_back(m_strokes);
    // Limit undo stack size to 50
    if (m_undoStack.size() > 50) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

// ── Serialization ───────────────────────────────────────────────────────────
// Binary format per stroke:
//   [toolType:4][color:4][thickness:4][numPoints:4][points...][textLen:4][text...]

std::vector<uint8_t> Board::serializeStroke(const Stroke& s) {
    std::vector<uint8_t> buf;
    auto pushU32 = [&](uint32_t v) {
        buf.push_back(static_cast<uint8_t>(v & 0xFF));
        buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        buf.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    };
    auto pushFloat = [&](float f) {
        uint32_t v;
        std::memcpy(&v, &f, 4);
        pushU32(v);
    };

    pushU32(static_cast<uint32_t>(s.tool));
    pushU32(s.color);
    pushFloat(s.thickness);
    pushU32(static_cast<uint32_t>(s.points.size()));
    for (auto& p : s.points) {
        pushFloat(p.x);
        pushFloat(p.y);
    }
    pushU32(static_cast<uint32_t>(s.text.size()));
    for (char c : s.text) buf.push_back(static_cast<uint8_t>(c));

    return buf;
}

Stroke Board::deserializeStroke(const uint8_t* data, size_t size) {
    Stroke s{};
    size_t off = 0;

    auto readU32 = [&]() -> uint32_t {
        if (off + 4 > size) return 0;
        uint32_t v = data[off] | (data[off + 1] << 8) | (data[off + 2] << 16) | (data[off + 3] << 24);
        off += 4;
        return v;
    };
    auto readFloat = [&]() -> float {
        uint32_t v = readU32();
        float f;
        std::memcpy(&f, &v, 4);
        return f;
    };

    s.tool = static_cast<ToolType>(readU32());
    s.color = readU32();
    s.thickness = readFloat();
    uint32_t numPts = readU32();
    s.points.resize(numPts);
    for (uint32_t i = 0; i < numPts; ++i) {
        s.points[i].x = readFloat();
        s.points[i].y = readFloat();
    }
    uint32_t textLen = readU32();
    s.text.resize(textLen);
    for (uint32_t i = 0; i < textLen; ++i) {
        s.text[i] = static_cast<char>(data[off++]);
    }
    return s;
}

std::vector<uint8_t> Board::serialize() const {
    std::vector<uint8_t> buf;
    // Header: number of strokes
    uint32_t n = static_cast<uint32_t>(m_strokes.size());
    buf.push_back(static_cast<uint8_t>(n & 0xFF));
    buf.push_back(static_cast<uint8_t>((n >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((n >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((n >> 24) & 0xFF));

    for (auto& s : m_strokes) {
        auto strokeData = serializeStroke(s);
        // Write stroke size first
        uint32_t sz = static_cast<uint32_t>(strokeData.size());
        buf.push_back(static_cast<uint8_t>(sz & 0xFF));
        buf.push_back(static_cast<uint8_t>((sz >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>((sz >> 16) & 0xFF));
        buf.push_back(static_cast<uint8_t>((sz >> 24) & 0xFF));
        buf.insert(buf.end(), strokeData.begin(), strokeData.end());
    }
    return buf;
}

void Board::deserialize(const std::vector<uint8_t>& data) {
    m_strokes.clear();
    if (data.size() < 4) return;
    size_t off = 0;
    uint32_t n = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    off = 4;
    for (uint32_t i = 0; i < n && off + 4 <= data.size(); ++i) {
        uint32_t sz = data[off] | (data[off + 1] << 8) | (data[off + 2] << 16) | (data[off + 3] << 24);
        off += 4;
        if (off + sz > data.size()) break;
        m_strokes.push_back(deserializeStroke(&data[off], sz));
        off += sz;
    }
}
