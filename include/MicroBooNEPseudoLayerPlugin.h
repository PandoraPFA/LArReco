/**
 *  @file   LArPandora/MicroBooNEPseudoLayerPlugin.h
 * 
 *  @brief  Header file for the MicroBooNE pseudo layer plugin class.
 * 
 *  $Log: $
 */
#ifndef MICRO_BOONE_PSEUDO_LAYER_PLUGIN_H
#define MICRO_BOONE_PSEUDO_LAYER_PLUGIN_H 1

#include "Plugins/PseudoLayerPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNEPseudoLayerPlugin class
 */
class MicroBooNEPseudoLayerPlugin : public pandora::PseudoLayerPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    MicroBooNEPseudoLayerPlugin();

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

    float           m_zPitch;       ///< The z pitch
    float           m_zOffset;      ///< The z offset
    unsigned int    m_zerothLayer;  ///< The zeroth layer
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline unsigned int MicroBooNEPseudoLayerPlugin::GetPseudoLayerAtIp() const
{
    return 0;
}

} // namespace lar

#endif // #ifndef MICRO_BOONE_PSEUDO_LAYER_PLUGIN_H
