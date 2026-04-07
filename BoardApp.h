#pragma once

#include "Board.h"
#include "FunctionPlotter.h"
#include "ProjectileSimulator.h"
#include "PhysicsToolbox.h"
#include "NetworkSession.h"
#include "imgui.h"
#include <array>

struct GLFWwindow;

// ── BoardApp ────────────────────────────────────────────────────────────────
// Main application class for PhysicsBoard.
//   - 5 switchable boards (tabs)
//   - Drawing tools: Pencil, Line, Rectangle, Circle, Arrow, Eraser, Ruler, Text
//   - Color picker and thickness slider
//   - Function plotter window (math expressions → plots)
//   - Network collaboration (host/join)
//   - Overlay mode for screen annotation
// ─────────────────────────────────────────────────────────────────────────────

class BoardApp {
public:
    static constexpr int NUM_BOARDS = 5;

    BoardApp();

    // Set GLFW window handle (needed for overlay mode)
    void setWindow(GLFWwindow* window);

    // Main render function — call each frame inside ImGui context
    void render();

    // Overlay state (queried by main loop for clear color)
    bool isOverlayMode() const { return m_overlayMode; }

private:
    // Boards
    std::array<Board, NUM_BOARDS> m_boards;
    int m_activeBoard;

    // Current tool state
    ToolType    m_currentTool;
    float       m_toolColor[3];
    uint32_t    m_packedColor;
    float       m_thickness;
    float       m_eraserRadius;
    char        m_textInput[256];

    // Drawing state
    bool        m_drawing;
    Stroke      m_currentStroke;
    ImVec2      m_canvasOrigin;
    ImVec2      m_canvasSize;

    // Pan & zoom
    ImVec2      m_panOffset;
    float       m_zoom;

    // Sub-windows
    FunctionPlotter m_plotter;
    bool            m_showPlotter;

    ProjectileSimulator m_projectile;
    bool                m_showProjectile;

    PhysicsToolbox  m_toolbox;
    bool            m_showToolbox;

    NetworkSession  m_network;
    bool            m_showNetwork;
    char            m_netIP[64];
    int             m_netPort;

    // Overlay mode (screen annotation)
    GLFWwindow*     m_window;
    bool            m_overlayMode;
    int             m_savedWinX, m_savedWinY;
    int             m_savedWinW, m_savedWinH;

    // Rendering helpers
    void renderToolbar();
    void renderBoardTabs();
    void renderCanvas();
    void renderNetworkPanel();
    void renderStroke(const Stroke& stroke, ImDrawList* drawList);

    // Overlay helpers
    void toggleOverlayMode();

    // Coordinate transforms (canvas ↔ board)
    ImVec2 boardToScreen(const Point& p) const;
    Point  screenToBoard(const ImVec2& p) const;

    // Tool helpers
    uint32_t packColor() const;
    void     finishStroke();
    void     handleEraser(const ImVec2& mousePos);

    // Network helpers
    void pollNetwork();
};
