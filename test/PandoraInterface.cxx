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
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArPersistency/EventReadingAlgorithm.h"
#include "larpandoracontent/LArPlugins/LArPseudoLayerPlugin.h"
#include "larpandoracontent/LArPlugins/LArRotationalTransformationPlugin.h"
#include "larpandoracontent/LArStitching/MultiPandoraApi.h"
#include "larpandoracontent/LArUtility/ParentAlgorithm.h"

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
    CreatePrimaryPandoraInstance(parameters, pPrimaryPandora);

    if (parameters.m_nDriftVolumes > 1)
        CreateDaughterPandoraInstances(parameters, pPrimaryPandora);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessEvents(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    int nEvents(0);
    PandoraInstanceList pandoraInstanceList(MultiPandoraApi::GetDaughterPandoraInstanceList(pPrimaryPandora));
    pandoraInstanceList.push_back(pPrimaryPandora);

    while ((nEvents++ < parameters.m_nEventsToProcess) || (0 > parameters.m_nEventsToProcess))
    {
        if (parameters.m_shouldDisplayEventNumber)
            std::cout << std::endl << "   PROCESSING EVENT: " << (nEvents - 1) << std::endl << std::endl;

        for (const Pandora *const pPandora : pandoraInstanceList)
            PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(*pPandora));

        for (const Pandora *const pPandora : pandoraInstanceList)
            PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*pPandora));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreatePrimaryPandoraInstance(const Parameters &parameters, const Pandora *&pPrimaryPandora)
{
    pPrimaryPandora = CreateNewPandora();

    if (!pPrimaryPandora)
        throw StatusCodeException(STATUS_CODE_FAILURE);

    MultiPandoraApi::AddPrimaryPandoraInstance(pPrimaryPandora);

    // If only single drift volume, primary pandora instance will do all pattern recognition, rather than perform a particle stitching role
    if (1 == parameters.m_nDriftVolumes)
    {
        ProcessExternalParameters(parameters, pPrimaryPandora);
        MultiPandoraApi::SetVolumeId(pPrimaryPandora, 0);
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetPseudoLayerPlugin(*pPrimaryPandora, new lar_content::LArPseudoLayerPlugin));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetLArTransformationPlugin(*pPrimaryPandora, new lar_content::LArRotationalTransformationPlugin));
    }

    const std::string configFileName((parameters.m_nDriftVolumes > 1) ? parameters.m_stitchingSettingsFile : parameters.m_settingsFile);
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPrimaryPandora, configFileName));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreateDaughterPandoraInstances(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    if (!pPrimaryPandora)
    {
        std::cout << "Trying to create daughter Pandora instances in absence of primary instance" << std::endl;
        throw StatusCodeException(STATUS_CODE_FAILURE);
    }

    if (parameters.m_nDriftVolumes < 2)
        return;

    if (parameters.m_stitchingSettingsFile.empty())
    {
        std::cout << "Multiple drift volumes present - must provide a stitching settings file." << std::endl;
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }

    for (unsigned int volumeId = 0; volumeId < parameters.m_nDriftVolumes; ++volumeId)
    {
        const std::string volumeIdString(pandora::TypeToString(volumeIdString));
        const Pandora *const pPandora = CreateNewPandora();

        if (!pPandora)
            throw StatusCodeException(STATUS_CODE_FAILURE);

        ProcessExternalParameters(parameters, pPandora, volumeIdString);
        MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandora);
        MultiPandoraApi::SetVolumeId(pPandora, volumeId);

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetPseudoLayerPlugin(*pPandora, new lar_content::LArPseudoLayerPlugin));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetLArTransformationPlugin(*pPandora, new lar_content::LArRotationalTransformationPlugin));

        const std::string thisConfigFileName(!parameters.m_uniqueInstanceSettings ? parameters.m_settingsFile : ReplaceString(parameters.m_settingsFile, ".", volumeIdString + "."));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandora, thisConfigFileName));
    }
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

bool ParseCommandLine(int argc, char *argv[], Parameters &parameters)
{
    if (1 == argc)
        return PrintOptions();

    int c(0);
    std::string recoOption;

    while ((c = getopt(argc, argv, "r:i:e:g:v:t:n:s:upNh")) != -1)
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
        case 'v':
            parameters.m_nDriftVolumes = atoi(optarg);
            break;
        case 't':
            parameters.m_stitchingSettingsFile = optarg;
            break;
        case 'n':
            parameters.m_nEventsToProcess = atoi(optarg);
            break;
        case 's':
            parameters.m_nEventsToSkip = atoi(optarg);
            break;
        case 'u':
            parameters.m_uniqueInstanceSettings = true;
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
              << "    -g GeometryFile        (optional) [detector gap description: xml/pndr]" << std::endl
              << "    -v NDriftVolumes       (optional) [number of drift volumes: default 1]" << std::endl
              << "    -t StitchingSettings   (optional) [stitching algorithm description: xml]" << std::endl
              << "    -n NEventsToProcess    (optional) [no. of events to process]" << std::endl
              << "    -s NEventsToSkip       (optional) [no. of events to skip in first file]" << std::endl
              << "    -u                     (optional) [use unique settings file for each pandora instance]" << std::endl
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
        parameters.m_shouldRunCosmicHitRemoval = true;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldIdentifyNeutrinoSlice = true;
    }
    else if ("allhitscr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = false;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldIdentifyNeutrinoSlice = false;
    }
    else if ("allhitsnu" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = false;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldIdentifyNeutrinoSlice = false;
    }
    else if ("crremhitsslicecr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunCosmicHitRemoval = true;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldIdentifyNeutrinoSlice = false;
    }
    else if ("crremhitsslicenu" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = true;
        parameters.m_shouldRunCosmicHitRemoval = true;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldIdentifyNeutrinoSlice = false;
    }
    else if ("allhitsslicecr" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = false;
        parameters.m_shouldRunCosmicRecoOption = true;
        parameters.m_shouldIdentifyNeutrinoSlice = false;
    }
    else if ("allhitsslicenu" == chosenRecoOption)
    {
        parameters.m_shouldRunAllHitsCosmicReco = false;
        parameters.m_shouldRunCosmicHitRemoval = false;
        parameters.m_shouldRunSlicing = true;
        parameters.m_shouldRunNeutrinoRecoOption = true;
        parameters.m_shouldRunCosmicRecoOption = false;
        parameters.m_shouldIdentifyNeutrinoSlice = false;
    }
    else
    {
        std::cout << "LArReco, Unrecognized reconstruction option: " << recoOption << std::endl << std::endl;
        return PrintOptions();
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessExternalParameters(const Parameters &parameters, const Pandora *const pPandora, const std::string volumeIdString)
{
    auto *const pEventReadingParameters = new lar_content::EventReadingAlgorithm::ExternalEventReadingParameters;
    pEventReadingParameters->m_geometryFileName = (volumeIdString.empty() ? parameters.m_geometryFileName : ReplaceString(parameters.m_geometryFileName, ".", volumeIdString + "."));
    pEventReadingParameters->m_eventFileNameList = (volumeIdString.empty() ? parameters.m_eventFileNameList : ReplaceString(parameters.m_eventFileNameList, ".", volumeIdString + "."));
    pEventReadingParameters->m_skipToEvent = parameters.m_nEventsToSkip;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetExternalParameters(*pPandora, "LArEventReading", pEventReadingParameters));

    auto *const pEventSteeringParameters = new lar_content::ParentAlgorithm::ExternalSteeringParameters;
    pEventSteeringParameters->m_shouldRunAllHitsCosmicReco = parameters.m_shouldRunAllHitsCosmicReco;
    pEventSteeringParameters->m_shouldRunCosmicHitRemoval = parameters.m_shouldRunCosmicHitRemoval;
    pEventSteeringParameters->m_shouldRunSlicing = parameters.m_shouldRunSlicing;
    pEventSteeringParameters->m_shouldRunNeutrinoRecoOption = parameters.m_shouldRunNeutrinoRecoOption;
    pEventSteeringParameters->m_shouldRunCosmicRecoOption = parameters.m_shouldRunCosmicRecoOption;
    pEventSteeringParameters->m_shouldIdentifyNeutrinoSlice = parameters.m_shouldIdentifyNeutrinoSlice;
    pEventSteeringParameters->m_printOverallRecoStatus = parameters.m_printOverallRecoStatus;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetExternalParameters(*pPandora, "LArParent", pEventSteeringParameters));
}

//------------------------------------------------------------------------------------------------------------------------------------------

std::string ReplaceString(std::string subject, const std::string &search, const std::string &replacement)
{
    size_t pos(0);

    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
         subject.replace(pos, search.length(), replacement);
         pos += replacement.length();
    }

    return subject;
}

} // namespace lar_reco
