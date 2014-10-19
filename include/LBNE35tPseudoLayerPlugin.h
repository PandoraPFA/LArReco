/**
 *  @file   larpandora/LArPandoraInterface/LBNE35tPseudoLayerPlugin.h
 *
 *  @brief  Header file for the LBNE35t pseudo layer plugin class.
 *
 *  $Log: $
 */
#ifndef LBNE_35T_PSEUDO_LAYER_PLUGIN_H
#define LBNE_35T_PSEUDO_LAYER_PLUGIN_H 1

#include "LArPandoraPseudoLayerPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  LBNE35tPseudoLayerPlugin class
 */
class LBNE35tPseudoLayerPlugin : public LArPandoraPseudoLayerPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    LBNE35tPseudoLayerPlugin();
};

} // namespace lar

#endif // #ifndef LBNE_35T_PSEUDO_LAYER_PLUGIN_H
