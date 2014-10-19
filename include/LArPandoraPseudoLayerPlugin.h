/**
 *  @file   larpandora/LArPandoraInterface/LarPandoraPseudoLayerPlugin.h
 *
 *  @brief  Header file for the LarPandora pseudo layer plugin class.
 *
 *  $Log: $
 */
#ifndef LAR_PANDORA_PSEUDO_LAYER_PLUGIN_H
#define LAR_PANDORA_PSEUDO_LAYER_PLUGIN_H 1

#include "Plugins/PseudoLayerPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  LarPandoraPseudoLayerPlugin class
 */
class LArPandoraPseudoLayerPlugin : public pandora::PseudoLayerPlugin
{
public:
    /**
     *  @brief  Constructor
     *
     *  @param  zPitch the wire pitch
     */
    LArPandoraPseudoLayerPlugin(const float zPitch);

    /**
     *  @brief  Get pseudolayer corresponding to a specified z coordinate
     *
     *  @param  zCoordinate the z coordinate
     *
     *  @return the pseudolayer
     */
    unsigned int GetPseudoLayer(const float zCoordinate) const;

private:
    unsigned int GetPseudoLayer(const pandora::CartesianVector &positionVector) const;
    unsigned int GetPseudoLayerAtIp() const;
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    const float     m_zPitch;       ///< The z pitch
    float           m_zOffset;      ///< The z offset
    unsigned int    m_zerothLayer;  ///< The zeroth layer
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline unsigned int LArPandoraPseudoLayerPlugin::GetPseudoLayerAtIp() const
{
    return 0;
}

} // namespace lar

#endif // #ifndef LAR_PANDORA_PSEUDO_LAYER_PLUGIN_H
