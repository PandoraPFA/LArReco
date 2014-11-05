/**
 *  @file   larpandora/LArPandoraInterface/LBNE35tTransformationPlugin.h
 *
 *  @brief  Header file for the LBNE35t transformation plugin class.
 *
 *  $Log: $
 */
#ifndef LBNE_35T_TRANSFORMATION_PLUGIN_H
#define LBNE_35T_TRANSFORMATION_PLUGIN_H 1

#include "LArPandoraTransformationPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  LBNE35tTransformationPlugin class
 */
class LBNE35tTransformationPlugin : public LArPandoraTransformationPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    LBNE35tTransformationPlugin(const bool isForward);
};

} // namespace lar_pandora

#endif // #ifndef LBNE_35T_TRANSFORMATION_PLUGIN_H
