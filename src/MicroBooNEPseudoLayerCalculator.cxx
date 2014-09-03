/**
 *  @file   LArContent/MicroBooNEPseudoLayerCalculator.cxx
 * 
 *  @brief  Implementation of the MicroBooNE pseudo layer calculator class.
 * 
 *  $Log: $
 */

#include "Helpers/XmlHelper.h"

#include "Pandora/PandoraInputTypes.h"

#include "MicroBooNEPseudoLayerCalculator.h"

#include <limits>

namespace lar_pandora
{

MicroBooNEPseudoLayerCalculator::MicroBooNEPseudoLayerCalculator() :
    m_zPitch(0.3f),
    m_zOffset(0.01f),
    m_zerothLayer(1000)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

float MicroBooNEPseudoLayerCalculator::GetZCoordinate(const unsigned int pseudoLayer) const
{
    const float zCoordinate((static_cast<float>(pseudoLayer) - static_cast<float>(m_zerothLayer)) * m_zPitch);

    return zCoordinate;
}

//------------------------------------------------------------------------------------------------------------------------------------------

unsigned int MicroBooNEPseudoLayerCalculator::GetPseudoLayer(const float zCoordinate) const
{
    const float zLayer((zCoordinate + m_zOffset) / m_zPitch + static_cast<float>(m_zerothLayer));

    if (zLayer < std::numeric_limits<float>::epsilon())
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

    return static_cast<unsigned int>(zLayer);
}

//------------------------------------------------------------------------------------------------------------------------------------------

unsigned int MicroBooNEPseudoLayerCalculator::GetPseudoLayer(const pandora::CartesianVector &positionVector) const
{
    return this->GetPseudoLayer(positionVector.GetZ());
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode MicroBooNEPseudoLayerCalculator::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ZPitch", m_zPitch));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ZOffset", m_zOffset));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ZerothLayer", m_zerothLayer));

    return pandora::STATUS_CODE_SUCCESS;
}

} // namespace lar_pandora
