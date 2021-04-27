/**
 *  @file   LArRecoMP/test/PandoraInterface.cc
 *
 *  @brief  Implementation of the lar reco mp application
 *
 *  $Log: $
 */

#include "TFile.h"
#include "TTree.h"

#include "TG4Event.h"
#include "TGeoManager.h"
#include "TObjArray.h"
#include "TGeoVolume.h"
#include "TObject.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"

#include "Api/PandoraApi.h"
#include "Helpers/XmlHelper.h"
#include "Xml/tinyxml.h"

#include "larpandoracontent/LArContent.h"
#include "larpandoracontent/LArControlFlow/MultiPandoraApi.h"
#include "larpandoracontent/LArPlugins/LArPseudoLayerPlugin.h"
#include "larpandoracontent/LArPlugins/LArRotationalTransformationPlugin.h"

#include "PandoraInterface.h"

#ifdef MONITORING
#include "TApplication.h"
#endif

#include <getopt.h>
#include <iostream>
#include <random>
#include <string>

using namespace pandora;
using namespace lar_nd_reco;

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

        pPrimaryPandora = new pandora::Pandora();

        if (!pPrimaryPandora)
            throw StatusCodeException(STATUS_CODE_FAILURE);

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::RegisterAlgorithms(*pPrimaryPandora));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, LArContent::RegisterBasicPlugins(*pPrimaryPandora));

        MultiPandoraApi::AddPrimaryPandoraInstance(pPrimaryPandora);

	CreateGeometry(parameters, pPrimaryPandora);

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::SetPseudoLayerPlugin(*pPrimaryPandora, new lar_content::LArPseudoLayerPlugin));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=,
            PandoraApi::SetLArTransformationPlugin(*pPrimaryPandora, new lar_content::LArRotationalTransformationPlugin));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPrimaryPandora, parameters.m_settingsFile));

        ProcessEvents(parameters, pPrimaryPandora);
    }
    catch (const StatusCodeException &statusCodeException)
    {
        std::cerr << "Pandora StatusCodeException: " << statusCodeException.ToString() << statusCodeException.GetBackTrace() << std::endl;
        errorNo = 1;
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

namespace lar_nd_reco
{

void CreateGeometry(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{ 
    //Get the geometry info from the input root file
    TFile fileSource(parameters.m_inputFileName.c_str(), "READ");
    TGeoManager *pEDepSimGeo = (TGeoManager*)fileSource.Get("EDepSimGeometry");

    if (!pEDepSimGeo)
    {
        std::cout << "Missing the geometry info" << std::endl;
        return;
    }

    //Start by looking at the top level volume and move down to the one we need
    TGeoVolume *pMasterVol = pEDepSimGeo->GetMasterVolume();
    for (int i = 0; i < pMasterVol->GetNdaughters(); i++)
    {
      pEDepSimGeo->CdDown(i);
    }
    
    TGeoNode* currentnode = pEDepSimGeo->GetCurrentNode();
    std::string name(currentnode->GetName());										     
    for (int i = 0; i < currentnode->GetVolume()->GetNdaughters(); i++)
    {
      TGeoNode* node = pEDepSimGeo->GetCurrentNode();;
      pEDepSimGeo->CdDown(i);
      for (int i2=0; i2< node->GetNdaughters(); ++i2) {
	    pEDepSimGeo->CdDown(i+1);
        }
    }
      
    //This should now be the ArgonCube volume
    currentnode = pEDepSimGeo->GetCurrentNode();
    name = currentnode->GetName();
    std::cout << "Current Node: " << name << std::endl;
    std::cout << "Current N daughters: " << currentnode->GetVolume()->GetNdaughters() << std::endl;
    std::cout << "  " << std::endl;

    //Get the BBox for the ArgonCube
    TGeoVolume *pCurrentVol = currentnode->GetVolume();
    TGeoShape *pCurrentShape = pCurrentVol->GetShape();
    pCurrentShape->InspectShape();
    TGeoBBox *pBox = dynamic_cast<TGeoBBox*>(pCurrentShape);

    //Now can get origin/width data from the BBox
    std::cout << "   " << std::endl;
    double dx = pBox->GetDX();        //Note these are the half widths
    double dy = pBox->GetDY();
    double dz = pBox->GetDZ();
    const double* origin = pBox->GetOrigin();

    //Translate the origin coordinates from the 4th level to the first.
    //Doesn't seem to change anything. Needed?
    Double_t level3[3] = { 0. , 0. , 0. };
    currentnode->LocalToMasterVect(origin, level3);
    Double_t level2[3] = { 0. , 0. , 0. };
    currentnode->LocalToMasterVect(level3, level2);
    Double_t level1[3] = { 0. , 0. , 0. };
    currentnode->LocalToMasterVect(level2, level1);

    //Can now create a geometry using the found parameters
    PandoraApi::Geometry::LArTPC::Parameters geoparameters;
    
    try {
        geoparameters.m_centerX = level1[0];
	//ATTN: offsets taken by visual comparison with edep-disp
        geoparameters.m_centerY = level1[1]-675;
        geoparameters.m_centerZ = level1[2]+6660;
        geoparameters.m_widthX = dx*2;
        geoparameters.m_widthY = dy*2;
        geoparameters.m_widthZ = dz*2;
	//ATTN: parameters past here taken from uboone
	geoparameters.m_larTPCVolumeId = 0;
	geoparameters.m_wirePitchU = 0.300000011921;
        geoparameters.m_wirePitchV = 0.300000011921;
        geoparameters.m_wirePitchW = 0.300000011921;
        geoparameters.m_wireAngleU = 1.04719758034;
        geoparameters.m_wireAngleV = -1.04719758034;
        geoparameters.m_wireAngleW = 0.;
        geoparameters.m_sigmaUVW = 1;
        geoparameters.m_isDriftInPositiveX = 0;
    }
    catch (const pandora::StatusCodeException&) {
	std::cout << "CreatePandoraLArTPCs - invalid tpc parameter provided" << std::endl;
    }

    try {
        PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::Geometry::LArTPC::Create(*pPrimaryPandora, geoparameters));
    }
    catch (const pandora::StatusCodeException&) {
	std::cout << "CreatePandoraLArTPCs - unable to create tpc, insufficient or invalid information supplied" << std::endl;
    }
    
}


//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessEvents(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    int nEvents(0);

    TFile fileSource(parameters.m_inputFileName.c_str(), "READ");
    TTree *pEDepSimTree = (TTree*)fileSource.Get("EDepSimEvents");

    if (!pEDepSimTree)
    {
        std::cout << "Missing the event tree" << std::endl;
        return;
    }

    TG4Event *pEDepSimEvent(nullptr);
    pEDepSimTree->SetBranchAddress("Event", &pEDepSimEvent);
    // fileSource->Get("EDepSimGeometry");

    while ((nEvents < parameters.m_nEventsToProcess) || (0 > parameters.m_nEventsToProcess))
    {
        if (parameters.m_shouldDisplayEventNumber)
            std::cout << std::endl << "   PROCESSING EVENT: " << nEvents << std::endl << std::endl;

        pEDepSimTree->GetEntry(nEvents++);

        if (!pEDepSimEvent)
            return;

        int hitCounter(0);

        for (TG4HitSegmentDetectors::iterator detector = pEDepSimEvent->SegmentDetectors.begin(); detector != pEDepSimEvent->SegmentDetectors.end(); ++detector)
        {
            std::cout << "Show hits for " << detector->first << " (" << detector->second.size() << " hits)" << std::endl;

            for (TG4HitSegmentContainer::iterator g4Hit = detector->second.begin(); g4Hit != detector->second.end(); ++g4Hit)
            {
                //const double length = g4Hit->GetTrackLength();
                //const int contrib = g4Hit->Contrib.front();
                //const std::string particle = pEDepSimEvent->Trajectories[contrib].GetName();

                const double energy = g4Hit->GetEnergyDeposit();
                const CartesianVector start(g4Hit->GetStart().X(), g4Hit->GetStart().Y(), g4Hit->GetStart().Z());
                const CartesianVector stop(g4Hit->GetStop().X(), g4Hit->GetStop().Y(), g4Hit->GetStop().Z());

                PandoraApi::CaloHit::Parameters caloHitParameters;
                caloHitParameters.m_positionVector = (start + stop) * 0.5f;
                caloHitParameters.m_expectedDirection = pandora::CartesianVector(0.f, 0.f, 1.f);
                caloHitParameters.m_cellNormalVector = pandora::CartesianVector(0.f, 0.f, 1.f);
                caloHitParameters.m_cellGeometry = pandora::RECTANGULAR;
                caloHitParameters.m_cellSize0 = std::fabs(start.GetY() - stop.GetY());
                caloHitParameters.m_cellSize1 = std::fabs(start.GetX() - stop.GetX());
                caloHitParameters.m_cellThickness = std::fabs(start.GetZ() - stop.GetZ());
                caloHitParameters.m_nCellRadiationLengths = 1.f;
                caloHitParameters.m_nCellInteractionLengths = 1.f;
                caloHitParameters.m_time = 0.f;
                caloHitParameters.m_inputEnergy = energy;
                caloHitParameters.m_mipEquivalentEnergy = energy;
                caloHitParameters.m_electromagneticEnergy = energy;
                caloHitParameters.m_hadronicEnergy = energy;
                caloHitParameters.m_isDigital = false;
                caloHitParameters.m_hitType = pandora::TPC_3D;
                caloHitParameters.m_hitRegion = pandora::SINGLE_REGION;
                caloHitParameters.m_layer = 0;
                caloHitParameters.m_isInOuterSamplingLayer = false;
                caloHitParameters.m_pParentAddress = (void*)(static_cast<uintptr_t>(++hitCounter));

                PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::CaloHit::Create(*pPrimaryPandora, caloHitParameters));
            }
        }

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

    while ((c = getopt(argc, argv, ":i:e:n:s:Nh")) != -1)
    {
        switch (c)
        {
        case 'i':
            parameters.m_settingsFile = optarg;
            break;
        case 'e':
            parameters.m_inputFileName = optarg;
            break;
        case 'n':
            parameters.m_nEventsToProcess = atoi(optarg);
            break;
        case 's':
            parameters.m_nEventsToSkip = atoi(optarg);
            break;
        case 'N':
            parameters.m_shouldDisplayEventNumber = true;
            break;
        case 'h':
        default:
            return PrintOptions();
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PrintOptions()
{
    std::cout << std::endl << "./bin/PandoraInterface " << std::endl
              << "    -i Settings            (required) [algorithm description: xml]" << std::endl
              << "    -e EventsFile          (required) [input edep-sim file, typically containing events and geometry]" << std::endl
              << "    -n NEventsToProcess    (optional) [no. of events to process]" << std::endl
              << "    -s NEventsToSkip       (optional) [no. of events to skip in first file]" << std::endl
              << "    -p                     (optional) [print status]" << std::endl
              << "    -N                     (optional) [print event numbers]" << std::endl << std::endl;

    return false;
}

} // namespace lar_nd_reco
