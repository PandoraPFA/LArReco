/**
 *  @file   larpandora/LArPandoraInterface/DUNE4APATransformationPlugin.cxx
 *
 *  @brief  Implementation of the DUNE4APA transformation plugin class.
 *
 *  $Log: $
 */

#include "DUNE4APATransformationPlugin.h"

#include <cmath>

namespace lar_pandora
{

DUNE4APATransformationPlugin::DUNE4APATransformationPlugin(const bool isForward) :
    lar_content::LArRotationalTransformationPlugin( (isForward ? M_PI * (36. / 180.) : -M_PI * (36. / 180.)),
        (isForward ? M_PI * (36. / 180.) : -M_PI * (36. / 180.)), 1.33)
{
}

} // namespace lar_pandora
