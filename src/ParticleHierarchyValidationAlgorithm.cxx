
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

namespace development_area
{ 
    ParticleHierarchyValidationAlgorithm::ParticleHierarchyValidationAlgorithm():
        m_writeToTree(true),
        m_treeName("EventTree"),
        m_fileName("FirstTree.root"),
        eventNo(0),
        addAll(false),   //In future, add this as an optional in the xml (ReadSettings), this just decides whether or not to add all particles to the relevance list or just the ones that meet criteria
        m_showAllPfoData(false)
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
        eventNo+=1;
        std::cout << "<--Event Number " << eventNo << "-->" << std::endl;
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
        
        
                //At this point, relevance is decided by "is it either electron flavour or muon flavoured (but not neutrinos)"
            //c_ stands for current here
        PfoList relevantPfos;
        for (const ParticleFlowObject *c_pfo : *Pfos)
        {
            if (!addAll){
                if (c_pfo->GetParticleId() == 11){
                    //std::cout << "Found a Reco Electron" << std::endl;
                    relevantPfos.push_back(c_pfo);
                    //std::cout << "Added Electron to 'relevantPfos'" << std::endl;
                }
                else if (c_pfo->GetParticleId() == 12){
                    //std::cout << "Found a Reco Electron-Neutrino" << std::endl;
                }
                else if (c_pfo->GetParticleId() == 13){
                    //std::cout << "Found a Reco Muon" << std::endl;
                    relevantPfos.push_back(c_pfo);
                    //std::cout << "Added Muon to 'relevantPfos'" << std::endl;
                }
                else if (c_pfo->GetParticleId() == 14){
                    //std::cout << "Found a Reco Muon-Neutrino" << std::endl;
                }else{
                    //std::cout << "Found PFO, Id: " << c_pfo->GetParticleId() << std::endl;
                }
                
                    //Add All Particles
            }else{
                std::cout << "Found PFO, Id: " << c_pfo->GetParticleId() << std::endl;
                relevantPfos.push_back(c_pfo);
            }
        }
        
        MCParticleList relevantMCParts;
        for (const MCParticle *c_mcp : *MCParts)
        {   if (!addAll){
                if (c_mcp->GetParticleId() == -11){
                    //std::cout << "Found a MC Electron" << std::endl;
                    relevantMCParts.push_back(c_mcp);
                    //std::cout << "Added Electron to 'relevantMCParts'" << std::endl;
                }
                else if (c_mcp->GetParticleId() == 12){
                    //std::cout << "Found a MC Electron-Neutrino" << std::endl;
                }
                else if (c_mcp->GetParticleId() == -13){
                    //std::cout << "Found a MC Muon" << std::endl;
                    relevantMCParts.push_back(c_mcp);
                    //std::cout << "Added Muon to 'relevantMCParts'" << std::endl;
                }
                else if (c_mcp->GetParticleId() == -14){
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
       
                                        
            //Finding the number of hits in each PFO
        
        int PfoTempId(0);
        for (const ParticleFlowObject *const c_PFO : relevantPfos)
        {
            PfoTempId++;
            
            int NUHits(0);
            int NVHits(0);
            int NWHits(0);
            
            ClusterList c_clusterList = c_PFO->GetClusterList();
            if (m_showAllPfoData)
                std::cout << "   >" << "Number of clusters in PFO " << c_PFO->GetParticleId() << ": " << c_clusterList.size() << std::endl; 
                
               
                
                //Looking through clusterlist for cluster
            for (const Cluster *const c_cluster : c_clusterList)
            {
                const CaloHitList c_caloHitList = c_cluster->GetIsolatedCaloHitList();
                if (m_showAllPfoData)
                    std::cout << "     >" << "Number of hits in this cluster: " << c_caloHitList.size() << std::endl;
                    //Looking through cluster for CaloHits
                for (const CaloHit *const c_hit : c_caloHitList)
                {
                    switch(c_hit->GetHitType())
                    {
                        case 4:
                            if (m_showAllPfoData)
                                std::cout << "         >" << "CaloHitView: U" << std::endl;
                            NUHits = NUHits+1;
                            break;
                        case 5:
                            if (m_showAllPfoData)
                                std::cout << "         >" << "CaloHitView: V" << std::endl;
                            NVHits = NVHits+1;
                            break;
                        case 6:
                            if (m_showAllPfoData)
                                std::cout << "         >" << "CaloHitView: W" << std::endl;
                            NWHits = NWHits+1;
                            break;
                        default:
                            break;
                    }       
                }
            }
            std::cout << "  " << std::endl;
            
            if (m_writeToTree){
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "eventNo", eventNo);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "PfoTempId", PfoTempId);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "UHits", NUHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "VHits", NVHits);
                PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "WHits", NWHits);
            
                PandoraMonitoringApi::FillTree(this->GetPandora(), m_treeName.c_str());
            }
        }
        
        
        m_primaryParameters.m_minPrimaryGoodHits = 15;
        m_primaryParameters.m_minHitsForGoodView = 5;
        m_primaryParameters.m_minPrimaryGoodViews = 2;
        m_primaryParameters.m_selectInputHits = true;
        m_primaryParameters.m_maxPhotonPropagation = 2.5f;
        m_primaryParameters.m_minHitSharingFraction = 0.9f;
        m_primaryParameters.m_foldBackHierarchy = true;
        
        
        //std::cout << "--Finding Overlap--" << std::endl;
            //Finding the calohits overlap
        for (const ParticleFlowObject *pPfo : *Pfos){ 
        
            const PfoList myPfoList(1, pPfo);
        
            //std::cout << "  --Entered for loop--" << std::endl;
            
            LArMCParticleHelper::MCContributionMap targetMCParticleToHitsMap;
            LArMCParticleHelper::SelectReconstructableMCParticles(MCParts, CaloHits, m_primaryParameters, LArMCParticleHelper::IsLeadingBeamParticle, targetMCParticleToHitsMap);
            
            //std::cout << "*Size 1: " << MCParts->size() << std::endl;
            //std::cout << "*Size 2: " << CaloHits->size() << std::endl;
            //std::cout << "*Size 3: " << m_primaryParameters << std::endl;
            //std::cout << "*Size 4: " << LArMCParticleHelper::IsBeamNeutrinoFinalState << std::endl;
            //std::cout << "*Size 5: " << targetMCParticleToHitsMap.size() << std::endl;

            LArMCParticleHelper::MCContributionMapVector mcParticlesToGoodHitsMaps({targetMCParticleToHitsMap});

            LArMCParticleHelper::PfoContributionMap pfoToReconstructable2DHitsMap;
            LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(*Pfos, mcParticlesToGoodHitsMaps, pfoToReconstructable2DHitsMap, m_primaryParameters.m_foldBackHierarchy);

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
	        
	        for (const LArMCParticleHelper::MCParticleCaloHitListPair &mcParticleCaloHitListPair : mcParticleToSharedHitsVector)
	        {
	            
	            std::cout << "      --In second for loop--" << std::endl;
	            
	            const pandora::MCParticle *const pAssociatedMCParticle(mcParticleCaloHitListPair.first);
	            const CaloHitList &allMCHits(targetMCParticleToHitsMap.at(pAssociatedMCParticle));
	            const CaloHitList &associatedMCHits(mcParticleCaloHitListPair.second);

	            if (static_cast<int>(associatedMCHits.size()) > nHitsSharedWithBestMCParticleTotal)
	            {
	            
	             std::cout << "         --Entered if statement--" << std::endl;
	            
		         nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();

		         nHitsInBestMCParticleTotal = allMCHits.size();               
	            }
	        }		

            std::cout << "      --Calculating completeness and purity--" << std::endl;

            const float completeness((nHitsInBestMCParticleTotal > 0) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal) / static_cast<float>(nHitsInBestMCParticleTotal) : 0.f);
            const float purity((nHitsInPfoTotal > 0 ) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal) / static_cast<float>(nHitsInPfoTotal) : 0.f);
            
            std::cout << "Completeness: " << completeness << std::endl;
            std::cout << "Purity: " << purity << std::endl;
        }
        
        
        return STATUS_CODE_SUCCESS;
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
