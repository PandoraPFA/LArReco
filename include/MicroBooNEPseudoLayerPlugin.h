/**
 *  @file   larpandora/LArPandoraInterface/MicroBooNEPseudoLayerPlugin.h
 *
 *  @brief  Header file for the MicroBooNE pseudo layer plugin class.
 *
 *  $Log: $
 */
#ifndef MICRO_BOONE_PSEUDO_LAYER_PLUGIN_H
#define MICRO_BOONE_PSEUDO_LAYER_PLUGIN_H 1

#include "LArPandoraPseudoLayerPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNEPseudoLayerPlugin class
 */
class MicroBooNEPseudoLayerPlugin : public LArPandoraPseudoLayerPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    MicroBooNEPseudoLayerPlugin();
};

} // namespace lar

#endif // #ifndef MICRO_BOONE_PSEUDO_LAYER_PLUGIN_H
