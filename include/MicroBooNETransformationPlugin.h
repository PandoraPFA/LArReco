/**
 *  @file   larpandora/LArPandoraInterface/MicroBooNETransformationPlugin.h
 *
 *  @brief  Header file for the MicroBooNE transformation plugin class.
 *
 *  $Log: $
 */
#ifndef MICRO_BOONE_TRANSFORMATION_PLUGIN_H
#define MICRO_BOONE_TRANSFORMATION_PLUGIN_H 1

#include "LArPlugins/LArRotationalTransformationPlugin.h"

namespace lar_pandora
{

/**
 *  @brief  MicroBooNETransformationPlugin class
 */
class MicroBooNETransformationPlugin : public lar_content::LArRotationalTransformationPlugin
{
public:
    /**
     *  @brief  Default constructor
     */
    MicroBooNETransformationPlugin();
};

} // namespace lar_pandora

#endif // #ifndef MICRO_BOONE_TRANSFORMATION_PLUGIN_H
