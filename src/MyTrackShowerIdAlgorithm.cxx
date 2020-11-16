/**
 *  @file   LArReco/src/MyTrackShowerIdAlgorithm.cxx
 *
 *  @brief  Implementation of the MyTrackShowerIdAlgorithm class.
 *
 *  $Log: $
 */
#include "Pandora/AlgorithmHeaders.h"
#include "Helpers/MCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArClusterHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArObjectHelper.h"
#include "larpandoracontent/LArHelpers/LArPcaHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "MyTrackShowerIdAlgorithm.h"
using namespace pandora;
using namespace lar_content;
using namespace lar_reco;


MyTrackShowerIdAlgorithm::MyTrackShowerIdAlgorithm():
    m_writeToTree(),
    m_eventId(-1),
    m_treeName(),
    m_fileName(),
    m_mcParticleListName(),
    m_caloHitListName(),
    m_inputPfoListName()
{
}

MyTrackShowerIdAlgorithm::~MyTrackShowerIdAlgorithm()
{
    if (m_writeToTree)
        PANDORA_MONITORING_API(SaveTree(this->GetPandora(), m_treeName.c_str(), m_fileName.c_str(), "UPDATE"));
}
//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode MyTrackShowerIdAlgorithm::Run()
{
    // ATTN - m_eventId is initialised to -1, so the first event is event zero
    ++m_eventId;

    // Input lists
    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_inputPfoListName, pPfoList));
    
    const MCParticleList *pMCParticleList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_mcParticleListName, pMCParticleList));
    
    const CaloHitList *pCaloHitList = nullptr;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_caloHitListName, pCaloHitList));
    
    // Mapping target MCParticles -> truth associated Hits
    // ATTN - We're writing out PFOs from the end of the reco chain, so don't apply any reconstructability criteria, or empty PFOs may result
    LArMCParticleHelper::MCContributionMap targetMCParticleToHitsMap;
    LArMCParticleHelper::PrimaryParameters parameters;
    parameters.m_foldBackHierarchy = true;
    parameters.m_minPrimaryGoodHits = 0;
    parameters.m_minHitsForGoodView = 0;
    parameters.m_minHitSharingFraction = 0.f;
    parameters.m_maxPhotonPropagation = std::numeric_limits<float>::max();
    parameters.m_selectInputHits = false;

    LArMCParticleHelper::SelectReconstructableMCParticles(pMCParticleList, pCaloHitList, parameters, LArMCParticleHelper::IsBeamNeutrinoFinalState, targetMCParticleToHitsMap);
    
    // Mapping reconstructed particles -> reconstruction associated Hits
    PfoList allConnectedPfos;
    LArPfoHelper::GetAllConnectedPfos(*pPfoList, allConnectedPfos);
    
    PfoList finalStatePfos;
    for (const ParticleFlowObject *const pPfo : allConnectedPfos)
    {
        if (LArPfoHelper::IsFinalState(pPfo))
            finalStatePfos.push_back(pPfo);
    }
    
    LArMCParticleHelper::PfoContributionMap pfoToHitsMap;
    const bool foldBackHierarchy(parameters.m_foldBackHierarchy);
    LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(finalStatePfos, targetMCParticleToHitsMap, pfoToHitsMap, foldBackHierarchy);
    
    // Matching step
    LArMCParticleHelper::PfoToMCParticleHitSharingMap pfoToMCHitSharingMap;
    LArMCParticleHelper::MCParticleToPfoHitSharingMap mcToPfoHitSharingMap;

    LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToHitsMap, {targetMCParticleToHitsMap}, pfoToMCHitSharingMap, mcToPfoHitSharingMap);
    
    // PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, -1.f, 1.f));
    // PANDORA_MONITORING_API(VisualizeParticleFlowObjects(this->GetPandora(), pPfoList, "GotThePfoList", RED));
    // PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
    // Write one tree entry for each Pfo
    int pfoId(0);
    for (const Pfo *const pPfo : finalStatePfos)
    {
        const CaloHitList &allHitsInPfo(pfoToHitsMap.at(pPfo));
        std::cout << "We got a pfo, isNeutrinoFinalState " << LArPfoHelper::IsNeutrinoFinalState(pPfo) << ", nHits " << allHitsInPfo.size()
                  << " (U: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allHitsInPfo)
                  << ", V: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allHitsInPfo)
                  << ", W: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allHitsInPfo) << ") " << std::endl;
        const int nHitsInPfoTotal(allHitsInPfo.size()),
            nHitsInPfoU(LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allHitsInPfo)),
            nHitsInPfoV(LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allHitsInPfo)),
            nHitsInPfoW(LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allHitsInPfo));
        
        CaloHitList wHitsInPfo;
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_W, wHitsInPfo);
        
        // PANDORA_MONITORING_API(VisualizeCaloHits(this->GetPandora(), &wHitsInPfo, "WHitsInThisPfo", CYAN));
        // PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
        
        int nHitsInBestMCParticleTotal(-1),
            nHitsInBestMCParticleU(-1),
            nHitsInBestMCParticleV(-1),
            nHitsInBestMCParticleW(-1),
            bestMCParticlePdgCode(0),
            bestMCParticleIsTrack(-1);
        int nHitsSharedWithBestMCParticleTotal(-1),
            nHitsSharedWithBestMCParticleU(-1),
            nHitsSharedWithBestMCParticleV(-1),
            nHitsSharedWithBestMCParticleW(-1);
        
        const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(pfoToMCHitSharingMap.at(pPfo));
        
        for (const LArMCParticleHelper::MCParticleCaloHitListPair &mcParticleCaloHitListPair : mcParticleToSharedHitsVector)
        {
            const pandora::MCParticle *const pAssociatedMCParticle(mcParticleCaloHitListPair.first);
            const CaloHitList &allMCHits(targetMCParticleToHitsMap.at(pAssociatedMCParticle));
            std::cout << "Associated MCParticle: " << pAssociatedMCParticle->GetParticleId() << ", nTotalHits " << allMCHits.size()
                      << " (U: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allMCHits)
                      << ", V: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allMCHits)
                      << ", W: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allMCHits) << ") " << std::endl;
            
            const CaloHitList &associatedMCHits(mcParticleCaloHitListPair.second);
            
            CaloHitList associatedMCHitsW;
            for (const CaloHit *const pCaloHit : associatedMCHits)
            {
                if (TPC_VIEW_W == pCaloHit->GetHitType())
                    associatedMCHitsW.push_back(pCaloHit);
            }
            
            std::cout << "Shared with MCParticle: " << pAssociatedMCParticle->GetParticleId() << ", nSharedHits " << associatedMCHits.size()
                      << " (U: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, associatedMCHits)
                      << ", V: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, associatedMCHits)
                      << ", W: " << LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, associatedMCHits) << ") " << std::endl;
            if (static_cast<int>(associatedMCHits.size()) > nHitsSharedWithBestMCParticleTotal)
            {
                // This is the current best matched MCParticle, to be stored
                nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
                std::cout << "associatedMCHits.size() " << associatedMCHits.size() << ", nHitsSharedWithBestMCParticleTotal " << nHitsSharedWithBestMCParticleTotal << std::endl;
                nHitsSharedWithBestMCParticleU = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, associatedMCHits);
                nHitsSharedWithBestMCParticleV = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, associatedMCHits);
                nHitsSharedWithBestMCParticleW = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, associatedMCHits);
                nHitsInBestMCParticleTotal = allMCHits.size();
                nHitsInBestMCParticleU = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allMCHits);
                nHitsInBestMCParticleV = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allMCHits);
                nHitsInBestMCParticleW = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allMCHits);
                bestMCParticlePdgCode = pAssociatedMCParticle->GetParticleId();
                bestMCParticleIsTrack = ((PHOTON != pAssociatedMCParticle->GetParticleId()) && (E_MINUS != std::abs(pAssociatedMCParticle->GetParticleId())) ? 1 : 0);
            }
            // PANDORA_MONITORING_API(VisualizeCaloHits(this->GetPandora(), &associatedMCHitsW, "associatedMCHitsW", GREEN));
            // PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
        }
        // Cache values here
        const float purity((nHitsInPfoTotal > 0) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal) / static_cast<float>(nHitsInPfoTotal) : 0.f);
        const float completeness((nHitsInBestMCParticleTotal > 0) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal)/static_cast<float>(nHitsInBestMCParticleTotal) : 0.f);
        FloatVector hitDriftPositionsW, hitWirePositionsW, hitEnergiesW;
        for (const CaloHit *const pCaloHit : wHitsInPfo)
        {
            hitDriftPositionsW.push_back(pCaloHit->GetPositionVector().GetX());
            hitWirePositionsW.push_back(pCaloHit->GetPositionVector().GetZ());
            hitEnergiesW.push_back(pCaloHit->GetInputEnergy());
        }
        // Write to tree here
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "eventId", m_eventId));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "pfoId", pfoId));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoTotal", nHitsInPfoTotal));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoU", nHitsInPfoU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoV", nHitsInPfoV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoW", nHitsInPfoW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleTotal", nHitsInBestMCParticleTotal));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleU", nHitsInBestMCParticleU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleV", nHitsInBestMCParticleV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleW", nHitsInBestMCParticleW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsSharedWithBestMCParticleTotal", nHitsSharedWithBestMCParticleTotal));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsSharedWithBestMCParticleU", nHitsSharedWithBestMCParticleU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsSharedWithBestMCParticleV", nHitsSharedWithBestMCParticleV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsSharedWithBestMCParticleW", nHitsSharedWithBestMCParticleW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "bestMCParticlePdgCode", bestMCParticlePdgCode));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "bestMCParticleIsTrack", bestMCParticleIsTrack));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "purity", purity));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "completeness", completeness));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "hitDriftPositionsW", &hitDriftPositionsW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "hitWirePositionsW", &hitWirePositionsW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "hitEnergiesW", &hitEnergiesW));
        PANDORA_MONITORING_API(FillTree(this->GetPandora(), m_treeName.c_str()));

        ++pfoId;
    }
    
    return STATUS_CODE_SUCCESS;
}
//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode MyTrackShowerIdAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "WriteToTree", m_writeToTree));
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "InputPfoListName", m_inputPfoListName));
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "CaloHitListName", m_caloHitListName));
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "MCParticleListName", m_mcParticleListName));
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "TreeName", m_treeName));
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "FileName", m_fileName));
    
    return STATUS_CODE_SUCCESS;
}

