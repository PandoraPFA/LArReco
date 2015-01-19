/**
 *  @file   larpandora/LArPandoraInterface/LBNE4APATransformationPlugin.h
 *
 *  @brief  Header file for the LBNE4APA transformation plugin class.
 *
 *  $Log: $
 */
#ifndef LBNE_4APA_TRANSFORMATION_PLUGIN_H
#define LBNE_4APA_TRANSFORMATION_PLUGIN_H 1

#include "LArPandoraTransformationPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  LBNE4APATransformationPlugin class
 */
class LBNE4APATransformationPlugin : public LArPandoraTransformationPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    LBNE4APATransformationPlugin(const bool isForward);
};

} // namespace lar_pandora

#endif // #ifndef LBNE_4APA_TRANSFORMATION_PLUGIN_H
