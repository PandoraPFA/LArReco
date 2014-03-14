/**
 *  @file   LArContent/MicroBooNEPseudoLayerCalculator.cxx
 * 
 *  @brief  Implementation of the MicroBooNE pseudo layer calculator class.
 * 
 *  $Log: $
 */

#include "MicroBooNEPseudoLayerCalculator.h"

namespace lar_pandora
{

// TODO read in from config
const float MicroBooNEPseudoLayerCalculator::Z_PITCH = 0.3f;
const float MicroBooNEPseudoLayerCalculator::Z_OFFSET = 0.01f;
const unsigned int MicroBooNEPseudoLayerCalculator::ZERO_LAYER = 1000;

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNEPseudoLayerCalculator::GetZCoordinate(const pandora::PseudoLayer pseudoLayer) const
{
    const float zCoordinate((static_cast<float>(pseudoLayer) - static_cast<float>(ZERO_LAYER)) * Z_PITCH);

    return zCoordinate;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::PseudoLayer MicroBooNEPseudoLayerCalculator::GetPseudoLayer(const float zCoordinate) const
{
    const float zLayer((zCoordinate + Z_OFFSET) / Z_PITCH + static_cast<float>(ZERO_LAYER));

    if (zLayer < std::numeric_limits<float>::epsilon())
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

    const pandora::PseudoLayer pseudoLayer(static_cast<unsigned int>(zLayer));

    return pseudoLayer;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void MicroBooNEPseudoLayerCalculator::Initialize(const pandora::GeometryHelper *const pGeometryHelper)
{
    // No initialization required
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::PseudoLayer MicroBooNEPseudoLayerCalculator::GetPseudoLayer(const pandora::CartesianVector &positionVector) const
{
    return MicroBooNEPseudoLayerCalculator::GetPseudoLayer(positionVector.GetZ());
}

} // namespace lar_pandora
