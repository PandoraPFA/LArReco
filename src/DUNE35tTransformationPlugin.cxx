/**
 *  @file   larpandora/LArPandoraInterface/DUNE35tTransformationPlugin.cxx
 *
 *  @brief  Implementation of the DUNE35t transformation plugin class.
 *
 *  $Log: $
 */

#include "DUNE35tTransformationPlugin.h"

#include <cmath>

namespace lar_pandora
{

DUNE35tTransformationPlugin::DUNE35tTransformationPlugin(const bool isForward) :
    lar_content::LArRotationalTransformationPlugin( (isForward ? M_PI * (45.707 / 180.) : -M_PI * (45.707 / 180.)),
        (isForward ? M_PI * (44.275 / 180.) : -M_PI * (44.275 / 180.)), 1.33)
{
}

} // namespace lar_pandora
