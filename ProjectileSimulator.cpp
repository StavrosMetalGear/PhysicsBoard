#include "ProjectileSimulator.h"
#include "imgui.h"
#include "implot.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ProjectileSimulator::ProjectileSimulator()
    : m_velocity(25.0f), m_angle(45.0f), m_gravity(9.81f), m_height(0.0f)
    , m_speed(1.0f)
    , m_running(false), m_time(0.0f), m_totalTime(0.0f)
    , m_maxHeight(0.0f), m_range(0.0f)
    , m_curX(0.0f), m_curY(0.0f), m_curVx(0.0f), m_curVy(0.0f)
{
    calculate();
}

void ProjectileSimulator::calculate() {
    float angleRad = m_angle * static_cast<float>(M_PI) / 180.0f;
    float vx = m_velocity * std::cos(angleRad);
    float vy = m_velocity * std::sin(angleRad);

    // Time of flight: y = h + vy*t - 0.5*g*t^2 = 0
    // t = (vy + sqrt(vy^2 + 2*g*h)) / g
    float disc = vy * vy + 2.0f * m_gravity * m_height;
    if (disc < 0.0f) disc = 0.0f;
    m_totalTime = (vy + std::sqrt(disc)) / m_gravity;
    if (m_totalTime < 0.01f) m_totalTime = 0.01f;

    // Max height at t_peak = vy / g
    float tPeak = vy / m_gravity;
    m_maxHeight = m_height + vy * tPeak - 0.5f * m_gravity * tPeak * tPeak;
    if (m_maxHeight < 0.0f) m_maxHeight = 0.0f;

    // Range
    m_range = vx * m_totalTime;

    // Generate trajectory curve
    int numPoints = 300;
    m_xData.resize(numPoints);
    m_yData.resize(numPoints);
    float dt = m_totalTime / static_cast<float>(numPoints - 1);
    for (int i = 0; i < numPoints; ++i) {
        float t = i * dt;
        m_xData[i] = static_cast<double>(vx * t);
        double y = static_cast<double>(m_height + vy * t - 0.5f * m_gravity * t * t);
        m_yData[i] = (y < 0.0) ? 0.0 : y;
    }
}

void ProjectileSimulator::reset() {
    m_running = false;
    m_time = 0.0f;
    m_curX = 0.0f;
    m_curY = m_height;
    m_curVx = 0.0f;
    m_curVy = 0.0f;
    calculate();
}

void ProjectileSimulator::render(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(700, 560), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Projectile Motion Simulator", open)) {
        ImGui::End();
        return;
    }

    // ── Controls ────────────────────────────────────────────────────────
    bool changed = false;
    ImGui::PushItemWidth(140);
    changed |= ImGui::SliderFloat("Velocity (m/s)##PM", &m_velocity, 1.0f, 100.0f, "%.1f");
    ImGui::SameLine();
    changed |= ImGui::SliderFloat("Angle (deg)##PM", &m_angle, 0.0f, 90.0f, "%.1f");
    changed |= ImGui::SliderFloat("Gravity (m/s" "\xc2\xb2" ")##PM", &m_gravity, 0.1f, 25.0f, "%.2f");
    ImGui::SameLine();
    changed |= ImGui::SliderFloat("Height (m)##PM", &m_height, 0.0f, 50.0f, "%.1f");
    ImGui::SliderFloat("Speed##PM", &m_speed, 0.1f, 3.0f, "%.1fx");
    ImGui::PopItemWidth();

    if (changed) {
        calculate();
        if (!m_running) {
            m_curX = 0.0f;
            m_curY = m_height;
        }
    }

    // ── Buttons ─────────────────────────────────────────────────────────
    if (!m_running) {
        if (ImGui::Button("Launch##PM")) {
            m_running = true;
            m_time = 0.0f;
        }
    } else {
        if (ImGui::Button("Pause##PM")) {
            m_running = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset##PM")) {
        reset();
    }

    // ── Animation update ────────────────────────────────────────────────
    if (m_running) {
        m_time += ImGui::GetIO().DeltaTime * m_speed;
        if (m_time >= m_totalTime) {
            m_time = m_totalTime;
            m_running = false;
        }
        float angleRad = m_angle * static_cast<float>(M_PI) / 180.0f;
        float vx = m_velocity * std::cos(angleRad);
        float vy = m_velocity * std::sin(angleRad);
        m_curX = vx * m_time;
        m_curY = m_height + vy * m_time - 0.5f * m_gravity * m_time * m_time;
        if (m_curY < 0.0f) m_curY = 0.0f;
        m_curVx = vx;
        m_curVy = vy - m_gravity * m_time;
    }

    // ── Info display ────────────────────────────────────────────────────
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
        "Range: %.2f m  |  Max Height: %.2f m  |  Flight Time: %.2f s",
        m_range, m_maxHeight, m_totalTime);
    if (m_time > 0.0f) {
        float speed = std::sqrt(m_curVx * m_curVx + m_curVy * m_curVy);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f),
            "t=%.2fs  x=%.2fm  y=%.2fm  |v|=%.2f m/s",
            m_time, m_curX, m_curY, speed);
    }

    // ── Plot ────────────────────────────────────────────────────────────
    float plotXMax = (m_range > 1.0f) ? m_range * 1.15f : 10.0f;
    float plotYMax = (m_maxHeight > 1.0f) ? m_maxHeight * 1.3f : 10.0f;

    if (ImPlot::BeginPlot("##TrajectoryPlot", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Distance (m)", "Height (m)");
        ImPlot::SetupAxisLimits(ImAxis_X1, -plotXMax * 0.05, plotXMax, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -plotYMax * 0.05, plotYMax, ImPlotCond_Always);

        // Trajectory curve
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.2f, 0.7f, 1.0f, 1.0f));
        ImPlot::PlotLine("Trajectory", m_xData.data(), m_yData.data(),
            static_cast<int>(m_xData.size()));
        ImPlot::PopStyleColor();

        // Ground line
        double gx[] = { -plotXMax * 0.05, static_cast<double>(plotXMax) };
        double gy[] = { 0.0, 0.0 };
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.4f, 0.8f, 0.3f, 0.6f));
        ImPlot::PlotLine("Ground", gx, gy, 2);
        ImPlot::PopStyleColor();

        // Animated position
        if (m_time > 0.0f) {
            double px = static_cast<double>(m_curX);
            double py = static_cast<double>(m_curY);
            ImPlot::PushStyleColor(ImPlotCol_MarkerFill, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 8.0f);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotScatter("Position", &px, &py, 1);
            ImPlot::PopStyleVar();
            ImPlot::PopStyleColor();

            // Velocity vector (scaled arrow)
            float arrowScale = plotXMax * 0.03f;
            double vEndX = px + static_cast<double>(m_curVx * arrowScale);
            double vEndY = py + static_cast<double>(m_curVy * arrowScale);
            double arrowX[] = { px, vEndX };
            double arrowY[] = { py, vEndY };
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 1.0f, 0.2f, 0.9f));
            ImPlot::PlotLine("Velocity", arrowX, arrowY, 2);
            ImPlot::PopStyleColor();
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}
