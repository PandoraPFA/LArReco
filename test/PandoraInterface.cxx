/**
 *  @file   LArRecoMP/test/PandoraInterface.cc
 *
 *  @brief  Implementation of the lar reco mp application
 *
 *  $Log: $
 */

#include "Api/PandoraApi.h"

#include "larpandoracontent/LArContent/LArContent.h"

#include "larpandoracontent/LArContent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArContent/LArStitching/MultiPandoraApi.h"

#include "PandoraInterface.h"

#include "MicroBooNEPseudoLayerPlugin.h"
#include "MicroBooNETransformationPlugin.h"
#include "DUNE35tPseudoLayerPlugin.h"
#include "DUNE35tTransformationPlugin.h"
#include "DUNE4APAPseudoLayerPlugin.h"
#include "DUNE4APATransformationPlugin.h"
#include "ProtoDUNEPseudoLayerPlugin.h"
#include "ProtoDUNETransformationPlugin.h"

#ifdef MONITORING
#include "TApplication.h"
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/time.h>

using namespace pandora;
using namespace lar_reco_mp;

int main(int argc, char *argv[])
{
    try
    {
        Parameters parameters;

        if (!ParseCommandLine(argc, argv, parameters))
            return 1;
#ifdef MONITORING
    TApplication *pTApplication = new TApplication("MyTest", &argc, argv);
    pTApplication->SetReturnFromRun(kTRUE);
#endif
        const Pandora *const pPrimaryPandora = CreateNewPandora();
        ConfigurePrimaryPandoraInstance(parameters, pPrimaryPandora);

        MultiPandoraApi::AddPrimaryPandoraInstance(pPrimaryPandora);

        if ("uboone" == parameters.m_whichDetector)
        {
            ConfigureMicroBooNEPandoraInstance(parameters, pPrimaryPandora);
        }
        else if ("dune35t" == parameters.m_whichDetector)
        {
            ConfigureDune35tPandoraInstances(parameters, pPrimaryPandora);
        }
        else if ("dune4apa" == parameters.m_whichDetector)
        {
            ConfigureDune4APAPandoraInstances(parameters, pPrimaryPandora);
        }
        else if ("protodune" == parameters.m_whichDetector)
        {
            ConfigureProtoDunePandoraInstances(parameters, pPrimaryPandora);
        }
        else
        {
            std::cout << " Not a valid detector (options: uboone, dune35t, dune4apa, protodune)" << std::endl << " Exiting" << std::endl;
            return 1;
        }

        ProcessEvents(parameters, pPrimaryPandora);
        MultiPandoraApi::DeletePandoraInstances(pPrimaryPandora);
    }
    catch (StatusCodeException &statusCodeException)
    {
        std::cerr << "Pandora Exception caught: " << statusCodeException.ToString() << std::endl;
        return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------

namespace lar_reco_mp
{

bool ParseCommandLine(int argc, char *argv[], Parameters &parameters)
{
    int c(0);

    while ((c = getopt(argc, argv, "i:d:n:t::N::h")) != -1)
    {
        switch (c)
        {
        case 'i':
            parameters.m_pandoraSettingsFile = optarg;
            break;
        case 'd':
            parameters.m_whichDetector = optarg;
            break;
        case 'n':
            parameters.m_nEventsToProcess = atoi(optarg);
            break;
        case 't':
            parameters.m_shouldDisplayEventTime = true;
            break;
        case 'N':
            parameters.m_shouldDisplayEventNumber = true;
            break;
        case 'h':
        default:
            std::cout << std::endl << "./bin/PandoraInterface " << std::endl
                      << "    -i PandoraSettings.xml  (mandatory)" << std::endl
                      << "    -d WhichDetector        (mandatory)" << std::endl
                      << "    -n NEventsToProcess     (optional)" << std::endl
                      << "    -N                      (optional, display event numbers)" << std::endl
                      << "    -t                      (optional, display event times)" << std::endl << std::endl;
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

const Pandora *CreateNewPandora()
{
    const Pandora *const pPandora = new Pandora();
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::RegisterAlgorithms(*pPandora));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::RegisterBasicPlugins(*pPandora));

    return pPandora;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ConfigurePrimaryPandoraInstance(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPrimaryPandora, parameters.m_pandoraSettingsFile));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ConfigureMicroBooNEPandoraInstance(const Parameters &/*parameters*/, const Pandora *const pPrimaryPandora)
{
    std::cout << " Loading plugins for MicroBooNE detector" << std::endl;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPrimaryPandora, new lar_pandora::MicroBooNEPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPrimaryPandora, new lar_pandora::MicroBooNETransformationPlugin));
    MultiPandoraApi::SetVolumeInfo(pPrimaryPandora, new VolumeInfo(0, "uboone", CartesianVector(0.f, 0.f, 0.f), true));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ConfigureDune35tPandoraInstances(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    std::cout << " Loading plugins for DUNE35t detector" << std::endl;

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPrimaryPandora, new lar_pandora::DUNE35tPseudoLayerPlugin));
    const size_t fileNameEnd(parameters.m_pandoraSettingsFile.empty() ? 0 : parameters.m_pandoraSettingsFile.length() - std::string(".xml").length());
    int volumeIdNumber(0);

    const Pandora *const pPandoraShort = CreateNewPandora();
    MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandoraShort);
    MultiPandoraApi::SetVolumeInfo(pPandoraShort, new VolumeInfo(volumeIdNumber++, "dune35t_short", CartesianVector(0.f, 0.f, 0.f), true));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandoraShort, new lar_pandora::DUNE35tPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandoraShort, new lar_pandora::DUNE35tTransformationPlugin(true)));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandoraShort, std::string(parameters.m_pandoraSettingsFile).insert(fileNameEnd, "_short")));

    const Pandora *const pPandoraLong = CreateNewPandora();
    MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandoraLong);
    MultiPandoraApi::SetVolumeInfo(pPandoraLong, new VolumeInfo(volumeIdNumber++, "dune35t_long", CartesianVector(0.f, 0.f, 0.f), false));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandoraLong, new lar_pandora::DUNE35tPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandoraLong, new lar_pandora::DUNE35tTransformationPlugin(false)));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandoraLong, std::string(parameters.m_pandoraSettingsFile).insert(fileNameEnd, "_long")));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ConfigureDune4APAPandoraInstances(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    std::cout << " Loading plugins for DUNE4APA detector" << std::endl;

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPrimaryPandora, new lar_pandora::DUNE4APAPseudoLayerPlugin));
    const size_t fileNameEnd(parameters.m_pandoraSettingsFile.empty() ? 0 : parameters.m_pandoraSettingsFile.length() - std::string(".xml").length());
    int volumeIdNumber(0);

    const Pandora *const pPandoraLeft = CreateNewPandora();
    MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandoraLeft);
    MultiPandoraApi::SetVolumeInfo(pPandoraLeft, new VolumeInfo(volumeIdNumber++, "dune4apa_left", CartesianVector(0.f, 0.f, 0.f), true));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandoraLeft, new lar_pandora::DUNE4APAPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandoraLeft, new lar_pandora::DUNE4APATransformationPlugin(true)));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandoraLeft, std::string(parameters.m_pandoraSettingsFile).insert(fileNameEnd, "_left")));

    const Pandora *const pPandoraRight = CreateNewPandora();
    MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandoraRight);
    MultiPandoraApi::SetVolumeInfo(pPandoraRight, new VolumeInfo(volumeIdNumber++, "dune4apa_right", CartesianVector(0.f, 0.f, 0.f), false));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandoraRight, new lar_pandora::DUNE4APAPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandoraRight, new lar_pandora::DUNE4APATransformationPlugin(false)));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandoraRight, std::string(parameters.m_pandoraSettingsFile).insert(fileNameEnd, "_right")));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ConfigureProtoDunePandoraInstances(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    std::cout << " Loading plugins for ProtoDUNE detector" << std::endl;

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPrimaryPandora, new lar_pandora::ProtoDUNEPseudoLayerPlugin));
    const size_t fileNameEnd(parameters.m_pandoraSettingsFile.empty() ? 0 : parameters.m_pandoraSettingsFile.length() - std::string(".xml").length());
    int volumeIdNumber(0);

    const Pandora *const pPandoraLeft = CreateNewPandora();
    MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandoraLeft);
    MultiPandoraApi::SetVolumeInfo(pPandoraLeft, new VolumeInfo(volumeIdNumber++, "protodune_left", CartesianVector(0.f, 0.f, 0.f), true));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandoraLeft, new lar_pandora::ProtoDUNEPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandoraLeft, new lar_pandora::ProtoDUNETransformationPlugin(true)));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandoraLeft, std::string(parameters.m_pandoraSettingsFile).insert(fileNameEnd, "_left")));

    const Pandora *const pPandoraRight = CreateNewPandora();
    MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandoraRight);
    MultiPandoraApi::SetVolumeInfo(pPandoraRight, new VolumeInfo(volumeIdNumber++, "protodune_right", CartesianVector(0.f, 0.f, 0.f), false));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandoraRight, new lar_pandora::ProtoDUNEPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandoraRight, new lar_pandora::ProtoDUNETransformationPlugin(false)));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandoraRight, std::string(parameters.m_pandoraSettingsFile).insert(fileNameEnd, "_right")));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessEvents(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    int nEvents(0);
    const PandoraInstanceList &pandoraInstanceList(MultiPandoraApi::GetDaughterPandoraInstanceList(pPrimaryPandora));

    while ((nEvents++ < parameters.m_nEventsToProcess) || (0 > parameters.m_nEventsToProcess))
    {
        struct timeval startTime, endTime;

        if (parameters.m_shouldDisplayEventNumber)
            std::cout << std::endl << "   PROCESSING EVENT: " << (nEvents - 1) << std::endl << std::endl;

        if (parameters.m_shouldDisplayEventTime)
            (void) gettimeofday(&startTime, NULL);

        for (const Pandora *const pPandora : pandoraInstanceList)
        {
            PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(*pPandora));
            PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, SetParticleX0Values(pPandora));        
        }

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(*pPrimaryPandora));

        if (parameters.m_shouldDisplayEventTime)
        {
            (void) gettimeofday(&endTime, NULL);
            std::cout << "Event time " << (endTime.tv_sec + (endTime.tv_usec / 1.e6) - startTime.tv_sec - (startTime.tv_usec / 1.e6)) << std::endl;
        }

        for (const Pandora *const pPandora : pandoraInstanceList)
        {
            PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*pPandora));
            MultiPandoraApi::ClearParticleX0Map(pPandora);
        }

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*pPrimaryPandora));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode SetParticleX0Values(const Pandora *const pPandora)
{
    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::GetCurrentPfoList(*pPandora, pPfoList));

    PfoList connectedPfoList;
    lar_content::LArPfoHelper::GetAllConnectedPfos(*pPfoList, connectedPfoList);

    for (const ParticleFlowObject *const pPfo : connectedPfoList)
        MultiPandoraApi::SetParticleX0(pPandora, pPfo, 0.f);

    return STATUS_CODE_SUCCESS;
}

} // namespace lar_reco_mp
