
#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArHelpers/LArPointingClusterHelper.h"
#include "larpandoracontent/LArObjects/LArPointingCluster.h"

#include "larpandoracontent/LArMonitoring/NeutrinoEventValidationAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArObjects/LArTwoDSlidingFitResult.h"

#include "ParticleHierarchyValidationAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArInteractionTypeHelper.h"
#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"

#include "larpandoracontent/LArMonitoring/EventValidationBaseAlgorithm.h"
#include "larpandoracontent/LArMonitoring/TestBeamEventValidationAlgorithm.h"

using namespace pandora;
using namespace std;

namespace development_area
{ 
    ParticleHierarchyValidationAlgorithm::ParticleHierarchyValidationAlgorithm():
        m_writeToTree(false),
        m_treeName("ElectronEventTree"),
        m_fileName("ElectronTree.root"),
        eventNo(0),
        addAll(false),   //In future, add this as an optional in the xml (ReadSettings), this just decides whether or not to add all particles to the relevance list or just the ones that meet criteria
        m_showAllPfoData(false),
        m_showAllMCPData(false)
    {
    }
    ParticleHierarchyValidationAlgorithm::~ParticleHierarchyValidationAlgorithm()
    {
        if (m_writeToTree)
        {
            PandoraMonitoringApi::SaveTree(this->GetPandora(), m_treeName.c_str(), m_fileName.c_str(), "UPDATE");
        }
        
    }

    StatusCode ParticleHierarchyValidationAlgorithm::Run(){
        
        //std::cout << "1" << std::endl;
        
        //SET THE EVE DISPLAY PARAMETERS
        PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1, 1, 1));
    
        m_primaryParameters.m_minPrimaryGoodHits = 15 /*15*/;
        m_primaryParameters.m_minHitsForGoodView = 5 /*5*/;
        m_primaryParameters.m_minPrimaryGoodViews = 2;
        m_primaryParameters.m_selectInputHits = false; /*true*/
        m_primaryParameters.m_maxPhotonPropagation = 2.5f;
        m_primaryParameters.m_minHitSharingFraction = 0.9f /*0.9f*/;
        m_primaryParameters.m_foldBackHierarchy = false;
    
            //For the root tree to know what event pfos and mcps came from
        eventNo+=1;
        std::cout << "<--Event Number " << eventNo << "-->" << std::endl;
        
        
                    //#---Defining and collecting all of the lists to do with the event---#
                    
            //(pointer to) List of PFOs (might need name)
        const PfoList *Pfos(nullptr);
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "RecreatedPfos" /*m_inputPfoListName*/, Pfos));
        
            //(pointer to) List of MCParticles (might need name)
        const MCParticleList *MCParts(nullptr);
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "Input" /*m_inputMCParticleListName*/, MCParts));


            //CaloHitLists
        const CaloHitList *CaloHitsU(nullptr);
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "CaloHitListU", CaloHitsU));
        
        const CaloHitList *CaloHitsV(nullptr);
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "CaloHitListV", CaloHitsV));
        
        const CaloHitList *CaloHitsW(nullptr);
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "CaloHitListW", CaloHitsW));
        
        const CaloHitList *CaloHits(nullptr);
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "CaloHitList2D", CaloHits));
        
        
        //std::cout << "2" << std::endl;
        
        bool MCElectronFound  = false;
        bool PfoElectronFound = false;
        
            //At this point, relevance is decided by "is it either electron flavour or muon flavoured (but not neutrinos)"
        PfoList relevantPfos;
        getRelevantPfos(Pfos, relevantPfos, addAll, PfoElectronFound);
        MCParticleList relevantMCParts;
        getRelevantMCParts(MCParts, relevantMCParts, addAll, MCElectronFound);
          
        /*                            
        if (MCElectronFound && !PfoElectronFound)
        {
            PANDORA_MONITORING_API(VisualizeParticleFlowObjects(this->GetPandora(), Pfos, "All Pfos", RED, true, true));
            PANDORA_MONITORING_API(VisualizeMCParticles(this->GetPandora(), MCParts, "All MCParticles", BLUE));
            PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
        }   
        */  

            //Finding the number of hits in each PFO
        MCParticleList matchedMCParticles;
        
        int PfoTempId(0);
        int matched(0);
        float c_PfoCompleteness(-1);
        float c_PfoPurity(-1);
        
        for (const ParticleFlowObject *const c_Pfo : relevantPfos)
        {
            PfoTempId++;  
            
                //Looking at the purity and completeness of c_Pfo 
            const MCParticle* bestMatch(nullptr);
            std::vector<float> CandP = ParticleHierarchyValidationAlgorithm::purityAndCompleteness(c_Pfo, MCParts, CaloHits, m_primaryParameters);
            bestMatch = findBestMatch(c_Pfo, MCParts, CaloHits, m_primaryParameters);
            c_PfoCompleteness = CandP[0];
            c_PfoPurity       = CandP[1];
            
            //std::cout << "Purity      : " << c_PfoPurity << std::endl;
            //std::cout << "Completeness: " << c_PfoCompleteness << std::endl;
            //std::cout << "" << std::endl;
            //std::cout << "bestMatchId: " << abs(bestMatch->GetParticleId()) << endl;
            //std::cout << "PfoId      : " << abs(c_Pfo->GetParticleId()) << endl;
            //std::cout << "" << std::endl;
            
            
                    //by this point, we have found the relevant MCParticle and Pfo, so can find all the information to fill the table with
                    //Lets now find the angle between the Muon and Michel Electron
                //Origin and endpoint of the Electron
            CartesianVector* resultant = new CartesianVector(0,0,0);
            if (abs(bestMatch->GetParticleId()) == 11)
            {    
                const CartesianVector origin   = bestMatch->GetVertex();
                const CartesianVector endpoint = bestMatch->GetEndpoint();
                PANDORA_MONITORING_API(AddMarkerToVisualization(this->GetPandora(), &origin, "Electron Start", RED, 1));
                PANDORA_MONITORING_API(AddMarkerToVisualization(this->GetPandora(), &endpoint, "Electron End", RED, 1));
                resultant = new CartesianVector(endpoint.GetX() - origin.GetY(), endpoint.GetY() - origin.GetX(), endpoint.GetZ() - origin.GetZ());
            }
            std::cout << resultant->GetX() << std::endl;
            
            const MCParticleList parentList = bestMatch->GetParentList();
            const MCParticle* parent(nullptr);
            for (const MCParticle* const c_parent : parentList)
            {
                std::cout << "PDG:" << abs(c_parent->GetParticleId()) << std::endl;
                if (abs(c_parent->GetParticleId()) == 13)
                {
                    parent = c_parent;
                }
            }
            std::cout << "parent:" << abs(parent->GetParticleId()) << std::endl;
            std::cout << "" << std::endl;
            
            CartesianVector* parentResultant = new CartesianVector(0,0,0);
            const CartesianVector origin   = parent->GetVertex();
            const CartesianVector endpoint = parent->GetEndpoint();
            PANDORA_MONITORING_API(AddMarkerToVisualization(this->GetPandora(), &origin, "Muon Start", BLUE, 1));
            PANDORA_MONITORING_API(AddMarkerToVisualization(this->GetPandora(), &endpoint, "Muon End", BLUE, 1));
            
            parentResultant = new CartesianVector(endpoint.GetX() - origin.GetY(), endpoint.GetY() - origin.GetX(), endpoint.GetZ() - origin.GetZ());
            std::cout << parentResultant->GetX() << std::endl;
            
            PANDORA_MONITORING_API(VisualizeParticleFlowObjects(this->GetPandora(), Pfos, "All Pfos", RED, true, true));
            PANDORA_MONITORING_API(VisualizeMCParticles(this->GetPandora(), MCParts, "All MCParticles", BLUE));
            
            PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
            
            
            
            
            
            
            
            
            int c_PfoPDG = abs(bestMatch->GetParticleId());     //Give the PDG Code as the code of the best macthing MCParticle, not the Pfo itself
            
            int NUHits(0);
            int NVHits(0);
            int NWHits(0);
            getMCParticleViewHits(bestMatch, NUHits, NVHits, NWHits, m_showAllMCPData, MCParts, CaloHits);
            //getViewHits(c_Pfo, NUHits, NVHits, NWHits, m_showAllPfoData);
            
            matchedMCParticles.push_back(bestMatch);
            
            //std::cout << "5" << std::endl;
            
            if (m_writeToTree && c_PfoCompleteness >= 0.1 && c_PfoPurity >= 0.5 && (NUHits >= 0 || NVHits >= 0 || NWHits >= 0)){
                matched = 1;
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "eventNo", eventNo);
                //PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "PfoTempId", PfoTempId);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "MCPPDG", c_PfoPDG);  //PDG Code is note the be-all-end-all, need to also clarify if it has been given wrong code
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "Matched", matched);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "UHits", NUHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "VHits", NVHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "WHits", NWHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "Completeness", c_PfoCompleteness);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "Purity", c_PfoPurity);
                PandoraMonitoringApi::FillTree(this->GetPandora(), m_treeName.c_str());
                
                std::cout << "#  >>>PFO added to tree" << std::endl;
            }else if (!(c_PfoCompleteness >= 0.1 && c_PfoPurity >= 0.5 && (NUHits >= 0 || NVHits >= 0 || NWHits >= 0))){
                std::cout << "Particle Rejected: " << std::endl;
                std::cout << "  C> " << c_PfoCompleteness << std::endl;
                std::cout << "  P> " << c_PfoPurity << std::endl;
                std::cout << "PDG> " << c_Pfo->GetParticleId() << std::endl;
                std::cout << "     " << std::endl;
                //PANDORA_MONITORING_API(VisualizeParticleFlowObjects(this->GetPandora(), Pfos, "Problematic Pfos", RED, true, true));
                //PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
            }
        }
        
        for (const MCParticle *const c_MCPart : *MCParts)
        {
            int NUHits(0);
            int NVHits(0);
            int NWHits(0);
            getMCParticleViewHits(c_MCPart, NUHits, NVHits, NWHits, m_showAllMCPData, MCParts, CaloHits);
            
            int MCPId = abs(c_MCPart->GetParticleId());
            
            bool found(false);
            for (const MCParticle *const c_matchedMCPart : matchedMCParticles)
            {
                if (c_matchedMCPart == c_MCPart)
                {
                    found = true;
                    break;
                }
            }
            if (m_writeToTree && !found)
            {
                PfoTempId         = -1;
                matched           =  0;
                c_PfoCompleteness = -1;
                c_PfoPurity       = -1;
                std::cout << "  >Missed MCParticle: " << MCPId << std::endl;
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "eventNo", eventNo);
                //PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "PfoTempId", PfoTempId);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "MCPPDG", MCPId);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "Matched", matched);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "UHits", NUHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "VHits", NVHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "WHits", NWHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "Completeness", c_PfoCompleteness);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "Purity", c_PfoPurity);
                PandoraMonitoringApi::FillTree(this->GetPandora(), m_treeName.c_str());
            }

        }
        
        //std::cout << "6" << std::endl;
        
        return STATUS_CODE_SUCCESS;
    }
    
    
    LArMCParticleHelper::PrimaryParameters ParticleHierarchyValidationAlgorithm::CreatePrimaryParameters(){
        LArMCParticleHelper::PrimaryParameters primaryParameters = LArMCParticleHelper::PrimaryParameters();
        primaryParameters.m_minPrimaryGoodHits = 15;
        primaryParameters.m_minHitsForGoodView = 5;
        primaryParameters.m_minPrimaryGoodViews = 2;
        primaryParameters.m_selectInputHits = true;
        primaryParameters.m_maxPhotonPropagation = 2.5f;
        primaryParameters.m_minHitSharingFraction = 0.9f;
        primaryParameters.m_foldBackHierarchy = true;
        return primaryParameters;
    }
    
    
    void ParticleHierarchyValidationAlgorithm::getPurityAndCompleteness(const PfoList *const pPfoList, const MCParticleList *const pMCParts, const CaloHitList *const pCaloHits, std::vector<std::vector<float>> &pfoPurityCompleteness, LArMCParticleHelper::PrimaryParameters primaryParameters){
        for (const ParticleFlowObject *const pPfo : *pPfoList){
            std::vector<float> results = ParticleHierarchyValidationAlgorithm::purityAndCompleteness(pPfo, pMCParts, pCaloHits, primaryParameters);
            pfoPurityCompleteness.push_back(results);
            //std::cout << "Added to the pfoPurityCompleteness Object" << std::endl; 
        }
    }
    
    
    void ParticleHierarchyValidationAlgorithm::getRelevantPfos(const PfoList *const Pfos, PfoList &relevantPfos, const bool f_addAll, bool &PfoElectronFound){
        for (const ParticleFlowObject *c_pfo : *Pfos)
        {
            if (!f_addAll){
                if (abs(c_pfo->GetParticleId()) == 11){
                    //std::cout << "Found a Reco Electron" << std::endl;
                    relevantPfos.push_back(c_pfo);
                    std::cout << "Added Electron to 'relevantPfos'" << std::endl;
                    PfoElectronFound = true;
                }
                else if (abs(c_pfo->GetParticleId()) == 12){
                    //std::cout << "Found a Reco Electron-Neutrino" << std::endl;
                }
                else if (abs(c_pfo->GetParticleId()) == 13){
                    //std::cout << "Found a Reco Muon" << std::endl;
                    //relevantPfos.push_back(c_pfo);
                    //std::cout << "Added Muon to 'relevantPfos'" << std::endl;
                }
                else if (abs(c_pfo->GetParticleId()) == 14){
                    //std::cout << "Found a Reco Muon-Neutrino" << std::endl;
                }else{
                    //std::cout << "Found PFO, Id: " << c_pfo->GetParticleId() << std::endl;
                }
                    
            }else{    //Add All Particles, regardless of flavour
                //std::cout << "Found PFO, Id: " << c_pfo->GetParticleId() << std::endl;
                relevantPfos.push_back(c_pfo);
            }
        }
    }
    
    void ParticleHierarchyValidationAlgorithm::getRelevantMCParts(const MCParticleList *const MCParts, MCParticleList &relevantMCParts, const bool f_addAll, bool &MCElectronFound){
        for (const MCParticle *c_mcp : *MCParts)
        {   
            if (!f_addAll){
                if (abs(c_mcp->GetParticleId()) == 11){
                    //std::cout << "Found a MC Electron" << std::endl;
                    relevantMCParts.push_back(c_mcp);
                    std::cout << "Added Electron to 'relevantMCParts'" << std::endl;
                    MCElectronFound = true;
                }
                else if (abs(c_mcp->GetParticleId()) == 12){
                    //std::cout << "Found a MC Electron-Neutrino" << std::endl;
                }
                else if (abs(c_mcp->GetParticleId()) == 13){
                    //std::cout << "Found a MC Muon" << std::endl;
                    //relevantMCParts.push_back(c_mcp);
                    //std::cout << "Added Muon to 'relevantMCParts'" << std::endl;
                }
                else if (abs(c_mcp->GetParticleId()) == 14){
                    //std::cout << "Found a MC Muon-Neutrino" << std::endl;
                }else{
                    //std::cout << "Found MCP, Id: " << c_mcp->GetParticleId() << std::endl;
                }
                    //Add All Particles
            }else{ 
                //std::cout << "Found MCP, Id: " << c_mcp->GetParticleId() << std::endl;
                relevantMCParts.push_back(c_mcp);
            }
            
        }
    }
    
    void ParticleHierarchyValidationAlgorithm::getViewHits(const ParticleFlowObject *const c_Pfo, int &NUHits, int &NVHits, int &NWHits, const bool f_showAllPfoData){
        ClusterList c_clusterList = c_Pfo->GetClusterList();
        if (f_showAllPfoData)
                std::cout << "   >" << "Number of clusters in PFO " << c_Pfo->GetParticleId() << ": " << c_clusterList.size() << std::endl; 
                
        for (const Cluster *const c_cluster : c_clusterList)
        {

            CaloHitList c_caloHitList;
            c_cluster->GetOrderedCaloHitList().FillCaloHitList(c_caloHitList);
            if (f_showAllPfoData)
                std::cout << "     >" << "Number of hits in this cluster: " << c_cluster->GetNCaloHits() << std::endl;
            
            for (const CaloHit *const c_hit : c_caloHitList)
            {
                
                switch(c_hit->GetHitType())
                {
                    case 4:
                        if (f_showAllPfoData)
                            std::cout << "         >" << "CaloHitView: U" << std::endl;
                        NUHits = NUHits+1;
                        break;
                    case 5:
                        if (f_showAllPfoData)
                            std::cout << "         >" << "CaloHitView: V" << std::endl;
                        NVHits = NVHits+1;
                        break;
                    case 6:
                        if (f_showAllPfoData)
                            std::cout << "         >" << "CaloHitView: W" << std::endl;
                        NWHits = NWHits+1;
                        break;
                    default:
                        break;
                } 
                     
            }
        }
    }
    
    
    
    //############################################
    void ParticleHierarchyValidationAlgorithm::getMCParticleViewHits(const MCParticle *const c_MCPart, int &NUHits, int &NVHits, int &NWHits, const bool f_showAllMCPData, const MCParticleList *MCParts, const CaloHitList *CaloHits){
                
        LArMCParticleHelper::MCRelationMap mcToSelfMap;
        LArMCParticleHelper::CaloHitToMCMap hitToMCMap;
        LArMCParticleHelper::MCContributionMap mcToTrueHitListMap;
        
        LArMCParticleHelper::GetMCToSelfMap(MCParts, mcToSelfMap);
        LArMCParticleHelper::GetMCParticleToCaloHitMatches(CaloHits, mcToSelfMap, hitToMCMap, mcToTrueHitListMap);
        
        for (std::pair c_pair : hitToMCMap)
        {
            if (c_pair.second == c_MCPart)
            {
                switch(c_pair.first->GetHitType())
                {
                    case 4:
                        if (f_showAllMCPData)
                            std::cout << "         >" << "(MCP) CaloHitView: U" << std::endl;
                        NUHits = NUHits+1;
                        break;
                    case 5:
                        if (f_showAllMCPData)
                            std::cout << "         >" << "(MCP) CaloHitView: V" << std::endl;
                        NVHits = NVHits+1;
                        break;
                    case 6:
                        if (f_showAllMCPData)
                            std::cout << "         >" << "(MCP) CaloHitView: W" << std::endl;
                        NWHits = NWHits+1;
                        break;
                    default:
                        break;
                } 
            }
        }
    }
    
    
    
    
    std::vector<float> ParticleHierarchyValidationAlgorithm::purityAndCompleteness(const ParticleFlowObject *const pPfo, const MCParticleList *const MCParts, const CaloHitList *const CaloHits, LArMCParticleHelper::PrimaryParameters &primaryParameters){
        const PfoList myPfoList(1, pPfo);
        
        //std::cout << "  --Entered for loop--" << std::endl;
            
        LArMCParticleHelper::MCContributionMap targetMCParticleToHitsMap;
        LArMCParticleHelper::SelectReconstructableMCParticles(MCParts, CaloHits, primaryParameters, LArMCParticleHelper::IsVisible, targetMCParticleToHitsMap);
            
        //std::cout << "*Size 1: " << MCParts->size() << std::endl;
        //std::cout << "*Size 2: " << CaloHits->size() << std::endl;
        //std::cout << "*Size 3: " << primaryParameters << std::endl;
        //std::cout << "*Size 4: " << LArMCParticleHelper::IsBeamNeutrinoFinalState << std::endl;
        //std::cout << "*Size 5: " << targetMCParticleToHitsMap.size() << std::endl;

        LArMCParticleHelper::MCContributionMapVector mcParticlesToGoodHitsMaps({targetMCParticleToHitsMap});

        LArMCParticleHelper::PfoContributionMap pfoToReconstructable2DHitsMap;
        LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(myPfoList, mcParticlesToGoodHitsMaps, pfoToReconstructable2DHitsMap, primaryParameters.m_foldBackHierarchy);

        LArMCParticleHelper::PfoToMCParticleHitSharingMap pfoToMCParticleHitSharingMap;
        LArMCParticleHelper::MCParticleToPfoHitSharingMap mcParticleToPfoHitSharingMap;
        LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToReconstructable2DHitsMap, mcParticlesToGoodHitsMaps, pfoToMCParticleHitSharingMap, mcParticleToPfoHitSharingMap);  //#!!!!!#

                        
        //std::cout << "  --Finished Calling LArMCParticleHelper functions--" << std::endl;            
            
        const CaloHitList &allHitsInPfo(pfoToReconstructable2DHitsMap.at(pPfo));
	    const int nHitsInPfoTotal(allHitsInPfo.size());
	    int nHitsInBestMCParticleTotal(-1);
	    int nHitsSharedWithBestMCParticleTotal(-1);
	        
	    //std::cout << "Size 1: " << pfoToReconstructable2DHitsMap.size() << std::endl;
        //std::cout << "Size 2: " << mcParticlesToGoodHitsMaps.size() << std::endl;
        //std::cout << "Size 3: " << pfoToMCParticleHitSharingMap.size() << std::endl;
        //std::cout << "Size 4: " << mcParticleToPfoHitSharingMap.size() << std::endl;
            
	    const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(pfoToMCParticleHitSharingMap.at(pPfo));
	    
	    //std::cout << "About to start the loop..." << std::endl;

	    for (const LArMCParticleHelper::MCParticleCaloHitListPair &mcParticleCaloHitListPair : mcParticleToSharedHitsVector)
	    {     
	        // std::cout << "      --In second for loop--" << std::endl;
	            
	        const pandora::MCParticle* pAssociatedMCParticle(mcParticleCaloHitListPair.first);  
	        const CaloHitList &allMCHits(targetMCParticleToHitsMap.at(pAssociatedMCParticle));
	        const CaloHitList &associatedMCHits(mcParticleCaloHitListPair.second);

	        if (static_cast<int>(associatedMCHits.size()) > nHitsSharedWithBestMCParticleTotal)
	        {
	            
	            //std::cout << "         --Entered if statement--" << std::endl;
	            
		        nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
		        nHitsInBestMCParticleTotal = allMCHits.size();  
	        }
	    }		

        //std::cout << "      --Calculating completeness and purity--" << std::endl;

        //std::cout << "nHitsSharedWithBestMCParticleTotal: " << nHitsSharedWithBestMCParticleTotal << std::endl;
        //std::cout << "nHitsInBestMCParticleTotal: " << nHitsInBestMCParticleTotal << std::endl;

        const float completeness((nHitsInBestMCParticleTotal > 0) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal) / static_cast<float>(nHitsInBestMCParticleTotal) : 0.f);
            
        //std::cout << "nHitsInPfoTotal: " << nHitsInPfoTotal << std::endl;
            
        const float purity((nHitsInPfoTotal > 0 ) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal) / static_cast<float>(nHitsInPfoTotal) : 0.f);
            
        //std::cout << "Completeness: " << completeness << std::endl;
        //std::cout << "Purity: " << purity << std::endl; 
        
        std::vector<float> return_vec = {completeness, purity};
        
        return return_vec;
    }
    
    
    
    const pandora::MCParticle* ParticleHierarchyValidationAlgorithm::findBestMatch(const ParticleFlowObject *const pPfo, const MCParticleList *const MCParts, const CaloHitList *const CaloHits, LArMCParticleHelper::PrimaryParameters &primaryParameters)
    {
        const PfoList myPfoList(1, pPfo);
        
        //std::cout << "  --Entered for loop--" << std::endl;
            
        LArMCParticleHelper::MCContributionMap targetMCParticleToHitsMap;
        LArMCParticleHelper::SelectReconstructableMCParticles(MCParts, CaloHits, primaryParameters, LArMCParticleHelper::IsVisible, targetMCParticleToHitsMap);
            
        //std::cout << "*Size 1: " << MCParts->size() << std::endl;
        //std::cout << "*Size 2: " << CaloHits->size() << std::endl;
        //std::cout << "*Size 3: " << primaryParameters << std::endl;
        //std::cout << "*Size 4: " << LArMCParticleHelper::IsBeamNeutrinoFinalState << std::endl;
        //std::cout << "*Size 5: " << targetMCParticleToHitsMap.size() << std::endl;

        LArMCParticleHelper::MCContributionMapVector mcParticlesToGoodHitsMaps({targetMCParticleToHitsMap});

        LArMCParticleHelper::PfoContributionMap pfoToReconstructable2DHitsMap;
        LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(myPfoList, mcParticlesToGoodHitsMaps, pfoToReconstructable2DHitsMap, primaryParameters.m_foldBackHierarchy);

        LArMCParticleHelper::PfoToMCParticleHitSharingMap pfoToMCParticleHitSharingMap;
        LArMCParticleHelper::MCParticleToPfoHitSharingMap mcParticleToPfoHitSharingMap;
        LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToReconstructable2DHitsMap, mcParticlesToGoodHitsMaps, pfoToMCParticleHitSharingMap, mcParticleToPfoHitSharingMap);  //#!!!!!#

                        
        //std::cout << "  --Finished Calling LArMCParticleHelper functions--" << std::endl;            
            
        //const CaloHitList &allHitsInPfo(pfoToReconstructable2DHitsMap.at(pPfo));
	    //const int nHitsInPfoTotal(allHitsInPfo.size());
	    int nHitsSharedWithBestMCParticleTotal(-1);
	        
	    //std::cout << "Size 1: " << pfoToReconstructable2DHitsMap.size() << std::endl;
        //std::cout << "Size 2: " << mcParticlesToGoodHitsMaps.size() << std::endl;
        //std::cout << "Size 3: " << pfoToMCParticleHitSharingMap.size() << std::endl;
        //std::cout << "Size 4: " << mcParticleToPfoHitSharingMap.size() << std::endl;
            
	    const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(pfoToMCParticleHitSharingMap.at(pPfo));
	    
	    //std::cout << "About to start the loop..." << std::endl;

        const pandora::MCParticle* bestMCParticleMatch(nullptr);
	    for (const LArMCParticleHelper::MCParticleCaloHitListPair &mcParticleCaloHitListPair : mcParticleToSharedHitsVector)
	    {     
	        // std::cout << "      --In second for loop--" << std::endl;
	            
	        //const pandora::MCParticle* pAssociatedMCParticle(mcParticleCaloHitListPair.first);  
	        //const CaloHitList &allMCHits(targetMCParticleToHitsMap.at(pAssociatedMCParticle));
	        const CaloHitList &associatedMCHits(mcParticleCaloHitListPair.second);
	       
	        //std::cout << bestMCParticleMatch << std::endl;

	        if (static_cast<int>(associatedMCHits.size()) > nHitsSharedWithBestMCParticleTotal)
	        {
	            
	            //std::cout << "         --Entered if statement--" << std::endl;
	            
		        nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
		        bestMCParticleMatch = mcParticleCaloHitListPair.first; //### Issue with const here ###     
	        }
	    }		
        
        return bestMCParticleMatch;
    }
    
    
    
    StatusCode ParticleHierarchyValidationAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle){
        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "MinPrimaryGoodHits", m_primaryParameters.m_minPrimaryGoodHits));

        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "MinHitsForGoodView", m_primaryParameters.m_minHitsForGoodView));

        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "MinPrimaryGoodViews", m_primaryParameters.m_minPrimaryGoodViews));

        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "SelectInputHits", m_primaryParameters.m_selectInputHits));

        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "MinHitSharingFraction", m_primaryParameters.m_minHitSharingFraction));

        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "MaxPhotonPropagation", m_primaryParameters.m_maxPhotonPropagation));

        PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
            "FoldToPrimaries", m_primaryParameters.m_foldBackHierarchy));
        return STATUS_CODE_SUCCESS;
    }
    
}
