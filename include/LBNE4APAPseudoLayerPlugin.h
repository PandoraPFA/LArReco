/**
 *  @file   larpandora/LArPandoraInterface/LBNE4APAPseudoLayerPlugin.h
 *
 *  @brief  Header file for the LBNE4APA pseudo layer plugin class.
 *
 *  $Log: $
 */
#ifndef LBNE_4APA_PSEUDO_LAYER_PLUGIN_H
#define LBNE_4APA_PSEUDO_LAYER_PLUGIN_H 1

#include "LArPlugins/LArPseudoLayerPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  LBNE4APAPseudoLayerPlugin class
 */
class LBNE4APAPseudoLayerPlugin : public lar_content::LArPseudoLayerPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    LBNE4APAPseudoLayerPlugin();
};

} // namespace lar

#endif // #ifndef LBNE_4APA_PSEUDO_LAYER_PLUGIN_H
