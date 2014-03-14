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
     *  @brief  Get z coordinate corresponding to a specified pseudolayer
     * 
     *  @param  pseudolayer the pseudolayer
     *
     *  @return the z coordinate
     */
    float GetZCoordinate(const pandora::PseudoLayer pseudoLayer) const;

    /**
     *  @brief  Get pseudolayer corresponding to a specified z coordinate
     *
     *  @param  zCoordinate the z coordinate
     *
     *  @return the pseudolayer
     */
    pandora::PseudoLayer GetPseudoLayer(const float zCoordinate) const;

    /**
     *  @brief  Get the z pitch
     *
     *  @return the z pitch
     */
    float GetZPitch() const;

private:
    void Initialize(const pandora::GeometryHelper *const pGeometryHelper);
    pandora::PseudoLayer GetPseudoLayer(const pandora::CartesianVector &positionVector) const;
    pandora::PseudoLayer GetPseudoLayerAtIp() const;

    static const float        Z_PITCH;      ///< The z pitch
    static const float        Z_OFFSET;     ///< The z offset
    static const unsigned int ZERO_LAYER;   ///< The zeroth layer
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline float MicroBooNEPseudoLayerCalculator::GetZPitch() const
{
    return Z_PITCH;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::PseudoLayer MicroBooNEPseudoLayerCalculator::GetPseudoLayerAtIp() const
{
    return 0;
}

} // namespace lar

#endif // #ifndef MICRO_BOONE_PSEUDO_LAYER_CALCULATOR_H
