#include "FunctionPlotter.h"
#include "imgui.h"
#include "implot.h"
#include <cmath>
#include <cstring>

FunctionPlotter::FunctionPlotter()
    : m_xMin(-10.0f), m_xMax(10.0f), m_numPoints(500) {
    // Default first slot with a sample function
    std::strcpy(m_slots[0].expression, "sin(x)");
    m_slots[0].enabled = true;
    m_slots[0].color[0] = 0.0f; m_slots[0].color[1] = 1.0f; m_slots[0].color[2] = 0.4f;

    // Preset colors for other slots
    float presets[][3] = {
        {0.2f, 0.6f, 1.0f}, {1.0f, 0.4f, 0.4f}, {1.0f, 0.8f, 0.2f},
        {0.8f, 0.4f, 1.0f}, {0.4f, 1.0f, 1.0f}, {1.0f, 0.6f, 0.2f},
        {0.6f, 1.0f, 0.6f}
    };
    for (int i = 1; i < MAX_FUNCTIONS; ++i) {
        int ci = (i - 1) % 7;
        m_slots[i].color[0] = presets[ci][0];
        m_slots[i].color[1] = presets[ci][1];
        m_slots[i].color[2] = presets[ci][2];
    }
}

void FunctionPlotter::evaluateSlot(int idx) {
    FunctionSlot& s = m_slots[idx];
    s.xData.resize(m_numPoints);
    s.yData.resize(m_numPoints);
    s.valid = true;
    s.errorMsg.clear();

    std::string expr(s.expression);
    if (expr.empty()) {
        s.valid = false;
        s.errorMsg = "Empty expression";
        return;
    }

    double dx = (m_xMax - m_xMin) / (m_numPoints - 1);
    for (int i = 0; i < m_numPoints; ++i) {
        double x = m_xMin + i * dx;
        s.xData[i] = x;
        double y = m_parser.evaluate(expr, x);
        if (m_parser.hasError()) {
            s.valid = false;
            s.errorMsg = m_parser.getError();
            return;
        }
        s.yData[i] = y;
    }
}

void FunctionPlotter::render(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Function Plotter", open)) {
        ImGui::End();
        return;
    }

    // Range controls
    ImGui::PushItemWidth(120);
    ImGui::DragFloat("x Min##FP", &m_xMin, 0.1f, -1000.0f, m_xMax - 0.1f);
    ImGui::SameLine();
    ImGui::DragFloat("x Max##FP", &m_xMax, 0.1f, m_xMin + 0.1f, 1000.0f);
    ImGui::SameLine();
    ImGui::SliderInt("Points##FP", &m_numPoints, 100, 2000);
    ImGui::PopItemWidth();

    ImGui::Separator();

    // Function input slots
    bool anyChanged = false;
    for (int i = 0; i < MAX_FUNCTIONS; ++i) {
        ImGui::PushID(i);
        FunctionSlot& s = m_slots[i];

        ImGui::Checkbox("##en", &s.enabled);
        ImGui::SameLine();
        ImGui::ColorEdit3("##col", s.color, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 80);
        char label[32];
        snprintf(label, sizeof(label), "f%d(x)##input", i + 1);
        if (ImGui::InputText(label, s.expression, sizeof(s.expression),
            ImGuiInputTextFlags_EnterReturnsTrue)) {
            s.enabled = true;
            anyChanged = true;
        }
        ImGui::PopItemWidth();

        if (!s.valid && s.enabled) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", s.errorMsg.c_str());
        }

        ImGui::PopID();
    }

    // Evaluate all enabled slots
    if (ImGui::Button("Plot All##FP") || anyChanged) {
        for (int i = 0; i < MAX_FUNCTIONS; ++i) {
            if (m_slots[i].enabled && std::strlen(m_slots[i].expression) > 0)
                evaluateSlot(i);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All##FP")) {
        for (int i = 0; i < MAX_FUNCTIONS; ++i) {
            m_slots[i].expression[0] = '\0';
            m_slots[i].enabled = false;
            m_slots[i].xData.clear();
            m_slots[i].yData.clear();
        }
    }

    ImGui::Separator();

    // Plot area
    if (ImPlot::BeginPlot("##FunctionPlot", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("x", "f(x)");
        ImPlot::SetupAxisLimits(ImAxis_X1, m_xMin, m_xMax, ImPlotCond_Always);

        for (int i = 0; i < MAX_FUNCTIONS; ++i) {
            FunctionSlot& s = m_slots[i];
            if (!s.enabled || !s.valid || s.xData.empty()) continue;

            char plotLabel[32];
            snprintf(plotLabel, sizeof(plotLabel), "f%d", i + 1);

            ImPlot::PushStyleColor(ImPlotCol_Line,
                ImVec4(s.color[0], s.color[1], s.color[2], 1.0f));
            ImPlot::PlotLine(plotLabel, s.xData.data(), s.yData.data(),
                static_cast<int>(s.xData.size()));
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }

    ImGui::End();
}
