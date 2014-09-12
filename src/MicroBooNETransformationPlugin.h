/**
 *  @file   LArPandora/MicroBooNETransformationPlugin.h
 * 
 *  @brief  Header file for the MicroBooNE transformation plugin class.
 * 
 *  $Log: $
 */
#ifndef MICRO_BOONE_TRANSFORMATION_PLUGIN_H
#define MICRO_BOONE_TRANSFORMATION_PLUGIN_H 1

#include "LArPlugins/LArTransformationPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNETransformationPlugin class
 */
class MicroBooNETransformationPlugin : public lar_content::LArTransformationPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    MicroBooNETransformationPlugin();

    /**
     *  @brief  Destructor
     */
    ~MicroBooNETransformationPlugin();

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

    /** 
     *  @brief  Get wire z pitch, in cm
     * 
     *  @return wire z pitch, in cm
     */
    float GetWireZPitch() const;

    /** 
     *  @brief  Get the y, z position that yields the minimum chi squared value with respect to specified u, v and w coordinates
     * 
     *  @param  u the u coordinate
     *  @param  v the v coordinate
     *  @param  w the w coordinate
     *  @param  sigmaU the uncertainty in the u coordinate
     *  @param  sigmaV the uncertainty in the v coordinate
     *  @param  sigmaW the uncertainty in the w coordinate
     *  @param  y to receive the y coordinate
     *  @param  z to receive the z coordinate
     *  @param  chiSquared to receive the chi squared value
     */
    void GetMinChiSquaredYZ(const float u, const float v, const float w, const float sigmaU, const float sigmaV, const float sigmaW,
        float &y, float &z, float &chiSquared) const;

    /** 
     *  @brief  Get the y, z position that corresponds to a projection of two fit positions onto the specific wire associated with a hit
     * 
     *  @param  hitPositionAndType the hit position and hit type
     *  @param  fitPositionAndType1 the first fit position and hit type
     *  @param  fitPositionAndType2 the second fit position and hit type
     *  @param  sigmaHit the uncertainty in the hit coordinate
     *  @param  sigmaFit the uncertainty in the fit coordinates
     *  @param  y to receive the y coordinate
     *  @param  z to receive the z coordinate
     *  @param  chiSquared to receive the chi squared value
     */
    void GetProjectedYZ(const PositionAndType &hitPositionAndType, const PositionAndType &fitPositionAndType1,
        const PositionAndType &fitPositionAndType2, const float sigmaHit, const float sigmaFit, float &y, float &z, float &chiSquared) const;

private:
    const float     m_thetaU;          ///< inclination of U wires (radians)
    const float     m_thetaV;          ///< inclination of V wires (radians)
    const float     m_sinUminusV;      ///< sin(thetaU-thetaV)
    const float     m_sinUplusV;       ///< sin(thetaU+thetaV)
    const float     m_sinU;            ///< sin(thetaU)
    const float     m_sinV;            ///< sin(thetaV)
    const float     m_cosU;            ///< cos(thetaU)
    const float     m_cosV;            ///< cos(thetaV)
    const float     m_H;               ///< height (cm)
    const float     m_sigmaUVW;        ///< resolution (cm), for calculation of chi2
    const float     m_wireZPitch;      ///< wire z pitch (cm)
};

} // namespace lar_pandora

#endif // #ifndef MICRO_BOONE_TRANSFORMATION_PLUGIN_H
