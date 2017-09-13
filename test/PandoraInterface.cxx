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
        std::cerr << "Pandora StatusCodeException: " << statusCodeException.ToString() << std::endl;
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
    LArDriftVolumeList driftVolumeList;
    LoadDriftVolumes(parameters, driftVolumeList);

    // Single volume: one Pandora instance. Multiple volumes: one Pandora for each volume and additional instance for stitching volumes
    CreatePrimaryPandoraInstance(parameters, driftVolumeList, pPrimaryPandora);
    CreateDaughterPandoraInstances(parameters, driftVolumeList, pPrimaryPandora);
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

void LoadDriftVolumes(const Parameters &parameters, LArDriftVolumeList &driftVolumeList)
{
    TiXmlDocument xmlDocument(parameters.m_driftVolumeDescriptionFile);

    if (!xmlDocument.LoadFile())
    {
        std::cout << "LArReco, LoadDriftVolumes - Invalid xml file: " << parameters.m_driftVolumeDescriptionFile << std::endl;
        throw StatusCodeException(STATUS_CODE_FAILURE);
    }

    for (TiXmlElement *pXmlElement = TiXmlHandle(&xmlDocument).FirstChild("LArDriftVolume").Element(); nullptr != pXmlElement;
        pXmlElement = pXmlElement->NextSiblingElement("LArDriftVolume"))
    {
        const TiXmlHandle xmlHandle(pXmlElement);

        unsigned int volumeID(std::numeric_limits<unsigned int>::max());
        bool isPositiveDrift(true);
        double wirePitchU(std::numeric_limits<double>::max());
        double wirePitchV(std::numeric_limits<double>::max());
        double wirePitchW(std::numeric_limits<double>::max());
        double wireAngleU(std::numeric_limits<double>::max());
        double wireAngleV(std::numeric_limits<double>::max());
        double centerX(std::numeric_limits<double>::max());
        double centerY(std::numeric_limits<double>::max());
        double centerZ(std::numeric_limits<double>::max());
        double widthX(std::numeric_limits<double>::max());
        double widthY(std::numeric_limits<double>::max());
        double widthZ(std::numeric_limits<double>::max());
        double sigmaUVZ(std::numeric_limits<double>::max());

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "VolumeID", volumeID));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "IsPositiveDrift", isPositiveDrift));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WirePitchU", wirePitchU));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WirePitchV", wirePitchV));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WirePitchW", wirePitchW));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WireAngleU", wireAngleU));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WireAngleV", wireAngleV));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "CenterX", centerX));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "CenterY", centerY));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "CenterZ", centerZ));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WidthX", widthX));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WidthY", widthY));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "WidthZ", widthZ));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "SigmaUVZ", sigmaUVZ));

        driftVolumeList.emplace_back(LArDriftVolume(volumeID, isPositiveDrift, wirePitchU, wirePitchV, wirePitchW, wireAngleU, wireAngleV,
            centerX, centerY, centerZ, widthX, widthY, widthZ, sigmaUVZ));
    }

    if (driftVolumeList.empty())
    {
        std::cout << "LArReco, LoadDriftVolumes - List of drift volumes is empty." << std::endl;
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreatePrimaryPandoraInstance(const Parameters &parameters, const LArDriftVolumeList &driftVolumeList, const Pandora *&pPrimaryPandora)
{
    pPrimaryPandora = CreateNewPandora();

    if (!pPrimaryPandora)
        throw StatusCodeException(STATUS_CODE_FAILURE);

    const std::string configFileName((driftVolumeList.size() > 1) ? parameters.m_stitchingSettingsFile : parameters.m_settingsFile);
    const LArDriftVolume &driftVolume(driftVolumeList.front());

    MultiPandoraApi::AddPrimaryPandoraInstance(pPrimaryPandora);

    // If only single drift volume, primary pandora instance will do all pattern recognition, rather than perform a particle stitching role
    if (1 == driftVolumeList.size())
    {
        // For single volume cases only - pass commandline parameters to algorithms
        ProcessExternalParameters(parameters, pPrimaryPandora);

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPrimaryPandora,
            new lar_content::LArRotationalTransformationPlugin(driftVolume.GetWireAngleU(), driftVolume.GetWireAngleV(), driftVolume.GetSigmaUVZ())));

        MultiPandoraApi::SetVolumeInfo(pPrimaryPandora, new VolumeInfo(0, "driftVolume",
            driftVolume.GetCenterX(), driftVolume.GetCenterY(), driftVolume.GetCenterZ(),
            driftVolume.GetWidthX(), driftVolume.GetWidthY(), driftVolume.GetWidthZ(),
            driftVolume.IsPositiveDrift()));
    }

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPrimaryPandora,
        new lar_content::LArPseudoLayerPlugin(driftVolume.GetWirePitchU(), driftVolume.GetWirePitchV(), driftVolume.GetWirePitchW())));

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPrimaryPandora, configFileName));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreateDaughterPandoraInstances(const Parameters &parameters, const LArDriftVolumeList &driftVolumeList, const Pandora *const pPrimaryPandora)
{
    if (!pPrimaryPandora)
    {
        std::cout << "Trying to create daughter Pandora instances in absence of primary instance" << std::endl;
        throw StatusCodeException(STATUS_CODE_FAILURE);
    }

    if (driftVolumeList.size() < 2)
        return;

    if (parameters.m_stitchingSettingsFile.empty())
    {
        std::cout << "Multiple drift volumes present - must provide a stitching settings file." << std::endl;
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }

    for (const LArDriftVolume &driftVolume : driftVolumeList)
    {
        const Pandora *const pPandora = CreateNewPandora();

        if (!pPandora)
            throw StatusCodeException(STATUS_CODE_FAILURE);

        std::ostringstream volumeIdString;
        volumeIdString << driftVolume.GetVolumeID();

        MultiPandoraApi::AddDaughterPandoraInstance(pPrimaryPandora, pPandora);
        MultiPandoraApi::SetVolumeInfo(pPandora, new VolumeInfo(driftVolume.GetVolumeID(), "driftVolume_" + volumeIdString.str(),
            driftVolume.GetCenterX(), driftVolume.GetCenterY(), driftVolume.GetCenterZ(),
            driftVolume.GetWidthX(), driftVolume.GetWidthY(), driftVolume.GetWidthZ(),
            driftVolume.IsPositiveDrift()));

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandora,
            new lar_content::LArPseudoLayerPlugin(driftVolume.GetWirePitchU(), driftVolume.GetWirePitchV(), driftVolume.GetWirePitchW())));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandora,
            new lar_content::LArRotationalTransformationPlugin(driftVolume.GetWireAngleU(), driftVolume.GetWireAngleV(), driftVolume.GetSigmaUVZ())));

        std::string thisConfigFileName(parameters.m_settingsFile);
        const size_t insertPosition((thisConfigFileName.length() < 4) ? 0 : thisConfigFileName.length() - std::string(".xml").length());
            thisConfigFileName = thisConfigFileName.insert(insertPosition, volumeIdString.str());

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

    while ((c = getopt(argc, argv, "r:i:e:v:g:n:s:pNh")) != -1)
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
        case 'v':
            parameters.m_driftVolumeDescriptionFile = optarg;
            break;
        case 'g':
            parameters.m_geometryFileName = optarg;
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
              << "    -e EventFileList       (required) [colon-separated list of files: xml/pndr]" << std::endl
              << "    -v DriftVolumeFile     (required) [drift volume description: xml]" << std::endl
              << "    -g GeometryFile        (optional) [detector gap description: xml/pndr]" << std::endl
              << "    -t StitchingSettings   (optional) [stitching algorithm description: xml]" << std::endl
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

void ProcessExternalParameters(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    auto *const pEventReadingParameters = new lar_content::EventReadingAlgorithm::ExternalEventReadingParameters;
    pEventReadingParameters->m_geometryFileName = parameters.m_geometryFileName;
    pEventReadingParameters->m_eventFileNameList = parameters.m_eventFileNameList;
    pEventReadingParameters->m_skipToEvent = parameters.m_nEventsToSkip;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, pandora::ExternallyConfiguredAlgorithm::SetExternalParameters(*pPrimaryPandora, "LArEventReading", pEventReadingParameters));

    auto *const pEventSteeringParameters = new lar_content::ParentAlgorithm::ExternalSteeringParameters;
    pEventSteeringParameters->m_shouldRunAllHitsCosmicReco = parameters.m_shouldRunAllHitsCosmicReco;
    pEventSteeringParameters->m_shouldRunCosmicHitRemoval = parameters.m_shouldRunCosmicHitRemoval;
    pEventSteeringParameters->m_shouldRunSlicing = parameters.m_shouldRunSlicing;
    pEventSteeringParameters->m_shouldRunNeutrinoRecoOption = parameters.m_shouldRunNeutrinoRecoOption;
    pEventSteeringParameters->m_shouldRunCosmicRecoOption = parameters.m_shouldRunCosmicRecoOption;
    pEventSteeringParameters->m_shouldIdentifyNeutrinoSlice = parameters.m_shouldIdentifyNeutrinoSlice;
    pEventSteeringParameters->m_printOverallRecoStatus = parameters.m_printOverallRecoStatus;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, pandora::ExternallyConfiguredAlgorithm::SetExternalParameters(*pPrimaryPandora, "LArParent", pEventSteeringParameters));
}

} // namespace lar_reco
