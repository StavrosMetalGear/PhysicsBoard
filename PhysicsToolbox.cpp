#include "PhysicsToolbox.h"
#include "imgui.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PhysicsToolbox::PhysicsToolbox()
    : m_kinU(0), m_kinV(0), m_kinA(9.81f), m_kinT(0), m_kinS(0)
    , m_frcMass(1.0f), m_frcAccel(9.81f), m_frcForce(9.81f)
    , m_frcMu(0.3f), m_frcNormal(9.81f), m_frcFriction(2.943f)
    , m_pendulumL(1.0f), m_pendulumG(9.81f), m_pendulumT(0.0f)
    , m_nrgMass(1.0f), m_nrgVelocity(0.0f), m_nrgHeight(1.0f), m_nrgGravity(9.81f)
    , m_nrgKE(0.0f), m_nrgPE(9.81f), m_nrgTotal(9.81f)
    , m_workForce(10.0f), m_workDist(1.0f), m_workAngle(0.0f), m_workResult(10.0f)
    , m_wavFreq(440.0f), m_wavLambda(0.773f), m_wavSpeed(340.0f), m_wavPeriod(0.00227f)
    , m_elVoltage(12.0f), m_elCurrent(2.0f), m_elResistance(6.0f), m_elPower(24.0f)
{}

void PhysicsToolbox::render(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(520, 480), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Physics Toolbox", open)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##PhysicsTabs")) {
        if (ImGui::BeginTabItem("Constants"))    { renderConstants();   ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Kinematics"))   { renderKinematics();  ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Forces"))       { renderForces();      ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Energy"))       { renderEnergy();      ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Waves"))        { renderWaves();       ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Electricity"))  { renderElectricity(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

// ── Constants Reference ─────────────────────────────────────────────────────
void PhysicsToolbox::renderConstants() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Fundamental Constants");
    ImGui::Separator();

    if (ImGui::BeginTable("##ConstTable", 3,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Constant", ImGuiTableColumnFlags_None, 0.45f);
        ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_None, 0.15f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None, 0.40f);
        ImGui::TableHeadersRow();

        auto row = [](const char* name, const char* sym, const char* val) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name);
            ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "%s", sym);
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(val);
        };

        row("Gravitational accel.",    "g",       "9.807 m/s\xc2\xb2");
        row("Gravitational const.",    "G",       "6.674e-11 N m\xc2\xb2/kg\xc2\xb2");
        row("Speed of light",         "c",       "2.998e+08 m/s");
        row("Planck constant",        "h",       "6.626e-34 J s");
        row("Boltzmann constant",     "k_B",     "1.381e-23 J/K");
        row("Elementary charge",      "e",       "1.602e-19 C");
        row("Electron mass",          "m_e",     "9.109e-31 kg");
        row("Proton mass",            "m_p",     "1.673e-27 kg");
        row("Avogadro number",        "N_A",     "6.022e+23 /mol");
        row("Gas constant",           "R",       "8.314 J/(mol K)");
        row("Vacuum permittivity",    "\xce\xb5\xe2\x82\x80", "8.854e-12 F/m");
        row("Vacuum permeability",    "\xce\xbc\xe2\x82\x80", "1.257e-06 H/m");
        row("Stefan-Boltzmann",       "\xcf\x83", "5.670e-08 W/(m\xc2\xb2 K\xe2\x81\xb4)");
        row("Coulomb constant",       "k_e",     "8.988e+09 N m\xc2\xb2/C\xc2\xb2");
        row("Atm pressure",           "P_0",     "1.013e+05 Pa");

        ImGui::EndTable();
    }
}

// ── Kinematics (SUVAT) ─────────────────────────────────────────────────────
void PhysicsToolbox::renderKinematics() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "SUVAT Equations of Motion");
    ImGui::TextWrapped("Enter initial velocity (u), acceleration (a), and time (t) to compute final velocity (v) and displacement (s).");
    ImGui::Separator();

    ImGui::PushItemWidth(150);
    ImGui::InputFloat("u  Initial vel (m/s)##K", &m_kinU, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("a  Acceleration (m/s\xc2\xb2)##K", &m_kinA, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("t  Time (s)##K", &m_kinT, 0.1f, 1.0f, "%.3f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Calculate v & s##K")) {
        m_kinV = m_kinU + m_kinA * m_kinT;
        m_kinS = m_kinU * m_kinT + 0.5f * m_kinA * m_kinT * m_kinT;
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "v = u + at = %.4f m/s", m_kinV);
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "s = ut + 1/2 at\xc2\xb2 = %.4f m", m_kinS);

    float vSquared = m_kinV * m_kinV;
    float uSquared = m_kinU * m_kinU;
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1, 1), "v\xc2\xb2 = u\xc2\xb2 + 2as  =>  v\xc2\xb2 = %.4f", uSquared + 2.0f * m_kinA * m_kinS);
    (void)vSquared;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1), "Equations reference:");
    ImGui::BulletText("v = u + at");
    ImGui::BulletText("s = ut + 1/2 at\xc2\xb2");
    ImGui::BulletText("v\xc2\xb2 = u\xc2\xb2 + 2as");
    ImGui::BulletText("s = 1/2 (u+v) t");
}

// ── Forces ──────────────────────────────────────────────────────────────────
void PhysicsToolbox::renderForces() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Newton's 2nd Law & Friction");
    ImGui::Separator();

    // F = ma
    ImGui::Text("F = m * a");
    ImGui::PushItemWidth(130);
    ImGui::InputFloat("Mass (kg)##F", &m_frcMass, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Acceleration (m/s\xc2\xb2)##F", &m_frcAccel, 0.1f, 1.0f, "%.3f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Calc Force##F")) m_frcForce = m_frcMass * m_frcAccel;
    ImGui::SameLine();
    if (ImGui::Button("Calc Mass##F"))  { if (m_frcAccel != 0) m_frcMass = m_frcForce / m_frcAccel; }
    ImGui::SameLine();
    if (ImGui::Button("Calc Accel##F")) { if (m_frcMass != 0)  m_frcAccel = m_frcForce / m_frcMass; }

    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "F = %.4f N", m_frcForce);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1, 1), "Weight = %.4f N", m_frcMass * 9.81f);

    ImGui::Separator();
    ImGui::Text("Friction: f = mu * N");
    ImGui::PushItemWidth(130);
    ImGui::InputFloat("mu (coeff)##FR", &m_frcMu, 0.01f, 0.1f, "%.3f");
    ImGui::InputFloat("Normal Force N##FR", &m_frcNormal, 0.1f, 1.0f, "%.3f");
    ImGui::PopItemWidth();
    if (ImGui::Button("Calc Friction##FR")) m_frcFriction = m_frcMu * m_frcNormal;
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "f = %.4f N", m_frcFriction);

    ImGui::Separator();
    ImGui::Text("Simple Pendulum: T = 2pi sqrt(L/g)");
    ImGui::PushItemWidth(130);
    ImGui::InputFloat("Length L (m)##PD", &m_pendulumL, 0.01f, 0.1f, "%.3f");
    ImGui::InputFloat("Gravity g (m/s\xc2\xb2)##PD", &m_pendulumG, 0.1f, 1.0f, "%.3f");
    ImGui::PopItemWidth();
    if (ImGui::Button("Calc Period##PD")) {
        if (m_pendulumG > 0 && m_pendulumL > 0)
            m_pendulumT = 2.0f * static_cast<float>(M_PI) * std::sqrt(m_pendulumL / m_pendulumG);
    }
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "T = %.4f s  |  f = %.4f Hz",
        m_pendulumT, (m_pendulumT > 0) ? 1.0f / m_pendulumT : 0.0f);
}

// ── Energy ──────────────────────────────────────────────────────────────────
void PhysicsToolbox::renderEnergy() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Kinetic & Potential Energy");
    ImGui::Separator();

    ImGui::PushItemWidth(130);
    ImGui::InputFloat("Mass (kg)##E", &m_nrgMass, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Velocity (m/s)##E", &m_nrgVelocity, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Height (m)##E", &m_nrgHeight, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Gravity (m/s\xc2\xb2)##E", &m_nrgGravity, 0.1f, 1.0f, "%.3f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Calculate##E")) {
        m_nrgKE = 0.5f * m_nrgMass * m_nrgVelocity * m_nrgVelocity;
        m_nrgPE = m_nrgMass * m_nrgGravity * m_nrgHeight;
        m_nrgTotal = m_nrgKE + m_nrgPE;
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1, 0.6f, 0.3f, 1), "KE = 1/2 mv\xc2\xb2 = %.4f J", m_nrgKE);
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1, 1), "PE = mgh     = %.4f J", m_nrgPE);
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1),    "Total E      = %.4f J", m_nrgTotal);

    ImGui::Separator();
    ImGui::Text("Work: W = F d cos(theta)");
    ImGui::PushItemWidth(130);
    ImGui::InputFloat("Force (N)##W", &m_workForce, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Distance (m)##W", &m_workDist, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Angle (deg)##W", &m_workAngle, 1.0f, 10.0f, "%.1f");
    ImGui::PopItemWidth();
    if (ImGui::Button("Calc Work##W")) {
        m_workResult = m_workForce * m_workDist *
            std::cos(m_workAngle * static_cast<float>(M_PI) / 180.0f);
    }
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "W = %.4f J", m_workResult);
}

// ── Waves ───────────────────────────────────────────────────────────────────
void PhysicsToolbox::renderWaves() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Wave Equation: v = f * lambda");
    ImGui::Separator();

    ImGui::PushItemWidth(150);
    ImGui::InputFloat("Frequency f (Hz)##WV", &m_wavFreq, 1.0f, 10.0f, "%.3f");
    ImGui::InputFloat("Wavelength lambda (m)##WV", &m_wavLambda, 0.001f, 0.1f, "%.4f");
    ImGui::InputFloat("Speed v (m/s)##WV", &m_wavSpeed, 1.0f, 10.0f, "%.3f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Calc Speed##WV")) {
        m_wavSpeed = m_wavFreq * m_wavLambda;
    }
    ImGui::SameLine();
    if (ImGui::Button("Calc Freq##WV")) {
        if (m_wavLambda != 0) m_wavFreq = m_wavSpeed / m_wavLambda;
    }
    ImGui::SameLine();
    if (ImGui::Button("Calc Lambda##WV")) {
        if (m_wavFreq != 0) m_wavLambda = m_wavSpeed / m_wavFreq;
    }

    m_wavPeriod = (m_wavFreq != 0) ? 1.0f / m_wavFreq : 0.0f;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "v = %.4f m/s", m_wavSpeed);
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "T = 1/f = %.6f s", m_wavPeriod);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1), "Quick reference:");
    ImGui::BulletText("Sound in air:  ~340 m/s");
    ImGui::BulletText("Sound in water: ~1480 m/s");
    ImGui::BulletText("Light in vacuum: 3.0e+08 m/s");
    ImGui::BulletText("Visible light: 380-700 nm");
}

// ── Electricity ─────────────────────────────────────────────────────────────
void PhysicsToolbox::renderElectricity() {
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Ohm's Law & Electrical Power");
    ImGui::Separator();

    ImGui::Text("V = I * R");
    ImGui::PushItemWidth(140);
    ImGui::InputFloat("Voltage V (V)##EL", &m_elVoltage, 0.1f, 1.0f, "%.3f");
    ImGui::InputFloat("Current I (A)##EL", &m_elCurrent, 0.01f, 0.1f, "%.4f");
    ImGui::InputFloat("Resistance R (Ohm)##EL", &m_elResistance, 0.1f, 1.0f, "%.3f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Calc V##EL")) {
        m_elVoltage = m_elCurrent * m_elResistance;
    }
    ImGui::SameLine();
    if (ImGui::Button("Calc I##EL")) {
        if (m_elResistance != 0) m_elCurrent = m_elVoltage / m_elResistance;
    }
    ImGui::SameLine();
    if (ImGui::Button("Calc R##EL")) {
        if (m_elCurrent != 0) m_elResistance = m_elVoltage / m_elCurrent;
    }

    m_elPower = m_elVoltage * m_elCurrent;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "V = %.4f V", m_elVoltage);
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "I = %.4f A", m_elCurrent);
    ImGui::TextColored(ImVec4(1, 1, 0.4f, 1), "R = %.4f Ohm", m_elResistance);
    ImGui::TextColored(ImVec4(1, 0.6f, 0.3f, 1), "P = V*I = %.4f W", m_elPower);
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1, 1), "P = I\xc2\xb2*R = %.4f W",
        m_elCurrent * m_elCurrent * m_elResistance);
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1, 1), "P = V\xc2\xb2/R = %.4f W",
        (m_elResistance != 0) ? (m_elVoltage * m_elVoltage / m_elResistance) : 0.0f);
}
