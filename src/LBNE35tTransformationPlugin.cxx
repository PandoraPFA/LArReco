/**
 *  @file   larpandora/LArPandoraInterface/LBNE35tTransformationPlugin.cxx
 *
 *  @brief  Implementation of the LBNE35t transformation plugin class.
 *
 *  $Log: $
 */

#include "LBNE35tTransformationPlugin.h"

#include <cmath>

namespace lar_pandora
{

LBNE35tTransformationPlugin::LBNE35tTransformationPlugin(const bool isForward) :
    LArPandoraTransformationPlugin( (isForward ? M_PI * (45.707f / 180.f) : -M_PI * (44.275f/ 180.f)),
        (isForward ? M_PI * (44.275f/ 180.f) : -M_PI * (45.707f / 180.f)), 196.f, 1.33f, 0.45f)
{
}

} // namespace lar_pandora
