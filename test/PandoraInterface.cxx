/**
 *  @file   LArRecoMP/test/PandoraInterface.cc
 *
 *  @brief  Implementation of the lar reco mp application
 *
 *  $Log: $
 */

#include "Api/PandoraApi.h"
#include "Helpers/XmlHelper.h"
#include "Xml/tinyxml.h"

#include "larpandoracontent/LArContent.h"
#include "larpandoracontent/LArControlFlow/MasterAlgorithm.h"
#include "larpandoracontent/LArControlFlow/MultiPandoraApi.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArPersistency/EventReadingAlgorithm.h"
#include "larpandoracontent/LArPlugins/LArPseudoLayerPlugin.h"
#include "larpandoracontent/LArPlugins/LArRotationalTransformationPlugin.h"

#ifdef LIBTORCH_DL
#include "larpandoradlcontent/LArDLContent.h"
#endif

#include "PandoraInterface.h"

#ifdef MONITORING
#include "TApplication.h"
#endif

#include <getopt.h>
#include <iostream>
#include <string>

using namespace pandora;
using namespace lar_reco;

int main(int argc, char *argv[])
{
    int errorNo(0);
    const Pandora *pPrimaryPandora(nullptr);

    try
    {
        Parameters parameters;

        if (!ParseCommandLine(argc, argv, parameters))
            return 1;

#ifdef MONITORING
        TApplication *pTApplication = new TApplication("LArReco", &argc, argv);
        pTApplication->SetReturnFromRun(kTRUE);
#endif
        CreatePandoraInstances(parameters, pPrimaryPandora);

        if (!pPrimaryPandora)
            throw StatusCodeException(STATUS_CODE_FAILURE);

        ProcessEvents(parameters, pPrimaryPandora);
    }
    catch (const StatusCodeException &statusCodeException)
    {
        std::cerr << "Pandora StatusCodeException: " << statusCodeException.ToString() << statusCodeException.GetBackTrace() << std::endl;
        errorNo = 1;
    }
    catch (const StopProcessingException &)
    {
        // Exit gracefully
        errorNo = 0;
    }
    catch (...)
    {
        std::cerr << "Unknown exception: " << std::endl;
        errorNo = 1;
    }

    MultiPandoraApi::DeletePandoraInstances(pPrimaryPandora);
    return errorNo;
}

//------------------------------------------------------------------------------------------------------------------------------------------

namespace lar_reco
{

void CreatePandoraInstances(const Parameters &parameters, const Pandora *&pPrimaryPandora)
{
    pPrimaryPandora = new Pandora();
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::RegisterAlgorithms(*pPrimaryPandora));
#ifdef LIBTORCH_DL
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArDLContent::RegisterAlgorithms(*pPrimaryPandora));
#endif
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::RegisterBasicPlugins(*pPrimaryPandora));

    if (!pPrimaryPandora)
        throw StatusCodeException(STATUS_CODE_FAILURE);

    MultiPandoraApi::AddPrimaryPandoraInstance(pPrimaryPandora);

    ProcessExternalParameters(parameters, pPrimaryPandora);
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetPseudoLayerPlugin(*pPrimaryPandora, new lar_content::LArPseudoLayerPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetLArTransformationPlugin(*pPrimaryPandora, new lar_content::LArRotationalTransformationPlugin));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPrimaryPandora, parameters.m_settingsFile));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessEvents(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    int nEvents(0);

    while ((nEvents++ < parameters.m_nEventsToProcess) || (0 > parameters.m_nEventsToProcess))
    {
        if (parameters.m_shouldDisplayEventNumber)
            std::cout << std::endl << "   PROCESSING EVENT: " << (nEvents - 1) << std::endl << std::endl;

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(*pPrimaryPandora));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*pPrimaryPandora));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ParseCommandLine(int argc, char *argv[], Parameters &parameters)
{
    if (1 == argc)
        return PrintOptions();

    int c(0);
    std::string recoOption;

    while ((c = getopt(argc, argv, "r:i:e:g:n:s:pNh")) != -1)
    {
        switch (c)
        {
        case 'r':
            recoOption = optarg;
            break;
        case 'i':
            parameters.m_settingsFile = optarg;
            break;
        case 'e':
            parameters.m_eventFileNameList = optarg;
            break;
        case 'g':
            parameters.m_geometryFileName = optarg;
            break;
        case 'n':
            parameters.m_nEventsToProcess = atoi(optarg);
            break;
        case 's':
            parameters.m_nEventsToSkip = atoi(optarg);
            break;
        case 'p':
            parameters.m_printOverallRecoStatus = true;
            break;
        case 'N':
            parameters.m_shouldDisplayEventNumber = true;
            break;
        case 'h':
        default:
            return PrintOptions();
        }
    }

    return ProcessRecoOption(recoOption, parameters);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PrintOptions()
{
    std::cout << std::endl << "./bin/PandoraInterface " << std::endl
              << "    -r RecoOption          (required) [Full, AllHitsCR, AllHitsNu, CRRemHitsSliceCR, CRRemHitsSliceNu, AllHitsSliceCR, AllHitsSliceNu]" << std::endl
              << "    -i Settings            (required) [algorithm description: xml]" << std::endl
              << "    -e EventFileList       (optional) [colon-separated list of files: xml/pndr]" << std::endl
              << "    -g GeometryFile        (optional) [detector geometry description: xml/pndr]" << std::endl
              << "    -n NEventsToProcess    (optional) [no. of events to process]" << std::endl
              << "    -s NEventsToSkip       (optional) [no. of events to skip in first file]" << std::endl
              << "    -p                     (optional) [print status]" << std::endl
              << "    -N                     (optional) [print event numbers]" << std::endl << std::endl;

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ProcessRecoOption(const std::string &recoOption, Parameters &parameters)
{
    std::string chosenRecoOption(recoOption);
    std::transform(chosenRecoOption.begin(), chosenRecoOption.end(), chosenRecoOption.begin(), ::tolower);

    if ("full" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunStitching = true;
        parameters.m_shouldRunCosmicHitRemoval = true;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldPerformSliceId = true;
    }
    else if ("allhitscr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunStitching = true;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = false;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldPerformSliceId = false;
    }
    else if ("nostitchingcr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunStitching = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = false;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldPerformSliceId = false;
    } 
    else if ("allhitsnu" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunStitching = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = false;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldPerformSliceId = false;
    }
    else if ("crremhitsslicecr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunStitching = true;
        parameters.m_shouldRunCosmicHitRemoval = true;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldPerformSliceId = false;
    }
    else if ("crremhitsslicenu" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunStitching = true;
        parameters.m_shouldRunCosmicHitRemoval = true;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldPerformSliceId = false;
    }
    else if ("allhitsslicecr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunStitching = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldPerformSliceId = false;
    }
    else if ("allhitsslicenu" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunStitching = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldPerformSliceId = false;
    }
    else
    {
        std::cout << "LArReco, Unrecognized reconstruction option: " << recoOption << std::endl << std::endl;
        return PrintOptions();
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessExternalParameters(const Parameters &parameters, const Pandora *const pPandora)
{
    auto *const pEventReadingParameters = new lar_content::EventReadingAlgorithm::ExternalEventReadingParameters;
    pEventReadingParameters->m_geometryFileName = parameters.m_geometryFileName;
    pEventReadingParameters->m_eventFileNameList = parameters.m_eventFileNameList;
    if (parameters.m_nEventsToSkip.IsInitialized()) pEventReadingParameters->m_skipToEvent = parameters.m_nEventsToSkip.Get();
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetExternalParameters(*pPandora, "LArEventReading", pEventReadingParameters));

    auto *const pEventSteeringParameters = new lar_content::MasterAlgorithm::ExternalSteeringParameters;
    pEventSteeringParameters->m_shouldRunAllHitsCosmicReco = parameters.m_shouldRunAllHitsCosmicReco;
    pEventSteeringParameters->m_shouldRunStitching = parameters.m_shouldRunStitching;
    pEventSteeringParameters->m_shouldRunCosmicHitRemoval = parameters.m_shouldRunCosmicHitRemoval;
    pEventSteeringParameters->m_shouldRunSlicing = parameters.m_shouldRunSlicing;
    pEventSteeringParameters->m_shouldRunNeutrinoRecoOption = parameters.m_shouldRunNeutrinoRecoOption;
    pEventSteeringParameters->m_shouldRunCosmicRecoOption = parameters.m_shouldRunCosmicRecoOption;
    pEventSteeringParameters->m_shouldPerformSliceId = parameters.m_shouldPerformSliceId;
    pEventSteeringParameters->m_printOverallRecoStatus = parameters.m_printOverallRecoStatus;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetExternalParameters(*pPandora, "LArMaster", pEventSteeringParameters));

#ifdef LIBTORCH_DL
    auto *const pEventSettingsParametersCopy = new lar_content::MasterAlgorithm::ExternalSteeringParameters(*pEventSteeringParameters);
    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::ExternallyConfiguredAlgorithm::SetExternalParameters(*pPandora,
        "LArDLMaster", pEventSettingsParametersCopy));
#endif
}

} // namespace lar_reco
