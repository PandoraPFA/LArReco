/**
 *  @file   larpandora/LArPandoraInterface/MicroBooNETransformationPlugin.cxx
 *
 *  @brief  Implementation of the MicroBooNE transformation plugin class.
 *
 *  $Log: $
 */

#include "MicroBooNETransformationPlugin.h"

#include <cmath>

namespace lar_pandora
{

MicroBooNETransformationPlugin::MicroBooNETransformationPlugin() :
    LArPandoraTransformationPlugin(M_PI / 3.f, M_PI / 3.f, 1.f, 0.3f)
{
}

} // namespace lar_pandora
