/**
 *  @file   larpandora/LArPandoraInterface/LBNE4APATransformationPlugin.cxx
 *
 *  @brief  Implementation of the LBNE4APA transformation plugin class.
 *
 *  $Log: $
 */

#include "LBNE4APATransformationPlugin.h"

#include <cmath>

namespace lar_pandora
{

LBNE4APATransformationPlugin::LBNE4APATransformationPlugin(const bool isForward) :
    LArPandoraTransformationPlugin( (isForward ? M_PI * (36.f / 180.f) : -M_PI * (36.f/ 180.f)),
        (isForward ? M_PI * (36.f/ 180.f) : -M_PI * (36.f / 180.f)), 1.33f, 0.45f)
{
}

} // namespace lar_pandora
