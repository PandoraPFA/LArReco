/**
 *  @file   larpandora/LArPandoraInterface/MicroBooNELegacyTransformationPlugin.h
 *
 *  @brief  Header file for the MicroBooNE legacy transformation plugin class.
 *
 *  $Log: $
 */
#ifndef MICRO_BOONE_LEGACY_TRANSFORMATION_PLUGIN_H
#define MICRO_BOONE_LEGACY_TRANSFORMATION_PLUGIN_H 1

#include "LArPlugins/LArTransformationPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNELegacyTransformationPlugin class
 */
class MicroBooNELegacyTransformationPlugin : public lar_content::LArTransformationPlugin
{
public:

    /**
     *  @brief  Default Constructor
     */
    MicroBooNELegacyTransformationPlugin();

    /**
     *  @brief  Constructor
     *
     *  @param  thetaU  the angle from the W axis to the U axis (radians)
     *  @param  thetaV  the angle from the W axis to the V axis (radians)
     *  @param  H  the height of the drift volume (cm)
     *  @param  sigmaUVW  nominal spatial resolution for U, V and W (cm)
     *  @param  wireZPitch  the wire pitch (cm)
     */
    MicroBooNELegacyTransformationPlugin(const float thetaU, const float thetaV, const float H, const float sigmaUVW, const float wireZPitch);

    /**
     *  @brief  Destructor
     */
    virtual ~MicroBooNELegacyTransformationPlugin();

    virtual float UVtoW(const float u, const float v) const;

    virtual float VWtoU(const float v, const float w) const;

    virtual float WUtoV(const float w, const float u) const;

    virtual float UVtoY(const float u, const float v) const;

    virtual float UVtoZ(const float u, const float v) const;

    virtual float YZtoU(const float y, const float z) const;

    virtual float YZtoV(const float y, const float z) const;

    virtual float PUPVtoPW(const float pu, const float pv) const;

    virtual float PVPWtoPU(const float pv, const float pw) const;

    virtual float PWPUtoPV(const float pw, const float pu) const;

    virtual float PUPVtoPY(const float pu, const float pv) const;

    virtual float PUPVtoPZ(const float pu, const float pv) const;

    virtual float PYPZtoPU(const float py, const float pz) const;

    virtual float PYPZtoPV(const float py, const float pz) const;

    virtual float GetSigmaUVW() const;

    virtual void GetMinChiSquaredYZ(const float u, const float v, const float w, const float sigmaU, const float sigmaV, const float sigmaW,
        float &y, float &z, float &chiSquared) const;

    virtual void GetProjectedYZ(const PositionAndType &hitPositionAndType, const PositionAndType &fitPositionAndType1,
        const PositionAndType &fitPositionAndType2, const float sigmaHit, const float sigmaFit, float &y, float &z, float &chiSquared) const;

private:

    const float     m_thetaU;          ///< inclination of U wires (radians)
    const float     m_thetaV;          ///< inclination of V wires (radians)
    const float     m_H;               ///< height (cm)
    const float     m_sigmaUVW;        ///< resolution (cm), for calculation of chi2
    const float     m_sinUminusV;      ///< sin(thetaU-thetaV)
    const float     m_sinUplusV;       ///< sin(thetaU+thetaV)
    const float     m_sinU;            ///< sin(thetaU)
    const float     m_sinV;            ///< sin(thetaV)
    const float     m_cosU;            ///< cos(thetaU)
    const float     m_cosV;            ///< cos(thetaV)
};

} // namespace lar_pandora

#endif // #ifndef MICRO_BOONE_LEGACY_TRANSFORMATION_PLUGIN_H
