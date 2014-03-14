/**
 *  @file   LArPandora/MicroBooNETransformationCalculator.cxx
 * 
 *  @brief  Implementation of the MicroBooNE transformation calculator class.
 * 
 *  $Log: $
 */

#include "MicroBooNETransformationCalculator.h"

#include <cmath>

namespace lar_pandora
{

// TODO read in from config
const float MicroBooNETransformationCalculator::m_thetaU = M_PI / 3.f;
const float MicroBooNETransformationCalculator::m_thetaV = M_PI / 3.f;
const float MicroBooNETransformationCalculator::m_sinUplusV = std::sin(MicroBooNETransformationCalculator::m_thetaU + MicroBooNETransformationCalculator::m_thetaV);
const float MicroBooNETransformationCalculator::m_sinUminusV = std::sin(MicroBooNETransformationCalculator::m_thetaU - MicroBooNETransformationCalculator::m_thetaV);
const float MicroBooNETransformationCalculator::m_sinU = std::sin(MicroBooNETransformationCalculator::m_thetaU);
const float MicroBooNETransformationCalculator::m_sinV = std::sin(MicroBooNETransformationCalculator::m_thetaV);
const float MicroBooNETransformationCalculator::m_cosU = std::cos(MicroBooNETransformationCalculator::m_thetaU);
const float MicroBooNETransformationCalculator::m_cosV = std::cos(MicroBooNETransformationCalculator::m_thetaV);
const float MicroBooNETransformationCalculator::m_H = 233.f;
const float MicroBooNETransformationCalculator::m_sigmaUVW = 1.f;

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::UVtoW(const float u, const float v) const
{
    return this->UVtoZ(u, v);
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::VWtoU(const float v, const float w) const
{
    return (w * m_sinUplusV - v * m_sinU + m_H * m_sinU * m_sinV) / m_sinV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::WUtoV(const float w, const float u) const
{
    return (w * m_sinUplusV - u * m_sinV + m_H * m_sinU * m_sinV) / m_sinU;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::UVtoY(const float u, const float v) const
{
    return (v * m_cosU - u * m_cosV + 0.5f * m_H * m_sinUminusV) / m_sinUplusV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::UVtoZ(const float u, const float v) const
{
    return (v * m_sinU + u * m_sinV - m_H * m_sinU * m_sinV) / m_sinUplusV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::YZtoU(const float y, const float z) const
{
    return z * m_cosU - (y - 0.5 * m_H) * m_sinU;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::YZtoV(const float y, const float z) const
{
    return z * m_cosV + (y + 0.5 * m_H) * m_sinV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::PUPVtoPW(const float pu, const float pv) const
{
    return (pu * m_sinV + pv * m_sinU) / m_sinUplusV;
}

//------------------------------------------------------------------------------------------------------------------------------------------
   
float MicroBooNETransformationCalculator::PVPWtoPU(const float pv, const float pw) const
{
    return (pw * m_sinUplusV - pv * m_sinU) / m_sinV;
}

//------------------------------------------------------------------------------------------------------------------------------------------
  
float MicroBooNETransformationCalculator::PWPUtoPV(const float pw, const float pu) const
{
    return (pw * m_sinUplusV - pu * m_sinV) / m_sinU;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::PYPZtoPU(const float py, const float pz) const
{
    return pz * m_cosU - py * m_sinU;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::PYPZtoPV(const float py, const float pz) const
{
    return pz * m_cosV + py * m_sinV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNETransformationCalculator::GetSigmaUVW() const
{
    return m_sigmaUVW;
}

} // namespace lar_pandora
