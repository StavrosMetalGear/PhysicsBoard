#pragma once

#include <vector>

// ── ProjectileSimulator ─────────────────────────────────────────────────────
// Interactive projectile motion simulator for physics lessons.
// Adjustable velocity, angle, gravity, and initial height.
// Shows animated trajectory with real-time position, range, max height,
// and flight time calculations.
// ─────────────────────────────────────────────────────────────────────────────

class ProjectileSimulator {
public:
    ProjectileSimulator();
    void render(bool* open);

private:
    // Input parameters
    float m_velocity;       // m/s
    float m_angle;          // degrees
    float m_gravity;        // m/s^2
    float m_height;         // initial height m
    float m_speed;          // simulation speed multiplier

    // Simulation state
    bool  m_running;
    float m_time;           // current simulation time
    float m_totalTime;      // total flight time
    float m_maxHeight;
    float m_range;

    // Trajectory data for plotting
    std::vector<double> m_xData;
    std::vector<double> m_yData;

    // Current animated position
    float m_curX, m_curY;
    float m_curVx, m_curVy;

    void calculate();
    void reset();
};
