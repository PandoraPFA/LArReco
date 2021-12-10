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
#include "larpandoracontent/LArHelpers/LArInteractionTypeHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArObjectHelper.h"
#include "larpandoracontent/LArHelpers/LArPcaHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
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
    
    const MCParticleList *pMCParticleList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_mcParticleListName, pMCParticleList));
    
    const CaloHitList *pCaloHitList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_caloHitListName, pCaloHitList));

    // True vertex extraction
    MCParticleVector primaries;
    LArMCParticleHelper::GetPrimaryMCParticleList(pMCParticleList, primaries);
    const MCParticle *pTrueNeutrino{nullptr};
    if (!primaries.empty())
    {   
        const MCParticle *primary{primaries.front()};
        const MCParticleList &parents{primary->GetParentList()};
        if (parents.size() == 1 && LArMCParticleHelper::IsNeutrino(parents.front()))
            pTrueNeutrino = parents.front();
    }
    float trueNeutrinoVertexX{0.f}, trueNeutrinoVertexY{0.f}, trueNeutrinoVertexZ{0.f}, trueNeutrinoVertexU{0.f}, trueNeutrinoVertexV{0.f},
        trueNeutrinoVertexW{0.f};
    if (pTrueNeutrino)
    {
        const LArTransformationPlugin *transform{this->GetPandora().GetPlugins()->GetLArTransformationPlugin()}; 
        const CartesianVector &trueVertex{pTrueNeutrino->GetVertex()};
        trueNeutrinoVertexX = trueVertex.GetX();
        trueNeutrinoVertexY = trueVertex.GetY();
        trueNeutrinoVertexZ = trueVertex.GetZ();
        trueNeutrinoVertexU = static_cast<float>(transform->YZtoU(trueVertex.GetY(), trueVertex.GetZ()));
        trueNeutrinoVertexV = static_cast<float>(transform->YZtoV(trueVertex.GetY(), trueVertex.GetZ()));
        trueNeutrinoVertexW = static_cast<float>(transform->YZtoW(trueVertex.GetY(), trueVertex.GetZ()));
    }

    // Get reconstructable MC
    LArMCParticleHelper::MCContributionMap primaryMCParticleToHitsMap;
    LArMCParticleHelper::PrimaryParameters parameters;
    parameters.m_foldBackHierarchy = true;

    LArMCParticleHelper::SelectReconstructableMCParticles(pMCParticleList, pCaloHitList, parameters,
        LArMCParticleHelper::IsBeamNeutrinoFinalState, primaryMCParticleToHitsMap);
    MCParticleList primaryMCList;
    for (auto [ pMC, hits ] : primaryMCParticleToHitsMap)
    {   (void)hits;
        primaryMCList.emplace_back(pMC);
    }

    int interactionType{static_cast<int>(LArInteractionTypeHelper::GetInteractionType(primaryMCList))};

    std::map<const MCParticle *, int> mcToHitsMapU, mcToHitsMapV, mcToHitsMapW;
    for (const CaloHit *pCaloHit : *pCaloHitList)
    {
        try
        {
            const MCParticle *pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));
            if (std::find(primaryMCList.begin(), primaryMCList.end(), pMCParticle) != primaryMCList.end())
            {
                if (pCaloHit->GetHitType() == TPC_VIEW_U)
                    mcToHitsMapU[pMCParticle]++;
                else if (pCaloHit->GetHitType() == TPC_VIEW_V)
                    mcToHitsMapV[pMCParticle]++;
                else if (pCaloHit->GetHitType() == TPC_VIEW_W)
                    mcToHitsMapW[pMCParticle]++;
            }
        }
        catch (const StatusCodeException &)
        {
        }
    }

    int nTrueTracksU{0}, nTrueTracksV{0}, nTrueTracksW{0}, nTrueShowersU{0}, nTrueShowersV{0}, nTrueShowersW{0};
    for (const MCParticle *pMCParticle : primaryMCList)
    {
        const int pdg{std::abs(pMCParticle->GetParticleId())};
        const bool isTrack{!(pdg == E_MINUS || pdg == PHOTON)};
        const int nHitsU{mcToHitsMapU[pMCParticle]}, nHitsV{mcToHitsMapV[pMCParticle]}, nHitsW{mcToHitsMapW[pMCParticle]};
        if (nHitsU >= 5)
        {
            if (isTrack)
                ++nTrueTracksU;
            else
                ++nTrueShowersU;
        }
        if (nHitsV >= 5)
        {
            if (isTrack)
                ++nTrueTracksV;
            else
                ++nTrueShowersV;
        }
        if (nHitsW >= 5)
        {
            if (isTrack)
                ++nTrueTracksW;
            else
                ++nTrueShowersW;
        }
    }

    // Mapping target MCParticles -> truth associated Hits
    // ATTN - We're writing out PFOs from the end of the reco chain, so don't apply any reconstructability criteria, or empty PFOs may result
    LArMCParticleHelper::MCContributionMap targetMCParticleToHitsMap;
    parameters.m_foldBackHierarchy = false;
    parameters.m_minPrimaryGoodHits = 0;
    parameters.m_minHitsForGoodView = 0;
    parameters.m_minHitSharingFraction = 0.f;
    LArMCParticleHelper::SelectReconstructableMCParticles(pMCParticleList, pCaloHitList, parameters,
        LArMCParticleHelper::IsBeamNeutrinoFinalState, targetMCParticleToHitsMap);
    
    // Mapping reconstructed particles -> reconstruction associated Hits
    PfoList allConnectedPfos;
    LArPfoHelper::GetAllConnectedPfos(*pPfoList, allConnectedPfos);

    std::map<const ParticleFlowObject *, int> pfoToIdMap;
    int i{1};
    for (const ParticleFlowObject *const pPfo : allConnectedPfos)
    {
        if (!LArPfoHelper::IsNeutrino(pPfo))
        {
            pfoToIdMap[pPfo] = i;
            ++i;
        }
        else
            pfoToIdMap[pPfo] = 0;
    }
    
    PfoList finalStatePfos;
    for (const ParticleFlowObject *const pPfo : allConnectedPfos)
    {
        if (LArPfoHelper::IsFinalState(pPfo))
            finalStatePfos.push_back(pPfo);
    }
    int nFinalStateParticles{static_cast<int>(finalStatePfos.size())};
    LArMCParticleHelper::PfoContributionMap pfoToHitsMap;
    LArMCParticleHelper::GetPfoToReconstructable2DHitsMap(allConnectedPfos, targetMCParticleToHitsMap, pfoToHitsMap, parameters.m_foldBackHierarchy);
    
    // Matching step
    LArMCParticleHelper::PfoToMCParticleHitSharingMap pfoToMCHitSharingMap;
    LArMCParticleHelper::MCParticleToPfoHitSharingMap mcToPfoHitSharingMap;

    LArMCParticleHelper::GetPfoMCParticleHitSharingMaps(pfoToHitsMap, {targetMCParticleToHitsMap}, pfoToMCHitSharingMap, mcToPfoHitSharingMap);
    
    // PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, -1.f, 1.f));
    // PANDORA_MONITORING_API(VisualizeParticleFlowObjects(this->GetPandora(), pPfoList, "GotThePfoList", RED));
    // PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
    // Write one tree entry for each Pfo
    for (const Pfo *const pPfo : allConnectedPfos)
    {
        if (LArPfoHelper::IsNeutrino(pPfo))
            continue;
        const PfoList &parentList{pPfo->GetParentPfoList()};
        const ParticleFlowObject *pParentPfo{parentList.size() == 1 ? parentList.front() : nullptr};
        const int parentId{pParentPfo ? pfoToIdMap[pParentPfo] : -1};
        IntVector childPfos;
        const PfoList &childList{pPfo->GetDaughterPfoList()}; 
        for (const ParticleFlowObject *pChild : childList)
            childPfos.emplace_back(pfoToIdMap[pChild]);

        CaloHitList allHitsInPfo;
        LArPfoHelper::GetAllCaloHits(pPfo, allHitsInPfo);
        const int nHitsInPfoTotal(allHitsInPfo.size()),
            nHitsInPfoU(LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allHitsInPfo)),
            nHitsInPfoV(LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allHitsInPfo)),
            nHitsInPfoW(LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allHitsInPfo));
        CaloHitList uHitsInPfo, vHitsInPfo, wHitsInPfo, hitsInPfo3D;
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_U, uHitsInPfo);
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_V, vHitsInPfo);
        LArPfoHelper::GetCaloHits(pPfo, TPC_VIEW_W, wHitsInPfo);
        LArPfoHelper::GetCaloHits(pPfo, TPC_3D, hitsInPfo3D);
        
        // PANDORA_MONITORING_API(VisualizeCaloHits(this->GetPandora(), &wHitsInPfo, "WHitsInThisPfo", CYAN));
        // PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));
        
        int nHitsInBestMCParticleTotal(-1),
            nHitsInBestMCParticleU(-1),
            nHitsInBestMCParticleV(-1),
            nHitsInBestMCParticleW(-1),
            bestMCParticlePdgCode(0);
        int nHitsSharedWithBestMCParticleTotal(-1);
        
        const LArMCParticleHelper::MCParticleToSharedHitsVector &mcParticleToSharedHitsVector(pfoToMCHitSharingMap.at(pPfo));
        
        for (const LArMCParticleHelper::MCParticleCaloHitListPair &mcParticleCaloHitListPair : mcParticleToSharedHitsVector)
        {
            const pandora::MCParticle *const pAssociatedMCParticle(mcParticleCaloHitListPair.first);
            const CaloHitList &allMCHits(targetMCParticleToHitsMap.at(pAssociatedMCParticle));
            const CaloHitList &associatedMCHits(mcParticleCaloHitListPair.second);
            
            CaloHitList associatedMCHitsW;
            for (const CaloHit *const pCaloHit : associatedMCHits)
            {
                if (TPC_VIEW_W == pCaloHit->GetHitType())
                    associatedMCHitsW.push_back(pCaloHit);
            }
            
            if (static_cast<int>(associatedMCHits.size()) > nHitsSharedWithBestMCParticleTotal)
            {
                // This is the current best matched MCParticle, to be stored
                nHitsSharedWithBestMCParticleTotal = associatedMCHits.size();
                nHitsInBestMCParticleTotal = allMCHits.size();
                nHitsInBestMCParticleU = LArMonitoringHelper::CountHitsByType(TPC_VIEW_U, allMCHits);
                nHitsInBestMCParticleV = LArMonitoringHelper::CountHitsByType(TPC_VIEW_V, allMCHits);
                nHitsInBestMCParticleW = LArMonitoringHelper::CountHitsByType(TPC_VIEW_W, allMCHits);
                bestMCParticlePdgCode = pAssociatedMCParticle->GetParticleId();
            }
        }
        bool isTrack(true);
        if (std::abs(bestMCParticlePdgCode) == E_MINUS || std::abs(bestMCParticlePdgCode) == PHOTON)
            isTrack = false;
        // Cache values here
        const int isPandoraTrack(pPfo->GetParticleId() == MU_MINUS ? 1 : 0);
        const int isTrueTrack(isTrack ? 1 : 0);
        const float purity((nHitsInPfoTotal > 0) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal) / static_cast<float>(nHitsInPfoTotal) : 0.f);
        const float completeness((nHitsInBestMCParticleTotal > 0) ? static_cast<float>(nHitsSharedWithBestMCParticleTotal)/static_cast<float>(nHitsInBestMCParticleTotal) : 0.f);
        FloatVector hitDriftPositionsU, hitWirePositionsU, hitEnergiesU,
                    hitDriftPositionsV, hitWirePositionsV, hitEnergiesV,
                    hitDriftPositionsW, hitWirePositionsW, hitEnergiesW,
                    hit3DPositionsX, hit3DPositionsY, hit3DPositionsZ;

        for (const CaloHit *const pCaloHit : uHitsInPfo)
        {
            hitDriftPositionsU.push_back(pCaloHit->GetPositionVector().GetX());
            hitWirePositionsU.push_back(pCaloHit->GetPositionVector().GetZ());
            hitEnergiesU.push_back(pCaloHit->GetInputEnergy());
        }
        for (const CaloHit *const pCaloHit : vHitsInPfo)
        {
            hitDriftPositionsV.push_back(pCaloHit->GetPositionVector().GetX());
            hitWirePositionsV.push_back(pCaloHit->GetPositionVector().GetZ());
            hitEnergiesV.push_back(pCaloHit->GetInputEnergy());
        }
        for (const CaloHit *const pCaloHit : wHitsInPfo)
        {
            hitDriftPositionsW.push_back(pCaloHit->GetPositionVector().GetX());
            hitWirePositionsW.push_back(pCaloHit->GetPositionVector().GetZ());
            hitEnergiesW.push_back(pCaloHit->GetInputEnergy());
        }
        for (const CaloHit *const pCaloHit : hitsInPfo3D)
        {
            hit3DPositionsX.push_back(pCaloHit->GetPositionVector().GetX());
            hit3DPositionsY.push_back(pCaloHit->GetPositionVector().GetY());
            hit3DPositionsZ.push_back(pCaloHit->GetPositionVector().GetZ());
        }
        //GetVertex
        const pandora::Vertex* vertex{nullptr};
        try
        {
            vertex = LArPfoHelper::GetVertex(pPfo);
        }
        catch (StatusCodeException &)
        {
        }
        CartesianVector vertexPosition(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        if (vertex)
            vertexPosition = vertex->GetPosition();
        float vertexPositionX = vertexPosition.GetX();
        float vertexPositionY = vertexPosition.GetY();
        float vertexPositionZ = vertexPosition.GetZ();

        //Project vertex on U,V views
        CartesianVector vertexPositionU = vertex ? LArGeometryHelper::ProjectPosition(this->GetPandora(),vertexPosition,TPC_VIEW_U) : vertexPosition;
        CartesianVector vertexPositionV = vertex ? LArGeometryHelper::ProjectPosition(this->GetPandora(),vertexPosition,TPC_VIEW_V) : vertexPosition;
        CartesianVector vertexPositionW = vertex ? LArGeometryHelper::ProjectPosition(this->GetPandora(),vertexPosition,TPC_VIEW_W) : vertexPosition;

        float vertexDriftPosition=vertexPositionU.GetX();
        float vertexWirePositionU=vertexPositionU.GetZ();
        float vertexWirePositionV=vertexPositionV.GetZ();
        float vertexWirePositionW=vertexPositionW.GetZ();

        //Get interaction vertex
        const pandora::Vertex* intVertex = LArPfoHelper::GetVertex(LArPfoHelper::GetParentNeutrino(pPfo));
        CartesianVector intVertexPosition = intVertex->GetPosition();
        CartesianVector intVertexPositionU = LArGeometryHelper::ProjectPosition(this->GetPandora(), intVertexPosition, TPC_VIEW_U);
        CartesianVector intVertexPositionV = LArGeometryHelper::ProjectPosition(this->GetPandora(), intVertexPosition, TPC_VIEW_V);
        CartesianVector intVertexPositionW = LArGeometryHelper::ProjectPosition(this->GetPandora(), intVertexPosition, TPC_VIEW_W);
        // Write to tree here
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "eventId", m_eventId));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "interactionType", interactionType));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nFinalStateParticles", nFinalStateParticles));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "pfoId", pfoToIdMap[pPfo]));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "parent", parentId));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "children", &childPfos));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoTotal", nHitsInPfoTotal));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoU", nHitsInPfoU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoV", nHitsInPfoV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInPfoW", nHitsInPfoW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleTotal", nHitsInBestMCParticleTotal));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleU", nHitsInBestMCParticleU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleV", nHitsInBestMCParticleV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nHitsInBestMCParticleW", nHitsInBestMCParticleW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "bestMCParticlePdgCode", bestMCParticlePdgCode));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "purity", purity));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "completeness", completeness));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "isPandoraTrack", isPandoraTrack));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "isTrueTrack", isTrueTrack));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "xU", &hitDriftPositionsU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "u", &hitWirePositionsU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "adcU", &hitEnergiesU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "xV", &hitDriftPositionsV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "v", &hitWirePositionsV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "adcV", &hitEnergiesV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "xW", &hitDriftPositionsW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "w", &hitWirePositionsW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "adcW", &hitEnergiesW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "x3d", &hit3DPositionsX));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "y3d", &hit3DPositionsY));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "z3d", &hit3DPositionsZ));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxX3d", vertexPositionX));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxY3d", vertexPositionY));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxZ3d", vertexPositionZ));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxX", vertexDriftPosition));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxU", vertexWirePositionU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxV", vertexWirePositionV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "vtxW", vertexWirePositionW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxX3d", intVertexPosition.GetX()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxY3d", intVertexPosition.GetY()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxZ3d", intVertexPosition.GetZ()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxX", intVertexPosition.GetX()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxU", intVertexPositionU.GetZ()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxV", intVertexPositionV.GetZ()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nuVtxW", intVertexPositionW.GetZ()));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxX3d", trueNeutrinoVertexX));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxY3d", trueNeutrinoVertexY));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxZ3d", trueNeutrinoVertexZ));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxX", trueNeutrinoVertexX));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxU", trueNeutrinoVertexU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxV", trueNeutrinoVertexV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "trueNuVtxW", trueNeutrinoVertexW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nTrueTracksU", nTrueTracksU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nTrueTracksV", nTrueTracksV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nTrueTracksW", nTrueTracksW));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nTrueShowersU", nTrueShowersU));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nTrueShowersV", nTrueShowersV));
        PANDORA_MONITORING_API(SetTreeVariable(this->GetPandora(), m_treeName.c_str(), "nTrueShowersW", nTrueShowersW));
        PANDORA_MONITORING_API(FillTree(this->GetPandora(), m_treeName.c_str()));
    }
    std::cout << std::endl;
    
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

