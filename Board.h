#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include <nlohmann/json.hpp>

// ── Board ───────────────────────────────────────────────────────────────────
// Represents a single whiteboard with strokes (drawings).
// Each board has its own stroke list, undo/redo history, and background.
// ─────────────────────────────────────────────────────────────────────────────

enum class ToolType {
    Pencil,
    Line,
    Rectangle,
    Circle,
    Arrow,
    Eraser,
    Text,
    Ruler
};

enum class BackgroundStyle {
    DarkGrid,
    WhiteLines,
    BluePrint,
    DotGrid,
    Plain
};

struct Point {
    float x, y;
};

struct Stroke {
    ToolType           tool;
    std::vector<Point> points;      // freehand: many points; shapes: 2 points (start, end)
    uint32_t           color;       // ImGui packed color (ABGR)
    float              thickness;
    std::string        text;        // only used for ToolType::Text
};

// JSON conversion for Point
inline void to_json(nlohmann::json& j, const Point& p) {
    j = nlohmann::json{
        {"x", p.x},
        {"y", p.y}
    };
}

inline void from_json(const nlohmann::json& j, Point& p) {
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

// JSON conversion for Stroke
inline void to_json(nlohmann::json& j, const Stroke& s) {
    j = nlohmann::json{
        {"tool", static_cast<int>(s.tool)},
        {"points", s.points},
        {"color", s.color},
        {"thickness", s.thickness},
        {"text", s.text}
    };
}

inline void from_json(const nlohmann::json& j, Stroke& s) {
    int toolValue = 0;
    j.at("tool").get_to(toolValue);
    s.tool = static_cast<ToolType>(toolValue);

    j.at("points").get_to(s.points);
    j.at("color").get_to(s.color);
    j.at("thickness").get_to(s.thickness);
    j.at("text").get_to(s.text);
}

class Board {
public:
    Board();

    // Board name / label
    void setName(const std::string& name);
    const std::string& getName() const;

    // Stroke management
    void addStroke(const Stroke& stroke);
    const std::vector<Stroke>& getStrokes() const;

    // Undo / Redo
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

    // Clear all strokes (pushes to undo)
    void clear();

    // Background color
    void setBackgroundColor(uint32_t color);
    uint32_t getBackgroundColor() const;

    // Background style
    void setBackgroundStyle(BackgroundStyle style);
    BackgroundStyle getBackgroundStyle() const;

    // Serialization for network transfer
    std::vector<uint8_t> serialize() const;
    void deserialize(const std::vector<uint8_t>& data);

    // Serialize a single stroke (for incremental network sync)
    static std::vector<uint8_t> serializeStroke(const Stroke& stroke);
    static Stroke deserializeStroke(const uint8_t* data, size_t size);

private:
    std::string                     m_name;
    std::vector<Stroke>             m_strokes;
    std::vector<std::vector<Stroke>> m_undoStack;
    std::vector<std::vector<Stroke>> m_redoStack;
    uint32_t                        m_bgColor;
    BackgroundStyle                 m_bgStyle;

    void saveUndoState();
};