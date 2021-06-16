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
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoShape.h"
#include "TGeoVolume.h"

#include "Api/PandoraApi.h"
#include "Helpers/XmlHelper.h"
#include "Managers/PluginManager.h"
#include "Xml/tinyxml.h"

#include "larpandoracontent/LArContent.h"
#include "larpandoracontent/LArControlFlow/MultiPandoraApi.h"
#include "larpandoracontent/LArObjects/LArCaloHit.h"
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
    // Get the geometry info from the input root file
    TFile fileSource(parameters.m_inputFileName.c_str(), "READ");
    TGeoManager *pEDepSimGeo = (TGeoManager *)fileSource.Get("EDepSimGeometry");

    if (!pEDepSimGeo)
    {
        std::cout << "Missing the geometry info" << std::endl;
        return;
    }

    // Start by looking at the top level volume and move down to the one we need
    std::string name;
    std::string needednode("volArgonCubeDetector_PV_0");
    TGeoNode *currentnode = pEDepSimGeo->GetCurrentNode();
    bool foundnode(false);

    TGeoVolume *pMasterVol = pEDepSimGeo->GetMasterVolume();
    for (int i = 0; i < pMasterVol->GetNdaughters(); i++)
    {
        pEDepSimGeo->CdDown(i);
    }

    while (foundnode == false)
    {
        currentnode = pEDepSimGeo->GetCurrentNode();
        name = currentnode->GetName();

        int i1 = 0;
        for (int i = 0; i < currentnode->GetNdaughters(); i++)
        {
            pEDepSimGeo->CdDown(i1);
            TGeoNode *node = pEDepSimGeo->GetCurrentNode();
            name = node->GetName();
            if (name == needednode)
            {
                foundnode = true;
                break;
            }
            else if (i + 1 != currentnode->GetNdaughters())
            {
                pEDepSimGeo->CdUp();
                i1++;
            }
        }

        if (foundnode == true)
        {
            break;
        }
    }

    // This should now be the ArgonCube volume
    currentnode = pEDepSimGeo->GetCurrentNode();
    name = currentnode->GetName();
    std::cout << "Current Node: " << name << std::endl;
    std::cout << "Current N daughters: " << currentnode->GetVolume()->GetNdaughters() << std::endl;
    std::cout << "  " << std::endl;

    // Get the BBox for the ArgonCube
    TGeoVolume *pCurrentVol = currentnode->GetVolume();
    TGeoShape *pCurrentShape = pCurrentVol->GetShape();
    pCurrentShape->InspectShape();
    TGeoBBox *pBox = dynamic_cast<TGeoBBox *>(pCurrentShape);

    // mm to cm conversion
    const double mm2cm(0.1);

    // Now can get origin/width data from the BBox
    double dx = pBox->GetDX() * mm2cm; // Note these are the half widths
    double dy = pBox->GetDY() * mm2cm;
    double dz = pBox->GetDZ() * mm2cm;
    const double *origin = pBox->GetOrigin();

    // Translate the origin coordinates from the 4th level to the first.
    // Doesn't seem to change anything. Needed?
    double level3[3] = {0.0, 0.0, 0.0};
    currentnode->LocalToMasterVect(origin, level3);
    double level2[3] = {0.0, 0.0, 0.0};
    currentnode->LocalToMasterVect(level3, level2);
    double level1[3] = {0.0, 0.0, 0.0};
    currentnode->LocalToMasterVect(level2, level1);

    // Can now create a geometry using the found parameters
    PandoraApi::Geometry::LArTPC::Parameters geoparameters;

    try
    {
        geoparameters.m_centerX = level1[0] * mm2cm;
        // ATTN: offsets taken by visual comparison with edep-disp
        geoparameters.m_centerY = (level1[1] - 675.0) * mm2cm;
        geoparameters.m_centerZ = (level1[2] + 6660.0) * mm2cm;
        geoparameters.m_widthX = dx * 2.0;
        geoparameters.m_widthY = dy * 2.0;
        geoparameters.m_widthZ = dz * 2.0;
        // ATTN: parameters past here taken from uboone
        geoparameters.m_larTPCVolumeId = 0;
        geoparameters.m_wirePitchU = 0.300000011921;
        geoparameters.m_wirePitchV = 0.300000011921;
        geoparameters.m_wirePitchW = 0.300000011921;
        geoparameters.m_wireAngleU = 1.04719758034;
        geoparameters.m_wireAngleV = -1.04719758034;
        geoparameters.m_wireAngleW = 0.0;
        geoparameters.m_sigmaUVW = 1;
        geoparameters.m_isDriftInPositiveX = 0;
    }
    catch (const pandora::StatusCodeException &)
    {
        std::cout << "CreatePandoraLArTPCs - invalid tpc parameter provided" << std::endl;
    }

    try
    {
        PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::Geometry::LArTPC::Create(*pPrimaryPandora, geoparameters));
    }
    catch (const pandora::StatusCodeException &)
    {
        std::cout << "CreatePandoraLArTPCs - unable to create tpc, insufficient or "
                     "invalid information supplied"
                  << std::endl;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessEvents(const Parameters &parameters, const Pandora *const pPrimaryPandora)
{
    int nEvents(0);

    TFile fileSource(parameters.m_inputFileName.c_str(), "READ");
    TTree *pEDepSimTree = (TTree *)fileSource.Get("EDepSimEvents");

    if (!pEDepSimTree)
    {
        std::cout << "Missing the event tree" << std::endl;
        return;
    }

    TG4Event *pEDepSimEvent(nullptr);
    pEDepSimTree->SetBranchAddress("Event", &pEDepSimEvent);

    // Factory for creating LArCaloHits
    lar_content::LArCaloHitFactory m_larCaloHitFactory;

    while ((nEvents < parameters.m_nEventsToProcess) || (0 > parameters.m_nEventsToProcess))
    {
        if (parameters.m_shouldDisplayEventNumber)
            std::cout << std::endl << "   PROCESSING EVENT: " << nEvents << std::endl << std::endl;

        pEDepSimTree->GetEntry(nEvents++);

        if (!pEDepSimEvent)
            return;

        int hitCounter(0);

        for (TG4HitSegmentDetectors::iterator detector = pEDepSimEvent->SegmentDetectors.begin();
             detector != pEDepSimEvent->SegmentDetectors.end(); ++detector)
        {
            std::cout << "Show hits for " << detector->first << " (" << detector->second.size() << " hits)" << std::endl;
            std::cout << "                                 " << std::endl;

            std::vector<LArVoxel> voxelList;
            const double voxelWidth_cm(0.4);

            for (TG4HitSegment &g4Hit : detector->second)
            {
                std::vector<LArVoxel> currentVoxelList = makeVoxels(g4Hit, voxelWidth_cm);

                for (LArVoxel &voxel : currentVoxelList)
                {
                    voxelList.push_back(voxel);
                }
            }

            std::cout << "Voxels produced: " << voxelList.size() << std::endl;

            // ATTN: Here we might need to add something to check if there are
            // multiple energy deposits from the same particle into one voxel. How can
            // we tell if they are from the same particle though? This should also
            // be rare I think

            // Loop over the voxels and make them into caloHits
            for (int i = 0; i < voxelList.size(); i++)
            {
                const LArVoxel voxel = voxelList[i];
                const CartesianVector voxelPos(voxel.m_voxelPosVect);
                const double voxelE = voxel.m_energyInVoxel;

                lar_content::LArCaloHitParameters caloHitParameters;
                caloHitParameters.m_positionVector = voxelPos;
                caloHitParameters.m_expectedDirection = pandora::CartesianVector(0.f, 0.f, 1.f);
                caloHitParameters.m_cellNormalVector = pandora::CartesianVector(0.f, 0.f, 1.f);
                caloHitParameters.m_cellGeometry = pandora::RECTANGULAR;
                caloHitParameters.m_cellSize0 = voxelWidth_cm;
                caloHitParameters.m_cellSize1 = voxelWidth_cm;
                caloHitParameters.m_cellThickness = voxelWidth_cm;
                caloHitParameters.m_nCellRadiationLengths = 1.f;
                caloHitParameters.m_nCellInteractionLengths = 1.f;
                caloHitParameters.m_time = 0.f;
                caloHitParameters.m_inputEnergy = voxelE;
                caloHitParameters.m_mipEquivalentEnergy = voxelE;
                caloHitParameters.m_electromagneticEnergy = voxelE;
                caloHitParameters.m_hadronicEnergy = voxelE;
                caloHitParameters.m_isDigital = false;
                caloHitParameters.m_hitType = pandora::TPC_3D;
                caloHitParameters.m_hitRegion = pandora::SINGLE_REGION;
                caloHitParameters.m_layer = 0;
                caloHitParameters.m_isInOuterSamplingLayer = false;
                caloHitParameters.m_pParentAddress = (void *)(static_cast<uintptr_t>(++hitCounter));
                caloHitParameters.m_larTPCVolumeId = 0;
                caloHitParameters.m_daughterVolumeId = 0;

                if (parameters.m_use3D)
                {
                    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                        PandoraApi::CaloHit::Create(*pPrimaryPandora, caloHitParameters, m_larCaloHitFactory));
                }

                if (parameters.m_useLArTPC)
                {
                    // Create U, V and W views assuming x is the common drift coordinate
                    const double x0_cm = voxelPos.GetX();
                    const double y0_cm = voxelPos.GetY();
                    const double z0_cm = voxelPos.GetZ();

                    lar_content::LArCaloHitParameters caloHitPars_UView(caloHitParameters);
                    caloHitPars_UView.m_hitType = pandora::TPC_VIEW_U;
                    const double upos_cm(pPrimaryPandora->GetPlugins()->GetLArTransformationPlugin()->YZtoU(y0_cm, z0_cm));
                    caloHitPars_UView.m_positionVector = CartesianVector(x0_cm, 0.0, upos_cm);

                    lar_content::LArCaloHitParameters caloHitPars_VView(caloHitParameters);
                    caloHitPars_VView.m_hitType = pandora::TPC_VIEW_V;
                    const double vpos_cm(pPrimaryPandora->GetPlugins()->GetLArTransformationPlugin()->YZtoV(y0_cm, z0_cm));
                    caloHitPars_VView.m_positionVector = CartesianVector(x0_cm, 0.0, vpos_cm);

                    lar_content::LArCaloHitParameters caloHitPars_WView(caloHitParameters);
                    caloHitPars_WView.m_hitType = pandora::TPC_VIEW_W;
                    const double wpos_cm(pPrimaryPandora->GetPlugins()->GetLArTransformationPlugin()->YZtoW(y0_cm, z0_cm));
                    caloHitPars_WView.m_positionVector = CartesianVector(x0_cm, 0.0, wpos_cm);

                    // Create LArCaloHits for U, V and W views
                    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                        PandoraApi::CaloHit::Create(*pPrimaryPandora, caloHitPars_UView, m_larCaloHitFactory));
                    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                        PandoraApi::CaloHit::Create(*pPrimaryPandora, caloHitPars_VView, m_larCaloHitFactory));
                    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                        PandoraApi::CaloHit::Create(*pPrimaryPandora, caloHitPars_WView, m_larCaloHitFactory));
                }

            } // end voxel loop

            // The voxelisation only works with ArgonCube
            break;

        } // end segment detector loop

        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(*pPrimaryPandora));
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*pPrimaryPandora));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
std::vector<LArVoxel> makeVoxels(const TG4HitSegment &g4Hit, double voxelWidth_cm)
{

    std::vector<LArVoxel> currentVoxelList;

    // Set the variables for the AV bounding box and voxel size
    const double voxelSize[3] = {voxelWidth_cm, voxelWidth_cm, voxelWidth_cm};
    const double boxTop[3] = {370.0, 160.0, 930.0};
    const double boxBottom[3] = {-370.0, -160.0, 400.0};
    double boxLength[3] = {0.0, 0.0, 0.0};
    for (int i = 0; i < 3; i++)
    {
        boxLength[i] = boxTop[i] - boxBottom[i];
    }
    const double xnum = boxLength[0] / voxelSize[0];
    const double ynum = boxLength[1] / voxelSize[1];
    const double znum = boxLength[2] / voxelSize[2];

    const double epsilon(1.0e-3);
    double energy_deposit(0.0);

    // Get start and stop points for what we want to voxelise
    // mm to cm conversion
    const double mm2cm(0.1);
    const TLorentzVector &hitStart = g4Hit.GetStart() * mm2cm;
    const TLorentzVector &hitStop = g4Hit.GetStop() * mm2cm;

    const CartesianVector start(hitStart.X(), hitStart.Y(), hitStart.Z());
    const CartesianVector stop(hitStop.X(), hitStop.Y(), hitStop.Z());

    std::cout<<"Hit start: " << start << std::endl;
    std::cout<<"Hit stop: "<< stop << std::endl;
    std::cout<<"Hit displ: " << start-stop <<std::endl;

    // Eliminate any tracks which are just points
    if ((stop - start).GetMagnitude() < 1e-10)
    {
        std::cout << "Cannot have zero track length." << std::endl;
        std::cout << "                              " << std::endl;
        return currentVoxelList;
    }

    // Vectors for the intersection points of the hit and the test box
    CartesianVector pt0(0, 0, 0);
    CartesianVector pt1(0, 0, 0);

    // Find intersections (check hit contained within AV)
    const int crossings = Intersections(boxBottom, boxTop, start, stop, pt0, pt1);

    if (crossings == 0)
    {
        std::cout << "No crossing point found..." << std::endl;
        // ATTN: In ML they return voxel list here if no crossing points
        // found. Think I'm doing this a slightly different way, so maybe
        // remove
    }

    std::cout << "Crossings = " << crossings << std::endl;
    std::cout << "   Intersects with bounding box at"
              << " pt0 = (" << pt0.GetX() << "," << pt0.GetY() << "," << pt0.GetZ() << ")"
              << " and pt1 = (" << pt1.GetX() << "," << pt1.GetY() << "," << pt1.GetZ() << ")" << std::endl;

    // Get a unit vector in the direction of the hit segment
    const CartesianVector dir = pt1 - pt0;
    const double length = dir.GetMagnitude();
    const CartesianVector dirNorm = dir.GetUnitVector();

    // Need this to check the inverse vector doesn't end up with a divide by 0
    float val1, val2, val3;
    if (dir.GetX() != 0)
    {
        val1 = 1.0 / dir.GetX();
    }
    else
    {
        val1 = std::numeric_limits<float>::max();
    }

    if (dir.GetY() != 0)
    {
        val2 = 1.0 / dir.GetY();
    }
    else
    {
        val2 = std::numeric_limits<float>::max();
    }

    if (dir.GetZ() != 0)
    {
        val3 = 1.0 / dir.GetZ();
    }
    else
    {
        val3 = std::numeric_limits<float>::max();
    }

    const CartesianVector invDirNorm(val1, val2, val3);
    int sign[3];
    sign[0] = (invDirNorm.GetX() < 0);
    sign[1] = (invDirNorm.GetY() < 0);
    sign[2] = (invDirNorm.GetZ() < 0);

    double t0(0.0);
    double t1(0.0);
    // size_t nx(0), ny(0), nz(0);

    // Shuffle along this hit segment
    while (true)
    {
        // Get the position vector that we are going to check out
        CartesianVector pt = pt0 + dirNorm * (t1 + epsilon);
        std::cout << "    New point: " << pt << std::endl;

        // Find which voxel this lives in
        //-----voxel Iding------------
        if (pt.GetX() > boxTop[0] || pt.GetX() < boxBottom[0] || pt.GetY() > boxTop[1] || pt.GetY() < boxBottom[1] ||
            pt.GetZ() > boxTop[2] || pt.GetZ() < boxBottom[2])
        {
            std::cout << "Invalid voxel! Out of Geometry!" << std::endl;
            std::cout << "                               " << std::endl;
            return currentVoxelList;
        }

        double xindex = (pt.GetX() - boxBottom[0]) / voxelSize[0];
        double yindex = (pt.GetY() - boxBottom[1]) / voxelSize[1];
        double zindex = (pt.GetZ() - boxBottom[2]) / voxelSize[2];

        if (xindex == xnum)
            xindex -= 1;
        if (yindex == ynum)
            yindex -= 1;
        if (zindex == znum)
            zindex -= 1;

        int voxelID = int(zindex * (xnum * ynum) + yindex * xnum + xindex);
        //---------------------------

        // ATTN: This wasn't working for x or y, but was for z. I couldn't
        // work out why, but seemed to work okay if I just used the indexes
        // straight out
        //--------------id_to_xyz_index---------------
        /*
	  nz = (double)voxelID / (xnum * ynum);
	  std::cout << "voxelID before = " << voxelID <<std::endl;
	  std::cout << "nz " << nz << std::endl;
	  voxelID -= nz * (xnum * ynum);
	  std::cout << "voxelID after = " << voxelID << std::endl;
	  ny = (double)voxelID / xnum;
	  nx = ((double)voxelID - ny * xnum);
	  
	  std::cout << "xindex : " << xindex << "  yindex : " << yindex << "    zindex
	  : " << zindex << std::endl; std::cout << "nx : " << nx << "  ny : " << ny <<
	  "    nz : " << nz << std::endl;
	*/
        //-----------------------------------------------------------------------
        // move the box bounds such that they are moved a voxel along
        // z in good....x and y aren't....but can just use index straight here
        double boxBottomUpdate[3] = {0, 0, 0};
        double boxTopUpdate[3] = {0, 0, 0};

        // boxBottomUpdate[0] = boxBottom[0] + (nx * voxelSize[0] - 1e-3);
        // boxBottomUpdate[1] = boxBottom[1] + (ny * voxelSize[1] - 1e-3);
        // boxBottomUpdate[2] = boxBottom[2] + (nz * voxelSize[2] - 1e-3);

        // Define an updated test box
        boxBottomUpdate[0] = boxBottom[0] + (xindex * voxelSize[0] - 1e-3);
        boxBottomUpdate[1] = boxBottom[1] + (yindex * voxelSize[1] - 1e-3);
        boxBottomUpdate[2] = boxBottom[2] + (zindex * voxelSize[2] - 1e-3);
        boxTopUpdate[0] = boxBottomUpdate[0] + voxelSize[0];
        boxTopUpdate[1] = boxBottomUpdate[1] + voxelSize[1];
        boxTopUpdate[2] = boxBottomUpdate[2] + voxelSize[2];

	std::cout<<"Updated box: bottom = ("<<boxBottomUpdate[0]<<", "<<boxBottomUpdate[1]<<", "
		 <<boxBottomUpdate[2]<<"), top = ("<<boxTopUpdate[0]<<", "<<boxTopUpdate[1]
		 <<", "<<boxTopUpdate[2]<<")"<<std::endl;

        std::cout << "    Inspecting a voxel id " << voxelID << " ... " << std::endl;

        double t1before = t1;
        int cross = BoxCrossings(boxBottomUpdate, boxTopUpdate, pt0, sign, invDirNorm, t0, t1);
        double t1after = t1;
	std::cout<<"t1: before = "<<t1before<<", after = "<<t1after<<std::endl;

        // ATTN: This is here so that it can not get stuck in an infinite loop
        // where t1 doesn't shuffle along. Probably should think of a better
        // way to do this
        if (t1before == t1after)
            return currentVoxelList;

        // Consider crossings with the test box
        if (cross == 0)
        {
            // Test box should have been set up to contain a section of this hit
            std::cout << "      No crossing (not expected) ... breaking" << std::endl;
            return currentVoxelList;
        }

        double dx(0.0);
        if (cross == 1)
        {
            std::cout << "      One crossing: " << pt0 + dir * t1 << std::endl;
            dx = std::min(t1, length);
        }
        else
        {
            std::cout << "      Two crossing" << pt0 + dir * t0 << " => " << pt0 + dir * t1 << std::endl;
            if (t0 > length)
                dx = length;
            else if (t1 > length)
                dx = length - t0;
            else
                dx = t1 - t0;
        }

        // Find the fraction of energy contained in voxel from the fraction of
        // track in voxel
        double energyInVoxel = g4Hit.GetEnergyDeposit() * dx / length;

        if (energyInVoxel < 0)
        {
            std::cout << "Voxel with negative energy deposited!" << std::endl
                      << "  ID = " << voxelID << std::endl
                      << "  edep computed from:" << std::endl
                      << "      dx = " << dx << ", length = " << length << ", TG4HitSegment edep = " << g4Hit.GetEnergyDeposit() << std::endl;
            throw StatusCodeException(STATUS_CODE_FAILURE);
        }

        energy_deposit += energyInVoxel;

        std::cout << "      Registering voxel id " << voxelID << " t1 =" << t1 << " (total length = " << length << ")" << std::endl;

        // Push back voxels back into a list
        const CartesianVector voxelPosVect(boxBottomUpdate[0], boxBottomUpdate[1], boxBottomUpdate[2]);
	std::cout << "Adding voxel with pos = " << voxelPosVect << " and E = " << energyInVoxel << std::endl;
        LArVoxel currentVoxel(voxelID, energyInVoxel, voxelPosVect);
        currentVoxelList.push_back(currentVoxel);

        // Once t1 is longer than the voxel, break out of the loop
        if (t1 > length)
        {
            std::cout << "      Reached the segment end (t1 = " << t1 << " fractional length " << t1 / length << ") ... breaking" << std::endl;
            std::cout << "                      " << std::endl;
            return currentVoxelList;
        }

        std::cout << "      Updated t1 = " << t1 << " (fractional length " << t1 / length << ")" << std::endl;
        std::cout << "                      " << std::endl;

    } // end while true

    std::cout << "current num of voxels: " << currentVoxelList.size() << std::endl;
    std::cout << "                      " << std::endl;

    return currentVoxelList;
}

//------------------------------------------------------------------------------------------------------------------------------------------
int Intersections(const double *const boxBottom, const double *const boxTop, const CartesianVector &start, const CartesianVector &stop,
    CartesianVector &pt0, CartesianVector &pt1)
{
    const CartesianVector displVec = (stop - start);
    const CartesianVector dir = displVec.GetUnitVector();
    float val1, val2, val3;
    if (dir.GetX() != 0)
    {
        val1 = 1.0 / dir.GetX();
    }
    else
    {
        val1 = std::numeric_limits<float>::max();
    }

    if (dir.GetY() != 0)
    {
        val2 = 1.0 / dir.GetY();
    }
    else
    {
        val2 = std::numeric_limits<float>::max();
    }

    if (dir.GetZ() != 0)
    {
        val3 = 1.0 / dir.GetZ();
    }
    else
    {
        val3 = std::numeric_limits<float>::max();
    }

    const CartesianVector invdir(val1, val2, val3);
    int sign[3];
    sign[0] = (invdir.GetX() < 0);
    sign[1] = (invdir.GetY() < 0);
    sign[2] = (invdir.GetZ() < 0);

    // Consider if start and stop points are contained
    const bool startContained = (start.GetX() > boxBottom[0] && start.GetX() < boxTop[0]) &&
                                (start.GetY() > boxBottom[1] && start.GetY() < boxTop[1]) &&
                                (start.GetZ() > boxBottom[2] && start.GetZ() < boxTop[2]);
    const bool stopContained = (stop.GetX() > boxBottom[0] && stop.GetX() < boxTop[0]) &&
                               (stop.GetY() > boxBottom[1] && stop.GetY() < boxTop[1]) && (stop.GetZ() > boxBottom[2] && stop.GetZ() < boxTop[2]);

    // If the start and stop points are contained, we know entry/exit points
    if (startContained)
    {
        pt0 = start;
    }
    if (stopContained)
    {
        pt1 = stop;
    }

    if (!startContained || !stopContained)
    {
        double t0, t1;

        int cross = BoxCrossings(boxBottom, boxTop, start, sign, invdir, t0, t1);

        if (cross > 0)
        {
            if ((!startContained && t0 < 0) || t0 > displVec.GetMagnitude())
                cross--;
            if (t1 < 0 || t1 > displVec.GetMagnitude())
                cross--;
        }

        if (cross > 0)
        {
            const float epsilon = 0.0001;
            if (!startContained)
                pt0 = start + (dir * (t0 + epsilon));

            if (!stopContained)
                pt1 = start + (dir * (t1 - epsilon));
        }

        std::cout << "Number of crossings=" << cross << " for bounding box " << boxBottom << "-" << boxTop << " and ray between "
                  << "(" << start.GetX() << "," << start.GetY() << "," << start.GetZ() << ")"
                  << " and (" << stop.GetX() << "," << stop.GetY() << "," << stop.GetZ() << ")" << std::endl;

        if (cross > 0)
        {
            std::cout << "Start point contained?: " << startContained << std::endl;
            if (!startContained)
                std::cout << "  entry point: " << pt0 << "; t0=" << t0 << std::endl;
            std::cout << "Stop point contained?: " << stopContained << std::endl;
            if (!stopContained)
                std::cout << "  exit point: " << pt1 << "; t1=" << t1 << std::endl;
        }

        if (cross == 1 && startContained == stopContained)
        {
            std::cout << "Unexpected number of crossings (" << cross << ")"
                      << " for bounding box and ray between "
                      << "(" << start.GetX() << "," << start.GetY() << "," << start.GetZ() << ")"
                      << " and (" << stop.GetX() << "," << stop.GetY() << "," << stop.GetZ() << ")" << std::endl;
            std::cout << "Start point contained?: " << startContained << ".  Stop point contained?: " << stopContained << std::endl;
        }

        return cross;
    }

    return 2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
int BoxCrossings(const double *const boxBottom, const double *const boxTop, const CartesianVector &start, int *const sign,
    const CartesianVector &invdir, double &t0, double &t1)
{

    float tmin(0), tmax(0), tymin(0), tymax(0), tzmin(0), tzmax(0);

    if (sign[0] == 0)
    {
        tmin = (boxBottom[0] - start.GetX()) * invdir.GetX();
        tmax = (boxTop[0] - start.GetX()) * invdir.GetX();
    }
    else if (sign[0] == 1)
    {
        tmin = (boxTop[0] - start.GetX()) * invdir.GetX();
        tmax = (boxBottom[0] - start.GetX()) * invdir.GetX();
    }

    if (sign[1] == 0)
    {
        tymin = (boxBottom[1] - start.GetY()) * invdir.GetY();
        tymax = (boxTop[1] - start.GetY()) * invdir.GetY();
    }
    else if (sign[1] == 1)
    {
        tymin = (boxTop[1] - start.GetY()) * invdir.GetY();
        tymax = (boxBottom[1] - start.GetY()) * invdir.GetY();
    }

    if ((tmin > tymax) || (tymin > tmax))
        return 0;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    if (sign[2] == 0)
    {
        tzmin = (boxBottom[2] - start.GetZ()) * invdir.GetZ();
        tzmax = (boxTop[2] - start.GetZ()) * invdir.GetZ();
    }
    else if (sign[2] == 1)
    {
        tzmin = (boxTop[2] - start.GetZ()) * invdir.GetZ();
        tzmax = (boxBottom[2] - start.GetZ()) * invdir.GetZ();
    }

    if ((tmin > tzmax) || (tzmin > tmax))
        return 0;

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    t0 = tmin;
    t1 = tmax;

    if (t1 <= 0)
        return 0;
    if (t0 <= 0)
        return 1;
    return 2;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ParseCommandLine(int argc, char *argv[], Parameters &parameters)
{
    if (1 == argc)
        return PrintOptions();

    int c(0);

    std::string viewOption("3d");

    while ((c = getopt(argc, argv, ":i:e:n:s:j:Nh")) != -1)
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
            case 'j':
                viewOption = optarg;
                break;
            case 'N':
                parameters.m_shouldDisplayEventNumber = true;
                break;
            case 'h':
            default:
                return PrintOptions();
        }
    }

    ProcessViewOption(viewOption, parameters);
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PrintOptions()
{
    std::cout << std::endl
              << "./bin/PandoraInterface " << std::endl
              << "    -i Settings            (required) [algorithm description: xml]" << std::endl
              << "    -e EventsFile          (required) [input edep-sim file, "
                 "typically containing events and geometry]"
              << std::endl
              << "    -n NEventsToProcess    (optional) [no. of events to process]" << std::endl
              << "    -s NEventsToSkip       (optional) [no. of events to skip in "
                 "first file]"
              << std::endl
              << "    -p                     (optional) [print status]" << std::endl
              << "    -N                     (optional) [print event numbers]" << std::endl
              << "    -j Projection          (optional) [3D (default), LArTPC, Both]" << std::endl
              << std::endl;

    return false;
}

void ProcessViewOption(const std::string &viewOption, Parameters &parameters)
{
    std::string chosenViewOption(viewOption);
    std::transform(chosenViewOption.begin(), chosenViewOption.end(), chosenViewOption.begin(), ::tolower);

    if (chosenViewOption == "3d")
    {
        // 3D hits only
        std::cout << "Using 3D hits" << std::endl;
        parameters.m_useLArTPC = false;
        parameters.m_use3D = true;
    }
    else if (chosenViewOption == "both")
    {
        // LArTPC and 3D hits
        std::cout << "Using LArTPC projections _and_ 3D hits" << std::endl;
        parameters.m_useLArTPC = true;
        parameters.m_use3D = true;
    }
    else
    {
        // LArTPC hits only
        std::cout << "Using LArTPC projections" << std::endl;
        parameters.m_useLArTPC = true;
        parameters.m_use3D = false;
    }
}

} // namespace lar_nd_reco
