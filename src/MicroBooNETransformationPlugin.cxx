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
    lar_content::LArRotationalTransformationPlugin(M_PI / 3., M_PI / 3., 1.)
{
}

} // namespace lar_pandora
