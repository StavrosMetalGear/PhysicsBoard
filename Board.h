#pragma once

#include <vector>
#include <string>
#include <cstdint>

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
    DarkGrid,      // Dark background with subtle grid lines
    WhiteLines,    // White background with horizontal ruled lines (notebook)
    BluePrint,     // Blue background with white engineering grid
    DotGrid,       // Light cream background with dot grid pattern
    Plain          // Plain white background, no grid
};

struct Point {
    float x, y;
};

struct Stroke {
    ToolType             tool;
    std::vector<Point>   points;       // freehand: many points; shapes: 2 points (start, end)
    uint32_t             color;        // ImGui packed color (ABGR)
    float                thickness;
    std::string          text;         // only used for ToolType::Text
};

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
    std::string                 m_name;
    std::vector<Stroke>         m_strokes;
    std::vector<std::vector<Stroke>> m_undoStack;
    std::vector<std::vector<Stroke>> m_redoStack;
    uint32_t                    m_bgColor;
    BackgroundStyle             m_bgStyle;

    void saveUndoState();
};
