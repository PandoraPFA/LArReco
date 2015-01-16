/**
 *  @file   LArContent/LArPandoraPseudoLayerPlugin.cxx
 *
 *  @brief  Implementation of the LArPandora pseudo layer Plugin class.
 *
 *  $Log: $
 */

#include "Helpers/XmlHelper.h"

#include "Pandora/PandoraInputTypes.h"

#include "LArPandoraPseudoLayerPlugin.h"

#include <limits>

namespace lar_pandora
{

LArPandoraPseudoLayerPlugin::LArPandoraPseudoLayerPlugin(const float zPitch) :
    m_zPitch(zPitch),
    m_zOffset(0.01f),
    m_zerothLayer(5000)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

unsigned int LArPandoraPseudoLayerPlugin::GetPseudoLayer(const float zCoordinate) const
{
    const float zLayer((zCoordinate + m_zOffset) / m_zPitch + static_cast<float>(m_zerothLayer));

    if (zLayer < std::numeric_limits<float>::epsilon())
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

    return static_cast<unsigned int>(zLayer);
}

//------------------------------------------------------------------------------------------------------------------------------------------

unsigned int LArPandoraPseudoLayerPlugin::GetPseudoLayer(const pandora::CartesianVector &positionVector) const
{
    return this->GetPseudoLayer(positionVector.GetZ());
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LArPandoraPseudoLayerPlugin::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ZOffset", m_zOffset));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ZerothLayer", m_zerothLayer));

    return pandora::STATUS_CODE_SUCCESS;
}

} // namespace lar_pandora
