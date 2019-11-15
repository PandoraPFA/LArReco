/**
 *  @file   PandoraSDK/src/Persistency/BScProjectInputsAlgorithm.cc
 *
 *  @brief  Implementation of the event writing algorithm class.
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "BScProjectInputsAlgorithm.h"

#include "Pandora/PandoraInternal.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArInteractionTypeHelper.h"
#include "Objects/OrderedCaloHitList.h"

#include <fstream>

using namespace pandora;
using namespace lar_content;

namespace bsc_project
{

BScProjectInputsAlgorithm::BScProjectInputsAlgorithm():
    m_inputCaloHitListName(""),
    m_inputCaloHitListNameW(""),
    m_outputFilename("")
{

}

//------------------------------------------------------------------------------------------------------------------------------------------

BScProjectInputsAlgorithm::~BScProjectInputsAlgorithm()
{
    if(m_file.is_open())
    {
        m_file.close();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode BScProjectInputsAlgorithm::Run()
{
    if(!m_file.is_open())
    {
        return STATUS_CODE_FAILURE;
    }

    const CaloHitList *pCaloHitList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(
        *this, m_inputCaloHitListName, pCaloHitList));
        
    const CaloHitList *pCaloHitListW(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(
        *this, m_inputCaloHitListNameW, pCaloHitListW));

    const MCParticleList *pMCParticleList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=,
        PandoraContentApi::GetCurrentList(*this, pMCParticleList));

    LArMCParticleHelper::MCContributionMap targetMCParticlesToGoodHitsMap;

    // ATTN Assumes single neutrino is parent of all neutrino-induced mc particles
    LArMCParticleHelper::SelectReconstructableMCParticles(pMCParticleList,
        pCaloHitList, LArMCParticleHelper::PrimaryParameters(),
        LArMCParticleHelper::IsBeamNeutrinoFinalState,
        targetMCParticlesToGoodHitsMap);

    MCParticleList mcPrimaryList;
    for (const auto &mapEntry : targetMCParticlesToGoodHitsMap)
    {
        mcPrimaryList.push_back(mapEntry.first);
    }
    mcPrimaryList.sort(LArMCParticleHelper::SortByMomentum);

    const LArInteractionTypeHelper::InteractionType interactionType(LArInteractionTypeHelper::GetInteractionType(mcPrimaryList));
    std::string intType = LArInteractionTypeHelper::ToString(interactionType);

    if (std::find(std::begin(m_interactions), std::end(m_interactions), intType) != std::end(m_interactions))
    {
        for (auto iter = pMCParticleList->begin(); iter != pMCParticleList->end(); ++iter)
        {
            const auto pMCParticle = *iter;
            if(LArMCParticleHelper::IsNeutrino(pMCParticle))
            {
                const CartesianVector& trueVertex = pMCParticle->GetEndpoint();
                m_file << trueVertex.GetX() << "," << trueVertex.GetY() << "," <<
                    trueVertex.GetZ() << ",";
            }
        }

        m_file << intType << ",";

        // Record the total number of calo hits
        m_file << pCaloHitListW->size() << ",";
        // Loop over all of the W calo hits and store their locations
        for (auto iter = pCaloHitListW->begin(); iter != pCaloHitListW->end(); ++iter)
        {
            const auto pCaloHit = *iter;
            const CartesianVector hitPos = pCaloHit->GetPositionVector();
            float energy = pCaloHit->GetInputEnergy();
            
            m_file << hitPos.GetX() << " " << hitPos.GetZ() << " " << energy << " ";
        }
        m_file << std::endl;
    }
    
    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode BScProjectInputsAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND,
        !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitListName",
        m_inputCaloHitListName));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND,
        !=, XmlHelper::ReadValue(xmlHandle, "InputCaloHitListNameW",
        m_inputCaloHitListNameW));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND,
        !=, XmlHelper::ReadValue(xmlHandle, "OutputFile", m_outputFilename));
        
    if(!m_outputFilename.empty())
    {
        m_file.open(m_outputFilename, std::ios::app);
        if(m_file.is_open())
        {
            //m_file << "true_vertex_x,true_vertex_y,true_vertex_z,interaction_type,num_hits,hits(x wire energy)" << std::endl;
        }
    }
            
    return STATUS_CODE_SUCCESS;
}

} // namespace lar_content
