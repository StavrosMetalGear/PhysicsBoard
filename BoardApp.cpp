#include "BoardApp.h"
#include "imgui.h"
#include "implot.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BoardApp::BoardApp()
    : m_activeBoard(0)
    , m_currentTool(ToolType::Pencil)
    , m_thickness(2.0f)
    , m_eraserRadius(20.0f)
    , m_drawing(false)
    , m_canvasOrigin(0, 0)
    , m_canvasSize(800, 600)
    , m_panOffset(0, 0)
    , m_zoom(1.0f)
    , m_showPlotter(false)
    , m_showProjectile(false)
    , m_showToolbox(false)
    , m_showNetwork(false)
    , m_netPort(7777)
    , m_window(nullptr)
    , m_overlayMode(false)
    , m_savedWinX(0), m_savedWinY(0)
    , m_savedWinW(1600), m_savedWinH(900)
{
    m_toolColor[0] = 0.0f; m_toolColor[1] = 0.0f; m_toolColor[2] = 0.0f;
    m_packedColor = packColor();
    m_textInput[0] = '\0';
    std::memset(m_netIP, 0, sizeof(m_netIP));
    std::strcpy(m_netIP, "127.0.0.1");

    // Name the boards and assign distinct background styles
    m_boards[0].setName("Board 1 - Lined");
    m_boards[0].setBackgroundStyle(BackgroundStyle::WhiteLines);

    m_boards[1].setName("Board 2 - Dark");
    m_boards[1].setBackgroundStyle(BackgroundStyle::DarkGrid);

    m_boards[2].setName("Board 3 - Blueprint");
    m_boards[2].setBackgroundStyle(BackgroundStyle::BluePrint);

    m_boards[3].setName("Board 4 - Dots");
    m_boards[3].setBackgroundStyle(BackgroundStyle::DotGrid);

    m_boards[4].setName("Board 5 - Plain");
    m_boards[4].setBackgroundStyle(BackgroundStyle::Plain);
}

uint32_t BoardApp::packColor() const {
    uint8_t r = static_cast<uint8_t>(m_toolColor[0] * 255.0f);
    uint8_t g = static_cast<uint8_t>(m_toolColor[1] * 255.0f);
    uint8_t b = static_cast<uint8_t>(m_toolColor[2] * 255.0f);
    return IM_COL32(r, g, b, 255);
}

void BoardApp::setWindow(GLFWwindow* window) {
    m_window = window;
}

void BoardApp::toggleOverlayMode() {
    if (!m_window) return;
    m_overlayMode = !m_overlayMode;

    if (m_overlayMode) {
        // Save current window geometry
        glfwGetWindowPos(m_window, &m_savedWinX, &m_savedWinY);
        glfwGetWindowSize(m_window, &m_savedWinW, &m_savedWinH);

        // Make always-on-top and remove decorations
        glfwSetWindowAttrib(m_window, GLFW_FLOATING, GLFW_TRUE);
        glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);

        // Expand to cover the primary monitor work area
        int mx, my, mw, mh;
        glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &mx, &my, &mw, &mh);
        glfwSetWindowPos(m_window, mx, my);
        glfwSetWindowSize(m_window, mw, mh);
    }
    else {
        // Restore normal window
        glfwSetWindowAttrib(m_window, GLFW_FLOATING, GLFW_FALSE);
        glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);

        // Restore saved geometry
        glfwSetWindowPos(m_window, m_savedWinX, m_savedWinY);
        glfwSetWindowSize(m_window, m_savedWinW, m_savedWinH);
    }
}

ImVec2 BoardApp::boardToScreen(const Point& p) const {
    return ImVec2(m_canvasOrigin.x + (p.x + m_panOffset.x) * m_zoom,
                  m_canvasOrigin.y + (p.y + m_panOffset.y) * m_zoom);
}

Point BoardApp::screenToBoard(const ImVec2& p) const {
    return { (p.x - m_canvasOrigin.x) / m_zoom - m_panOffset.x,
             (p.y - m_canvasOrigin.y) / m_zoom - m_panOffset.y };
}

// ── Main render ─────────────────────────────────────────────────────────────
void BoardApp::render() {
    // Poll network events
    pollNetwork();

    // F11 shortcut for overlay toggle
    if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
        toggleOverlayMode();
    }

    // Full-window dockspace
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    // In overlay mode, make the main window background transparent
    if (m_overlayMode) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    }

    ImGui::Begin("PhysicsBoard##Main",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_MenuBar);

    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Function Plotter", nullptr, &m_showPlotter);
            ImGui::MenuItem("Projectile Motion", nullptr, &m_showProjectile);
            ImGui::MenuItem("Physics Toolbox", nullptr, &m_showToolbox);
            ImGui::MenuItem("Network / Collaborate", nullptr, &m_showNetwork);
            ImGui::Separator();
            if (ImGui::MenuItem(m_overlayMode ? "Exit Overlay Mode" : "Screen Overlay Mode", "F11")) {
                toggleOverlayMode();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Board Background")) {
                Board& bg = m_boards[m_activeBoard];
                BackgroundStyle cur = bg.getBackgroundStyle();
                if (ImGui::RadioButton("Dark Grid",    cur == BackgroundStyle::DarkGrid))
                    bg.setBackgroundStyle(BackgroundStyle::DarkGrid);
                if (ImGui::RadioButton("White Lines",  cur == BackgroundStyle::WhiteLines))
                    bg.setBackgroundStyle(BackgroundStyle::WhiteLines);
                if (ImGui::RadioButton("Blueprint",    cur == BackgroundStyle::BluePrint))
                    bg.setBackgroundStyle(BackgroundStyle::BluePrint);
                if (ImGui::RadioButton("Dot Grid",     cur == BackgroundStyle::DotGrid))
                    bg.setBackgroundStyle(BackgroundStyle::DotGrid);
                if (ImGui::RadioButton("Plain White",  cur == BackgroundStyle::Plain))
                    bg.setBackgroundStyle(BackgroundStyle::Plain);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            Board& b = m_boards[m_activeBoard];
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, b.canUndo())) {
                b.undo();
                if (m_network.isConnected()) m_network.sendCommand(NetMsgType::Undo);
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, b.canRedo())) {
                b.redo();
                if (m_network.isConnected()) m_network.sendCommand(NetMsgType::Redo);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Clear Board")) {
                b.clear();
                if (m_network.isConnected()) m_network.sendCommand(NetMsgType::ClearBoard);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    renderToolbar();
    renderBoardTabs();
    renderCanvas();

    ImGui::End();

    if (m_overlayMode) {
        ImGui::PopStyleColor(); // WindowBg
    }

    // Sub-windows
    if (m_showPlotter)     m_plotter.render(&m_showPlotter);
    if (m_showProjectile)  m_projectile.render(&m_showProjectile);
    if (m_showToolbox)     m_toolbox.render(&m_showToolbox);
    if (m_showNetwork)     renderNetworkPanel();
}

// ── Toolbar ─────────────────────────────────────────────────────────────────
void BoardApp::renderToolbar() {
    ImGui::BeginChild("##Toolbar", ImVec2(-1, 50), true);

    const char* toolNames[] = {
        "Pencil", "Line", "Rectangle", "Circle", "Arrow", "Eraser", "Text", "Ruler"
    };
    for (int i = 0; i < 8; ++i) {
        if (i > 0) ImGui::SameLine();
        bool selected = (static_cast<int>(m_currentTool) == i);
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
        if (ImGui::Button(toolNames[i], ImVec2(70, 30))) {
            m_currentTool = static_cast<ToolType>(i);
        }
        if (selected) ImGui::PopStyleColor();
    }

    ImGui::SameLine(0, 20);
    ImGui::PushItemWidth(100);
    ImGui::ColorEdit3("##Color", m_toolColor, ImGuiColorEditFlags_NoInputs);
    m_packedColor = packColor();
    ImGui::SameLine();
    ImGui::SliderFloat("Size##Thick", &m_thickness, 1.0f, 20.0f, "%.1f");
    ImGui::PopItemWidth();

    if (m_currentTool == ToolType::Eraser) {
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("Eraser##Rad", &m_eraserRadius, 5.0f, 80.0f, "%.0f");
        ImGui::PopItemWidth();
    }

    if (m_currentTool == ToolType::Text) {
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputText("Text##Input", m_textInput, sizeof(m_textInput));
        ImGui::PopItemWidth();
    }

    // Zoom controls
    ImGui::SameLine(0, 20);
    ImGui::PushItemWidth(100);
    ImGui::SliderFloat("Zoom##Z", &m_zoom, 0.25f, 4.0f, "%.2fx");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Reset View##RV")) {
        m_zoom = 1.0f;
        m_panOffset = ImVec2(0, 0);
    }

    ImGui::EndChild();
}

// ── Board Tabs ──────────────────────────────────────────────────────────────
void BoardApp::renderBoardTabs() {
    if (ImGui::BeginTabBar("##BoardTabs")) {
        for (int i = 0; i < NUM_BOARDS; ++i) {
            ImGuiTabItemFlags flags = 0;
            if (ImGui::BeginTabItem(m_boards[i].getName().c_str(), nullptr, flags)) {
                if (m_activeBoard != i) {
                    m_activeBoard = i;
                    if (m_network.isConnected())
                        m_network.sendCommand(NetMsgType::BoardSwitch, i);
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

// ── Canvas ──────────────────────────────────────────────────────────────────
void BoardApp::renderCanvas() {
    Board& board = m_boards[m_activeBoard];

    // Canvas fills remaining space
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
    if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;

    m_canvasOrigin = canvasPos;
    m_canvasSize = canvasSize;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Background — style-dependent (skip in overlay mode for transparency)
    BackgroundStyle bgStyle = board.getBackgroundStyle();
    if (!m_overlayMode) {
        uint32_t bgFill;
        switch (bgStyle) {
            case BackgroundStyle::WhiteLines: bgFill = IM_COL32(255, 255, 255, 255); break;
            case BackgroundStyle::BluePrint:  bgFill = IM_COL32(26, 58, 92, 255);    break;
            case BackgroundStyle::DotGrid:    bgFill = IM_COL32(250, 250, 240, 255);  break;
            case BackgroundStyle::Plain:      bgFill = IM_COL32(255, 255, 255, 255);  break;
            default:                          bgFill = IM_COL32(43, 43, 43, 255);     break;
        }
        drawList->AddRectFilled(canvasPos,
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), bgFill);
    }

    // Clip to canvas
    drawList->PushClipRect(canvasPos,
        ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

    // Grid / lines — style-dependent (skip in overlay mode)
    if (!m_overlayMode) {
    float gridStep = 50.0f * m_zoom;
    float offsetX = std::fmod(m_panOffset.x * m_zoom, gridStep);
    float offsetY = std::fmod(m_panOffset.y * m_zoom, gridStep);
    if (offsetX < 0) offsetX += gridStep;
    if (offsetY < 0) offsetY += gridStep;

    switch (bgStyle) {
    case BackgroundStyle::DarkGrid: {
        uint32_t gridColor = IM_COL32(255, 255, 255, 15);
        for (float x = offsetX; x < canvasSize.x; x += gridStep)
            drawList->AddLine(ImVec2(canvasPos.x + x, canvasPos.y),
                              ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), gridColor);
        for (float y = offsetY; y < canvasSize.y; y += gridStep)
            drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
                              ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), gridColor);
        break;
    }
    case BackgroundStyle::WhiteLines: {
        // Horizontal ruled lines (notebook style)
        float lineStep = 30.0f * m_zoom;
        float lineOffsetY = std::fmod(m_panOffset.y * m_zoom, lineStep);
        if (lineOffsetY < 0) lineOffsetY += lineStep;
        uint32_t lineColor = IM_COL32(100, 149, 237, 80);
        for (float y = lineOffsetY; y < canvasSize.y; y += lineStep)
            drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
                              ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), lineColor);
        // Red margin line
        drawList->AddLine(ImVec2(canvasPos.x + 80.0f, canvasPos.y),
                          ImVec2(canvasPos.x + 80.0f, canvasPos.y + canvasSize.y),
                          IM_COL32(220, 80, 80, 100), 1.5f);
        break;
    }
    case BackgroundStyle::BluePrint: {
        uint32_t minorColor = IM_COL32(255, 255, 255, 30);
        uint32_t majorColor = IM_COL32(255, 255, 255, 70);
        for (float x = offsetX; x < canvasSize.x; x += gridStep)
            drawList->AddLine(ImVec2(canvasPos.x + x, canvasPos.y),
                              ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), minorColor);
        for (float y = offsetY; y < canvasSize.y; y += gridStep)
            drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
                              ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), minorColor);
        float majorStep = gridStep * 5.0f;
        float mox = std::fmod(m_panOffset.x * m_zoom, majorStep);
        float moy = std::fmod(m_panOffset.y * m_zoom, majorStep);
        if (mox < 0) mox += majorStep;
        if (moy < 0) moy += majorStep;
        for (float x = mox; x < canvasSize.x; x += majorStep)
            drawList->AddLine(ImVec2(canvasPos.x + x, canvasPos.y),
                              ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), majorColor, 2.0f);
        for (float y = moy; y < canvasSize.y; y += majorStep)
            drawList->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
                              ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), majorColor, 2.0f);
        break;
    }
    case BackgroundStyle::DotGrid: {
        uint32_t dotColor = IM_COL32(150, 150, 150, 100);
        for (float x = offsetX; x < canvasSize.x; x += gridStep)
            for (float y = offsetY; y < canvasSize.y; y += gridStep)
                drawList->AddCircleFilled(ImVec2(canvasPos.x + x, canvasPos.y + y), 1.5f, dotColor);
        break;
    }
    case BackgroundStyle::Plain:
        break;
    }
    } // end if (!m_overlayMode)

    // Render existing strokes
    for (auto& stroke : board.getStrokes()) {
        renderStroke(stroke, drawList);
    }

    // Render current stroke being drawn
    if (m_drawing) {
        renderStroke(m_currentStroke, drawList);
    }

    drawList->PopClipRect();

    // Invisible button for input
    ImGui::SetCursorScreenPos(canvasPos);
    ImGui::InvisibleButton("##canvas", canvasSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

    bool isHovered = ImGui::IsItemHovered();
    ImVec2 mousePos = ImGui::GetIO().MousePos;

    // Middle mouse button for panning
    if (isHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        m_panOffset.x += delta.x / m_zoom;
        m_panOffset.y += delta.y / m_zoom;
    }

    // Scroll wheel for zoom
    if (isHovered) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            float oldZoom = m_zoom;
            m_zoom *= (wheel > 0) ? 1.1f : 0.9f;
            m_zoom = std::clamp(m_zoom, 0.1f, 10.0f);
            // Zoom toward mouse position
            float zr = m_zoom / oldZoom;
            m_panOffset.x = mousePos.x / m_zoom - (mousePos.x / oldZoom - m_panOffset.x);
            m_panOffset.y = mousePos.y / m_zoom - (mousePos.y / oldZoom - m_panOffset.y);
            (void)zr;
        }
    }

    // Check if drawing is allowed (students need permission from teacher)
    bool drawingAllowed = true;
    if (m_network.isConnected() && m_network.getRole() == NetRole::Client) {
        drawingAllowed = m_network.studentCanDraw();
    }

    // Left mouse button for drawing
    if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && drawingAllowed) {
        if (m_currentTool == ToolType::Eraser) {
            handleEraser(mousePos);
        }
        else if (m_currentTool == ToolType::Text) {
            if (std::strlen(m_textInput) > 0) {
                Stroke s;
                s.tool = ToolType::Text;
                s.color = m_packedColor;
                s.thickness = m_thickness;
                s.text = m_textInput;
                Point p = screenToBoard(mousePos);
                s.points.push_back(p);
                board.addStroke(s);
                if (m_network.isConnected()) m_network.sendStroke(s);
            }
        }
        else {
            m_drawing = true;
            m_currentStroke = {};
            m_currentStroke.tool = m_currentTool;
            m_currentStroke.color = m_packedColor;
            m_currentStroke.thickness = m_thickness;
            Point p = screenToBoard(mousePos);
            m_currentStroke.points.push_back(p);
        }
    }

    if (m_drawing && isHovered) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            Point p = screenToBoard(mousePos);
            if (m_currentTool == ToolType::Pencil) {
                // Add point for freehand
                m_currentStroke.points.push_back(p);
            }
            else {
                // For shapes: update the second point
                if (m_currentStroke.points.size() < 2)
                    m_currentStroke.points.push_back(p);
                else
                    m_currentStroke.points[1] = p;
            }
        }
    }

    if (m_drawing && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        finishStroke();
    }

    // Eraser dragging
    if (m_currentTool == ToolType::Eraser && isHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left) && drawingAllowed) {
        handleEraser(mousePos);
    }

    // Show eraser cursor
    if (m_currentTool == ToolType::Eraser && isHovered) {
        drawList->AddCircle(mousePos, m_eraserRadius, IM_COL32(255, 100, 100, 180), 32, 1.5f);
    }

    // Show "drawing disabled" message for students without permission
    if (!drawingAllowed && isHovered) {
        drawList->AddText(ImVec2(canvasPos.x + 10, canvasPos.y + canvasSize.y - 30),
            IM_COL32(255, 80, 80, 200), "Drawing disabled by teacher");
    }

    // Ruler measurement display
    if (m_currentTool == ToolType::Ruler && m_drawing && m_currentStroke.points.size() == 2) {
        Point p0 = m_currentStroke.points[0];
        Point p1 = m_currentStroke.points[1];
        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        char distLabel[64];
        snprintf(distLabel, sizeof(distLabel), "%.1f px", dist);
        ImVec2 mid = boardToScreen({(p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f});
        drawList->AddText(ImVec2(mid.x + 5, mid.y - 15), IM_COL32(255, 255, 0, 255), distLabel);
    }

    // Keyboard shortcuts
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
            board.undo();
            if (m_network.isConnected()) m_network.sendCommand(NetMsgType::Undo);
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
            board.redo();
            if (m_network.isConnected()) m_network.sendCommand(NetMsgType::Redo);
        }
    }
}

// ── Stroke Rendering ────────────────────────────────────────────────────────
void BoardApp::renderStroke(const Stroke& stroke, ImDrawList* drawList) {
    if (stroke.points.empty()) return;
    float thick = stroke.thickness * m_zoom;
    if (thick < 0.5f) thick = 0.5f;

    switch (stroke.tool) {
    case ToolType::Pencil: {
        if (stroke.points.size() < 2) {
            ImVec2 p = boardToScreen(stroke.points[0]);
            drawList->AddCircleFilled(p, thick * 0.5f, stroke.color);
        }
        else {
            for (size_t i = 0; i + 1 < stroke.points.size(); ++i) {
                ImVec2 a = boardToScreen(stroke.points[i]);
                ImVec2 b = boardToScreen(stroke.points[i + 1]);
                drawList->AddLine(a, b, stroke.color, thick);
            }
        }
        break;
    }
    case ToolType::Line:
    case ToolType::Ruler: {
        if (stroke.points.size() >= 2) {
            ImVec2 a = boardToScreen(stroke.points[0]);
            ImVec2 b = boardToScreen(stroke.points[1]);
            drawList->AddLine(a, b, stroke.color, thick);

            // Ruler: show measurement on finished strokes too
            if (stroke.tool == ToolType::Ruler) {
                float dx = stroke.points[1].x - stroke.points[0].x;
                float dy = stroke.points[1].y - stroke.points[0].y;
                float dist = std::sqrt(dx * dx + dy * dy);
                char label[64];
                snprintf(label, sizeof(label), "%.1f", dist);
                ImVec2 mid = {(a.x + b.x) * 0.5f + 5, (a.y + b.y) * 0.5f - 15};
                drawList->AddText(mid, IM_COL32(255, 255, 0, 255), label);
            }
        }
        break;
    }
    case ToolType::Rectangle: {
        if (stroke.points.size() >= 2) {
            ImVec2 a = boardToScreen(stroke.points[0]);
            ImVec2 b = boardToScreen(stroke.points[1]);
            drawList->AddRect(a, b, stroke.color, 0.0f, 0, thick);
        }
        break;
    }
    case ToolType::Circle: {
        if (stroke.points.size() >= 2) {
            ImVec2 a = boardToScreen(stroke.points[0]);
            ImVec2 b = boardToScreen(stroke.points[1]);
            float radius = std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
            drawList->AddCircle(a, radius, stroke.color, 64, thick);
        }
        break;
    }
    case ToolType::Arrow: {
        if (stroke.points.size() >= 2) {
            ImVec2 a = boardToScreen(stroke.points[0]);
            ImVec2 b = boardToScreen(stroke.points[1]);
            drawList->AddLine(a, b, stroke.color, thick);
            // Arrowhead
            float dx = b.x - a.x;
            float dy = b.y - a.y;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 1.0f) {
                float ux = dx / len, uy = dy / len;
                float headSize = std::max(8.0f, thick * 4.0f);
                ImVec2 p1 = { b.x - headSize * (ux * 0.866f - uy * 0.5f),
                              b.y - headSize * (uy * 0.866f + ux * 0.5f) };
                ImVec2 p2 = { b.x - headSize * (ux * 0.866f + uy * 0.5f),
                              b.y - headSize * (uy * 0.866f - ux * 0.5f) };
                drawList->AddTriangleFilled(b, p1, p2, stroke.color);
            }
        }
        break;
    }
    case ToolType::Text: {
        if (!stroke.points.empty()) {
            ImVec2 pos = boardToScreen(stroke.points[0]);
            float fontSize = 14.0f * m_zoom;
            (void)fontSize; // ImGui default font doesn't support scaling here
            drawList->AddText(pos, stroke.color, stroke.text.c_str());
        }
        break;
    }
    case ToolType::Eraser:
        // Eraser doesn't render — it removes strokes
        break;
    }
}

void BoardApp::finishStroke() {
    if (!m_drawing) return;
    m_drawing = false;

    if (m_currentStroke.points.empty()) return;
    // For shapes, need at least 2 points
    if (m_currentStroke.tool != ToolType::Pencil && m_currentStroke.points.size() < 2) return;

    Board& board = m_boards[m_activeBoard];
    board.addStroke(m_currentStroke);

    // Send to network peer
    if (m_network.isConnected()) {
        m_network.sendStroke(m_currentStroke);
    }

    m_currentStroke = {};
}

void BoardApp::handleEraser(const ImVec2& mousePos) {
    Board& board = m_boards[m_activeBoard];
    auto& strokes = const_cast<std::vector<Stroke>&>(board.getStrokes());

    float eraseR = m_eraserRadius / m_zoom;
    Point mp = screenToBoard(mousePos);

    bool erased = false;
    for (int i = static_cast<int>(strokes.size()) - 1; i >= 0; --i) {
        for (auto& pt : strokes[i].points) {
            float dx = pt.x - mp.x;
            float dy = pt.y - mp.y;
            if (dx * dx + dy * dy < eraseR * eraseR) {
                strokes.erase(strokes.begin() + i);
                erased = true;
                break;
            }
        }
    }
    (void)erased;
}

// ── Network Panel ───────────────────────────────────────────────────────────
void BoardApp::renderNetworkPanel() {
    ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Collaboration##Net", &m_showNetwork)) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Status: %s",
        m_network.getStatusMessage().c_str());
    ImGui::Separator();

    if (!m_network.isConnected() && m_network.getRole() == NetRole::None) {
        ImGui::Text("Host a session (Teacher):");
        ImGui::PushItemWidth(100);
        ImGui::InputInt("Port##Host", &m_netPort);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Start Hosting##H")) {
            m_network.host(m_netPort);
        }

        ImGui::Separator();
        ImGui::Text("Join a session (Student):");
        ImGui::PushItemWidth(150);
        ImGui::InputText("IP##Join", m_netIP, sizeof(m_netIP));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::InputInt("Port##Join", &m_netPort);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Connect##J")) {
            m_network.join(m_netIP, m_netPort);
        }
    }
    else {
        if (m_network.isConnected()) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Connected!");
            if (ImGui::Button("Sync Board to Peer##Sync")) {
                m_network.sendBoardSync(m_boards[m_activeBoard]);
            }

            ImGui::Separator();

            // Teacher permission controls (host only)
            if (m_network.getRole() == NetRole::Host) {
                ImGui::Text("Student Permissions:");
                bool allow = m_network.studentCanDraw();
                if (ImGui::Checkbox("Allow Student to Draw", &allow)) {
                    m_network.sendDrawPermission(allow);
                }
                ImGui::TextWrapped("When enabled, the student can use pencil and other drawing tools.");
            }

            // Student permission status (client only)
            if (m_network.getRole() == NetRole::Client) {
                ImGui::Separator();
                if (m_network.studentCanDraw()) {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                        "Drawing: ENABLED by teacher");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                        "Drawing: DISABLED by teacher");
                }
            }
        }
        if (ImGui::Button("Disconnect##D")) {
            m_network.disconnect();
        }
    }

    // --- WebSocket Server Controls ---
    static int ws_port = 9002;
    ImGui::Separator();
    ImGui::Text("WebSocket Server (for web clients):");
    ImGui::PushItemWidth(100);
    ImGui::InputInt("Port##WS", &ws_port);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (!isWebSocketServerRunning()) {
        if (ImGui::Button("Start WebSocket Server")) {
            startWebSocketServer(ws_port);
        }
    } else {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Running on port %d", ws_port);
        ImGui::SameLine();
        if (ImGui::Button("Stop WebSocket Server")) {
            stopWebSocketServer();
        }
    }
    ImGui::Separator();

    ImGui::End();
}

// ── Network Polling ─────────────────────────────────────────────────────────
void BoardApp::pollNetwork() {
    // Process incoming strokes
    while (m_network.hasIncomingStroke()) {
        Stroke s = m_network.popIncomingStroke();
        m_boards[m_activeBoard].addStroke(s);
    }

    // Process incoming commands
    while (m_network.hasIncomingCommand()) {
        auto cmd = m_network.popIncomingCommand();
        Board& board = m_boards[m_activeBoard];
        switch (cmd.type) {
        case NetMsgType::ClearBoard:
            board.clear();
            break;
        case NetMsgType::Undo:
            board.undo();
            break;
        case NetMsgType::Redo:
            board.redo();
            break;
        case NetMsgType::BoardSwitch:
            if (cmd.boardIndex >= 0 && cmd.boardIndex < NUM_BOARDS)
                m_activeBoard = cmd.boardIndex;
            break;
        case NetMsgType::BoardSync:
            board.deserialize(cmd.boardData);
            break;
        default:
            break;
        }
    }
}
