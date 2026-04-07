#pragma once

#include "ExpressionParser.h"
#include <string>
#include <vector>

// ── FunctionPlotter ─────────────────────────────────────────────────────────
// Small window that lets the user type math expressions (e.g. "sin(x)")
// and renders the corresponding plots via ImPlot.
// Supports up to 8 simultaneous function slots with individual colors.
// ─────────────────────────────────────────────────────────────────────────────

struct FunctionSlot {
    char   expression[256] = "";
    bool   enabled         = false;
    bool   valid           = true;
    float  color[3]        = { 1.0f, 1.0f, 0.0f };
    std::string errorMsg;
    std::vector<double> xData;
    std::vector<double> yData;
};

class FunctionPlotter {
public:
    static constexpr int MAX_FUNCTIONS = 8;

    FunctionPlotter();

    // Render the plotter window (call each frame inside ImGui context)
    void render(bool* open);

private:
    FunctionSlot       m_slots[MAX_FUNCTIONS];
    ExpressionParser   m_parser;
    float              m_xMin;
    float              m_xMax;
    int                m_numPoints;

    void evaluateSlot(int idx);
};
