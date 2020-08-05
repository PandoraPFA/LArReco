
#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArHelpers/LArPointingClusterHelper.h"
#include "larpandoracontent/LArObjects/LArPointingCluster.h"

#include "larpandoracontent/LArMonitoring/NeutrinoEventValidationAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArObjects/LArTwoDSlidingFitResult.h"

#include "ParticleHierarchyValidationAlgorithm.h"

using namespace pandora;

namespace development_area
{
    ParticleHierarchyValidationAlgorithm::ParticleHierarchyValidationAlgorithm():
        m_writeToTree(true),
        m_treeName("EventTree"),
        m_fileName("FirstTree.root"),
        eventNo(0),
        addAll(false),   //In future, add this as an optional in the xml (ReadSettings)
        m_showAllPfoData(true)
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
        

            /*
            float PFONo(0);
            PFONo = relevantPfos.size();
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "PFONo", PFONo);
            
            float MCPNo(0); 
            MCPNo = relevantMCParts.size();
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "MCPNo", MCPNo);
 
            float HitsU(0);
            HitsU = CaloHitsU->size();
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "HitsU", HitsU);
            float HitsV(0);
            HitsV = CaloHitsV->size();
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "HitsV", HitsV);
            float HitsW(0);
            HitsW = CaloHitsW->size();
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "HitsW", HitsW);    
            */
                            
        
        
                /*Currently, this metric is very unorganised. 
                It will store the number of hits in each view a particular PFO and MCP got and then later we will just graph out this as is.
                There is  no association to a particular PFO by that point
                */
                
            //Finding the number of hits in each PFO
        
        int PfoTempId(0);
        for (const ParticleFlowObject *const c_PFO : relevantPfos)
        {
            std::vector<int> Hits;
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
                            std::cout << "         >" << "CaloHitView: U" << std::endl;
                            NUHits = NUHits+1;
                            break;
                        case 5:
                            std::cout << "         >" << "CaloHitView: V" << std::endl;
                            NVHits = NVHits+1;
                            break;
                        case 6:
                            std::cout << "         >" << "CaloHitView: W" << std::endl;
                            NWHits = NWHits+1;
                            break;
                        default:
                            break;
                    }       
                }
            }
            std::cout << "  " << std::endl;

            Hits.push_back(NUHits);
            Hits.push_back(NVHits);
            Hits.push_back(NWHits);
            
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "eventNo", eventNo);
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "PfoTempId", PfoTempId);
            PandoraMonitoringApi::SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "ViewHits", &Hits);
            
            PandoraMonitoringApi::FillTree(this->GetPandora(), m_treeName.c_str());
        }
        
        return STATUS_CODE_SUCCESS;
    }
    StatusCode ParticleHierarchyValidationAlgorithm::ReadSettings(const pandora::TiXmlHandle /*xmlHandle*/){
        
        return STATUS_CODE_SUCCESS;
    }
    
}
