/**
 *  @file   LArPandora/MicroBooNETransformationCalculator.h
 * 
 *  @brief  Header file for the MicroBooNE transformation calculator class.
 * 
 *  $Log: $
 */
#ifndef MICRO_BOONE_TRANSFORMATION_CALCULATOR_H
#define MICRO_BOONE_TRANSFORMATION_CALCULATOR_H 1

#include "LArCalculators/LArTransformationCalculator.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNETransformationCalculator class
 */
class MicroBooNETransformationCalculator : public lar::LArTransformationCalculator
{
public:
    /** 
     *  @brief  Transform from (U,V) to W position 
     *
     *  @param  U the U position
     *  @param  V the V position
     */
     float UVtoW(const float u, const float v) const;

    /** 
     *  @brief  Transform from (V,W) to U position 
     *
     *  @param  V the V position
     *  @param  W the W position
     */
     float VWtoU(const float v, const float w) const;

    /** 
     *  @brief  Transform from (W,U) to V position 
     *
     *  @param  W the W position
     *  @param  U the U position
     */
     float WUtoV(const float w, const float u) const;

    /** 
     *  @brief  Transform from (U,V) to Y position
     *          Y * sin(thetaU+thetaV) = U * cos(thetaV) + V * cos(thetaU) - H/2 * sin(thetaU-thetaV)
     *
     *  @param  U the U position
     *  @param  V the V position
     */
    float UVtoY(const float u, const float v) const;

    /** 
     *  @brief  Transform from (U,V) to Z position 
     *          Z * sin(thetaU+thetaV) = U * sin(thetaV) + V * sin(thetaU) - H * sin(thetaU) * sin(thetaV)
     *
     *  @param  U the U position
     *  @param  V the V position
     */
     float UVtoZ(const float u, const float v) const;

    /** 
     *  @brief  Transform from (Y,Z) to U position
     *          U = Z * cos(thetaU) + ( Y + H/2 ) * sin(thetaU)
     * 
     *  @param  Y the Y position
     *  @param  Z the Z position
     */
     float YZtoU(const float y, const float z) const;

    /** 
     *  @brief  Transform from (Y,Z) to V position
     *          V = Z * cos(thetaV) - ( Y - H/2 ) * sin(thetaV)
     * 
     *  @param  Y the Y position
     *  @param  Z the Z position
     */
     float YZtoV(const float y, const float z) const;

    /** 
     *  @brief  Transform from (pU,pV) to pW direction
     *
     *  @param  pU the pU direction
     *  @param  pV the pV direction
     */
     float PUPVtoPW(const float pu, const float pv) const;

    /** 
     *  @brief  Transform from (pV,pW) to pU direction 
     *
     *  @param  pV the pV direction
     *  @param  pW the pW direction
     */
     float PVPWtoPU(const float pv, const float pw) const;

    /** 
     *  @brief  Transform from (pW,pU) to pV direction 
     *
     *  @param  pW the pW direction
     *  @param  pU the pU direction
     */
     float PWPUtoPV(const float pw, const float pu) const;

    /** 
     *  @brief  Transform from (pY,pZ) to pU direction
     *          pU = pZ * cos(thetaU) + pY * sin(thetaU)  
     * 
     *  @param  pU the U component
     *  @param  pV the V component
     */
    float PYPZtoPU(const float py, const float pz) const;

    /** 
     *  @brief  Transform from (pY,pZ) to pV direction
     *          pV = pZ * cos(thetaV) - pY * sin(thetaV)
     * 
     *  @param  pU the U component
     *  @param  pV the V component
     */
    float PYPZtoPV(const float py, const float pz) const;

    /** 
     *  @brief  Get resolution, in cm, for calculation of chi2
     * 
     *  @return resolution, in cm, for calculation of chi2
     */
    float GetSigmaUVW() const;

private:
    static const float  m_thetaU;            // inclination of U wires (radians)
    static const float  m_thetaV;            // inclination of V wires (radians)
    static const float  m_sinUminusV;        // sin(thetaU-thetaV)
    static const float  m_sinUplusV;         // sin(thetaU+thetaV)
    static const float  m_sinU;              // sin(thetaU)
    static const float  m_sinV;              // sin(thetaV)
    static const float  m_cosU;              // cos(thetaU)
    static const float  m_cosV;              // cos(thetaV)
    static const float  m_H;                 // height (cm)
    static const float  m_sigmaUVW;          // resolution (cm), for calculation of chi2
};

} // namespace lar_pandora

#endif // #ifndef MICRO_BOONE_TRANSFORMATION_CALCULATOR_H
