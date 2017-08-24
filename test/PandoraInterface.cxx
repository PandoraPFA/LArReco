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

#include "PandoraInterface.h"

#ifdef MONITORING
#include "TApplication.h"
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/time.h>

using namespace pandora;
using namespace lar_reco;

int main(int argc, char *argv[])
{
    try
    {
        Parameters parameters;

        if (!ParseCommandLine(argc, argv, parameters))
            return 1;
#ifdef MONITORING
    TApplication *pTApplication = new TApplication("LArReco", &argc, argv);
    pTApplication->SetReturnFromRun(kTRUE);
#endif
        const Pandora *pPrimaryPandora(nullptr);
        CreatePandoraInstances(parameters, pPrimaryPandora);

        if (pPrimaryPandora)
        {
            ProcessEvents(parameters, pPrimaryPandora);
            MultiPandoraApi::DeletePandoraInstances(pPrimaryPandora);
        }
    }
    catch (StatusCodeException &statusCodeException)
    {
        std::cerr << "Pandora Exception caught: " << statusCodeException.ToString() << std::endl;
        return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------

namespace lar_reco
{

bool ParseCommandLine(int argc, char *argv[], Parameters &parameters)
{
    int c(0);

    while ((c = getopt(argc, argv, "d:i:v:s:e:g:n:t::N::h")) != -1)
    {
        switch (c)
        {
        case 'd':
            parameters.m_driftVolumeDescriptionFile = optarg;
            break;
        case 'i':
            parameters.m_pandoraSettingsFile = optarg;
            break;
        case 'v':
            parameters.m_stitchingSettingsFile = optarg;
            break;
        case 'e':
            parameters.m_eventFileName = optarg;
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
        case 't':
            parameters.m_shouldDisplayEventTime = true;
            break;
        case 'N':
            parameters.m_shouldDisplayEventNumber = true;
            break;
        case 'h':
        default:
            std::cout << std::endl << "./bin/PandoraInterface " << std::endl
                      << "    -d DriftVolumeDescription   (mandatory)" << std::endl
                      << "    -i PandoraSettings.xml      (mandatory)" << std::endl
                      << "    -v StitchingSettings.xml    (use-case dependent)" << std::endl
                      << "    -e PandoraEventFile         " << std::endl
                      << "    -g PandoraGeometryFile      " << std::endl
                      << "    -n NEventsToProcess         (optional)" << std::endl
                      << "    -s NEventsToSkip            (optional)" << std::endl
                      << "    -N                          (optional, display event numbers)" << std::endl
                      << "    -t                          (optional, display event times)" << std::endl << std::endl;
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreatePandoraInstances(const Parameters &parameters, const Pandora *&pPrimaryPandora)
{
    LArDriftVolumeList driftVolumeList;
    LoadGeometry(parameters, driftVolumeList);

    if (driftVolumeList.empty())
    {
        std::cout << "List of drift volumes is empty." << std::endl;
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }

    // Single volume: one Pandora instance. Multiple volumes: one Pandora for each volume and additional instance for stitching volumes
    if ((driftVolumeList.size() > 1) && parameters.m_stitchingSettingsFile.empty())
    {
        std::cout << "Multiple drift volumes present - must provide a stitching settings file." << std::endl;
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }

    CreatePrimaryPandoraInstance(parameters, driftVolumeList, pPrimaryPandora);

    if (driftVolumeList.size() > 1)
        CreateDaughterPandoraInstances(parameters, driftVolumeList, pPrimaryPandora);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void LoadGeometry(const Parameters &parameters, LArDriftVolumeList &driftVolumeList)
{
    TiXmlDocument xmlDocument(parameters.m_driftVolumeDescriptionFile);

    if (!xmlDocument.LoadFile())
    {
        std::cout << "LArReco, LoadGeometry - Invalid xml file: " << parameters.m_driftVolumeDescriptionFile << std::endl;
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
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreatePrimaryPandoraInstance(const Parameters &parameters, const LArDriftVolumeList &driftVolumeList, const Pandora *&pPrimaryPandora)
{
    pPrimaryPandora = CreateNewPandora();

    if (!pPrimaryPandora)
        throw StatusCodeException(STATUS_CODE_FAILURE);

    const std::string configFileName((driftVolumeList.size() > 1) ? parameters.m_stitchingSettingsFile : parameters.m_pandoraSettingsFile);
    const LArDriftVolume &driftVolume(driftVolumeList.front());

    MultiPandoraApi::AddPrimaryPandoraInstance(pPrimaryPandora);

    // For single volume cases only - pass commandline parameters to algorithms
    if (1 == driftVolumeList.size())
    {
        auto *const pEventReadingParameters = new lar_content::EventReadingAlgorithm::ExternalEventReadingParameters;
        pEventReadingParameters->m_geometryFileName = parameters.m_geometryFileName;
        pEventReadingParameters->m_eventFileName = parameters.m_eventFileName;
        pEventReadingParameters->m_skipToEvent = parameters.m_nEventsToSkip;
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, lar_content::EventReadingAlgorithm::SetExternalParameters(*pPrimaryPandora, pEventReadingParameters));
    }

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPrimaryPandora, configFileName));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPrimaryPandora,
        new lar_content::LArPseudoLayerPlugin(driftVolume.GetWirePitchU(), driftVolume.GetWirePitchV(), driftVolume.GetWirePitchW())));

    // If only single drift volume, primary pandora instance will do all pattern recognition, rather than perform a particle stitching role
    if (1 == driftVolumeList.size())
    {
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPrimaryPandora,
            new lar_content::LArRotationalTransformationPlugin(driftVolume.GetWireAngleU(), driftVolume.GetWireAngleV(), driftVolume.GetSigmaUVZ())));

        MultiPandoraApi::SetVolumeInfo(pPrimaryPandora, new VolumeInfo(0, "driftVolume",
            driftVolume.GetCenterX(), driftVolume.GetCenterY(), driftVolume.GetCenterZ(),
            driftVolume.GetWidthX(), driftVolume.GetWidthY(), driftVolume.GetWidthZ(),
            driftVolume.IsPositiveDrift()));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CreateDaughterPandoraInstances(const Parameters &parameters, const LArDriftVolumeList &driftVolumeList, const Pandora *const pPrimaryPandora)
{
    if (!pPrimaryPandora)
    {
        std::cout << "Trying to create daughter Pandora instances in absence of primary instance" << std::endl;
        throw StatusCodeException(STATUS_CODE_FAILURE);
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

        std::string thisConfigFileName(parameters.m_pandoraSettingsFile);
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

} // namespace lar_reco
