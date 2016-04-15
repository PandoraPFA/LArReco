/**
 *  @file   dunetpc/DUNEPandora/ProtoDUNETransformationPlugin.cxx
 *
 *  @brief  Implementation of the ProtoDUNE transformation plugin class.
 *
 *  $Log: $
 */

#include "ProtoDUNETransformationPlugin.h"

#include <cmath>

namespace lar_pandora
{

ProtoDUNETransformationPlugin::ProtoDUNETransformationPlugin(const bool isForward) :
    lar_content::LArRotationalTransformationPlugin( (isForward ? M_PI * (35.71 / 180.) : -M_PI * (35.71 / 180.)),
        (isForward ? M_PI * (35.71 / 180.) : -M_PI * (35.71 / 180.)), 1.33)
{
}

} // namespace lar_pandora
