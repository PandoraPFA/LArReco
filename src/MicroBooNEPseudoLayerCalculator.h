/**
 *  @file   LArPandora/MicroBooNEPseudoLayerCalculator.h
 * 
 *  @brief  Header file for the MicroBooNE pseudo layer calculator class.
 * 
 *  $Log: $
 */
#ifndef MICRO_BOONE_PSEUDO_LAYER_CALCULATOR_H
#define MICRO_BOONE_PSEUDO_LAYER_CALCULATOR_H 1

#include "LArCalculators/LArPseudoLayerCalculator.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNEPseudoLayerCalculator class
 */
class MicroBooNEPseudoLayerCalculator : public lar::LArPseudoLayerCalculator
{
public:
    /**
     *  @brief  Default constructor
     */
    MicroBooNEPseudoLayerCalculator();

    /**
     *  @brief  Get z coordinate corresponding to a specified pseudolayer
     * 
     *  @param  pseudolayer the pseudolayer
     *
     *  @return the z coordinate
     */
    float GetZCoordinate(const unsigned int pseudoLayer) const;

    /**
     *  @brief  Get pseudolayer corresponding to a specified z coordinate
     *
     *  @param  zCoordinate the z coordinate
     *
     *  @return the pseudolayer
     */
    unsigned int GetPseudoLayer(const float zCoordinate) const;

    /**
     *  @brief  Get the z pitch
     *
     *  @return the z pitch
     */
    float GetZPitch() const;

private:
    unsigned int GetPseudoLayer(const pandora::CartesianVector &positionVector) const;
    unsigned int GetPseudoLayerAtIp() const;
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    float           m_zPitch;       ///< The z pitch
    float           m_zOffset;      ///< The z offset
    unsigned int    m_zerothLayer;  ///< The zeroth layer
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline float MicroBooNEPseudoLayerCalculator::GetZPitch() const
{
    return m_zPitch;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline unsigned int MicroBooNEPseudoLayerCalculator::GetPseudoLayerAtIp() const
{
    return 0;
}

} // namespace lar

#endif // #ifndef MICRO_BOONE_PSEUDO_LAYER_CALCULATOR_H
