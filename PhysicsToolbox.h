#pragma once

// ── PhysicsToolbox ──────────────────────────────────────────────────────────
// Reference panel with physics constants and interactive calculators.
// Tabs: Constants, Kinematics (SUVAT), Forces, Energy, Waves, Electricity.
// ─────────────────────────────────────────────────────────────────────────────

class PhysicsToolbox {
public:
    PhysicsToolbox();
    void render(bool* open);

private:
    void renderConstants();
    void renderKinematics();
    void renderForces();
    void renderEnergy();
    void renderWaves();
    void renderElectricity();

    // Kinematics (SUVAT)
    float m_kinU, m_kinV, m_kinA, m_kinT, m_kinS;

    // Forces
    float m_frcMass, m_frcAccel, m_frcForce;
    float m_frcMu, m_frcNormal, m_frcFriction;
    float m_pendulumL, m_pendulumG, m_pendulumT;

    // Energy
    float m_nrgMass, m_nrgVelocity, m_nrgHeight, m_nrgGravity;
    float m_nrgKE, m_nrgPE, m_nrgTotal;
    float m_workForce, m_workDist, m_workAngle, m_workResult;

    // Waves
    float m_wavFreq, m_wavLambda, m_wavSpeed, m_wavPeriod;

    // Electricity
    float m_elVoltage, m_elCurrent, m_elResistance;
    float m_elPower;
};
