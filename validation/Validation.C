/**
 *  @file   LArReco/validation/Validation.C
 *
 *  @brief  Implementation of validation functionality
 *
 *  $Log: $
 */
#include "TChain.h"
#include "TH1F.h"

#include "Validation.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

void Validation(const std::string &inputFiles, const Parameters &parameters)
{
    TChain *pTChain = new TChain("Validation", "pTChain");
    pTChain->Add(inputFiles.c_str());

    InteractionCountingMap interactionCountingMap;
    InteractionTargetResultMap interactionTargetResultMap;

    int nEvents(0), nProcessedEvents(0);
    const int nChainEntries(pTChain->GetEntries());

    for (int iEntry = 0; iEntry < nChainEntries; )
    {
        SimpleMCEvent simpleMCEvent;
        iEntry += ReadNextEvent(pTChain, iEntry, simpleMCEvent, parameters);

        if (nEvents++ < parameters.m_skipEvents)
            continue;

        if (nEvents % 50 == 0)
            std::cout << "nEvents " << nEvents << "\r" << std::flush;

        if (nProcessedEvents++ >= parameters.m_nEventsToProcess)
            break;

        if (parameters.m_displayMatchedEvents)
            DisplaySimpleMCEventMatches(simpleMCEvent, parameters);

        CountPfoMatches(simpleMCEvent, parameters, interactionCountingMap, interactionTargetResultMap);
    }

    DisplayInteractionCountingMap(interactionCountingMap, parameters);
    AnalyseInteractionTargetResultMap(interactionTargetResultMap, parameters);
}

//------------------------------------------------------------------------------------------------------------------------------------------

int ReadNextEvent(TChain *const pTChain, const int iEntry, SimpleMCEvent &simpleMCEvent, const Parameters &parameters)
{
    int thisEventNumber(0), iTarget(0);
    const int nChainEntries(pTChain->GetEntries());

    pTChain->SetBranchAddress("eventNumber", &thisEventNumber);
    pTChain->SetBranchAddress("fileIdentifier", &simpleMCEvent.m_fileIdentifier);
    pTChain->GetEntry(iEntry);
    simpleMCEvent.m_eventNumber = thisEventNumber;

    while (iEntry + iTarget < nChainEntries)
    {
        SimpleMCTarget simpleMCTarget;

        pTChain->SetBranchAddress("interactionType", &simpleMCTarget.m_interactionType);
        pTChain->SetBranchAddress("mcNuanceCode", &simpleMCTarget.m_mcNuanceCode);
        pTChain->SetBranchAddress("isCosmicRay", &simpleMCTarget.m_isCosmicRay);
        pTChain->SetBranchAddress("targetVertexX", &simpleMCTarget.m_targetVertex.m_x);
        pTChain->SetBranchAddress("targetVertexY", &simpleMCTarget.m_targetVertex.m_y);
        pTChain->SetBranchAddress("targetVertexZ", &simpleMCTarget.m_targetVertex.m_z);
        pTChain->SetBranchAddress("recoVertexX", &simpleMCTarget.m_recoVertex.m_x);
        pTChain->SetBranchAddress("recoVertexY", &simpleMCTarget.m_recoVertex.m_y);
        pTChain->SetBranchAddress("recoVertexZ", &simpleMCTarget.m_recoVertex.m_z);
        pTChain->SetBranchAddress("isCorrectCR", &simpleMCTarget.m_isCorrectCR);
        pTChain->SetBranchAddress("isFakeCR", &simpleMCTarget.m_isFakeCR);
        pTChain->SetBranchAddress("isSplitCR", &simpleMCTarget.m_isSplitCR);
        pTChain->SetBranchAddress("isLost", &simpleMCTarget.m_isLost);
        pTChain->SetBranchAddress("nTargetMatches", &simpleMCTarget.m_nTargetMatches);
        pTChain->SetBranchAddress("nTargetCRMatches", &simpleMCTarget.m_nTargetCRMatches);
        pTChain->SetBranchAddress("nTargetPrimaries", &simpleMCTarget.m_nTargetPrimaries);

        if (parameters.m_testBeamMode)
        {
            pTChain->SetBranchAddress("isBeamParticle", &simpleMCTarget.m_isBeamParticle);
            pTChain->SetBranchAddress("isCorrectTB", &simpleMCTarget.m_isCorrectTB);
        }
        else
        {
            pTChain->SetBranchAddress("isNeutrino", &simpleMCTarget.m_isNeutrino);
            pTChain->SetBranchAddress("isCorrectNu", &simpleMCTarget.m_isCorrectNu);
            pTChain->SetBranchAddress("isFakeNu", &simpleMCTarget.m_isFakeNu);
            pTChain->SetBranchAddress("isSplitNu", &simpleMCTarget.m_isSplitNu);
            pTChain->SetBranchAddress("nTargetNuMatches", &simpleMCTarget.m_nTargetNuMatches);
        }

        IntVector *pMCPrimaryId(nullptr), *pMCPrimaryPdg(nullptr), *pNMCHitsTotal(nullptr), *pNMCHitsU(nullptr), *pNMCHitsV(nullptr), *pNMCHitsW(nullptr);
        FloatVector *pMCPrimaryE(nullptr), *pMCPrimaryPX(nullptr), *pMCPrimaryPY(nullptr), *pMCPrimaryPZ(nullptr);
        FloatVector *pMCPrimaryVtxX(nullptr), *pMCPrimaryVtxY(nullptr), *pMCPrimaryVtxZ(nullptr), *pMCPrimaryEndX(nullptr), *pMCPrimaryEndY(nullptr), *pMCPrimaryEndZ(nullptr);
        IntVector *pNPrimaryMatchedPfos(nullptr), *pNPrimaryMatchedNuPfos(nullptr), *pNPrimaryMatchedCRPfos(nullptr);
        IntVector *pBestMatchPfoId(nullptr), *pBestMatchPfoPdg(nullptr), *pBestMatchPfoIsRecoNu(nullptr), *pBestMatchPfoRecoNuId(nullptr), *pBestMatchPfoIsTestBeam(nullptr);
        IntVector *pBestMatchPfoNHitsTotal(nullptr), *pBestMatchPfoNHitsU(nullptr), *pBestMatchPfoNHitsV(nullptr), *pBestMatchPfoNHitsW(nullptr);
        IntVector *pBestMatchPfoNSharedHitsTotal(nullptr), *pBestMatchPfoNSharedHitsU(nullptr), *pBestMatchPfoNSharedHitsV(nullptr), *pBestMatchPfoNSharedHitsW(nullptr);

        pTChain->SetBranchAddress("mcPrimaryId", &pMCPrimaryId);
        pTChain->SetBranchAddress("mcPrimaryPdg", &pMCPrimaryPdg);
        pTChain->SetBranchAddress("mcPrimaryE", &pMCPrimaryE);
        pTChain->SetBranchAddress("mcPrimaryPX", &pMCPrimaryPX);
        pTChain->SetBranchAddress("mcPrimaryPY", &pMCPrimaryPY);
        pTChain->SetBranchAddress("mcPrimaryPZ", &pMCPrimaryPZ);
        pTChain->SetBranchAddress("mcPrimaryVtxX", &pMCPrimaryVtxX);
        pTChain->SetBranchAddress("mcPrimaryVtxY", &pMCPrimaryVtxY);
        pTChain->SetBranchAddress("mcPrimaryVtxZ", &pMCPrimaryVtxZ);
        pTChain->SetBranchAddress("mcPrimaryEndX", &pMCPrimaryEndX);
        pTChain->SetBranchAddress("mcPrimaryEndY", &pMCPrimaryEndY);
        pTChain->SetBranchAddress("mcPrimaryEndZ", &pMCPrimaryEndZ);
        pTChain->SetBranchAddress("mcPrimaryNHitsTotal", &pNMCHitsTotal);
        pTChain->SetBranchAddress("mcPrimaryNHitsU", &pNMCHitsU);
        pTChain->SetBranchAddress("mcPrimaryNHitsV", &pNMCHitsV);
        pTChain->SetBranchAddress("mcPrimaryNHitsW", &pNMCHitsW);
        pTChain->SetBranchAddress("nPrimaryMatchedPfos", &pNPrimaryMatchedPfos);
        pTChain->SetBranchAddress("nPrimaryMatchedCRPfos", &pNPrimaryMatchedCRPfos);
        pTChain->SetBranchAddress("bestMatchPfoNHitsTotal", &pBestMatchPfoNHitsTotal);
        pTChain->SetBranchAddress("bestMatchPfoNHitsU", &pBestMatchPfoNHitsU);
        pTChain->SetBranchAddress("bestMatchPfoNHitsV", &pBestMatchPfoNHitsV);
        pTChain->SetBranchAddress("bestMatchPfoNHitsW", &pBestMatchPfoNHitsW);
        pTChain->SetBranchAddress("bestMatchPfoId", &pBestMatchPfoId);
        pTChain->SetBranchAddress("bestMatchPfoPdg", &pBestMatchPfoPdg);
        pTChain->SetBranchAddress("bestMatchPfoNSharedHitsTotal", &pBestMatchPfoNSharedHitsTotal);
        pTChain->SetBranchAddress("bestMatchPfoNSharedHitsU", &pBestMatchPfoNSharedHitsU);
        pTChain->SetBranchAddress("bestMatchPfoNSharedHitsV", &pBestMatchPfoNSharedHitsV);
        pTChain->SetBranchAddress("bestMatchPfoNSharedHitsW", &pBestMatchPfoNSharedHitsW);

        if (parameters.m_testBeamMode)
        {
            pTChain->SetBranchAddress("bestMatchPfoIsTB", &pBestMatchPfoIsTestBeam);
        }
        else
        {
            pTChain->SetBranchAddress("nPrimaryMatchedNuPfos", &pNPrimaryMatchedNuPfos);
            pTChain->SetBranchAddress("bestMatchPfoIsRecoNu", &pBestMatchPfoIsRecoNu);
            pTChain->SetBranchAddress("bestMatchPfoRecoNuId", &pBestMatchPfoRecoNuId);
            pTChain->SetBranchAddress("nTargetGoodNuMatches", &simpleMCTarget.m_nTargetGoodNuMatches);
            pTChain->SetBranchAddress("nTargetNuSplits", &simpleMCTarget.m_nTargetNuSplits);
            pTChain->SetBranchAddress("nTargetNuLosses", &simpleMCTarget.m_nTargetNuLosses);
        }

        pTChain->GetEntry(iEntry + iTarget++);

        if (simpleMCEvent.m_eventNumber != thisEventNumber)
            break;

        for (int iPrimary = 0; iPrimary < simpleMCTarget.m_nTargetPrimaries; ++iPrimary)
        {
            SimpleMCPrimary simpleMCPrimary;
            simpleMCPrimary.m_primaryId = pMCPrimaryId->at(iPrimary);
            simpleMCPrimary.m_pdgCode = pMCPrimaryPdg->at(iPrimary);
            simpleMCPrimary.m_energy = pMCPrimaryE->at(iPrimary);
            simpleMCPrimary.m_momentum.m_x = pMCPrimaryPX->at(iPrimary);
            simpleMCPrimary.m_momentum.m_y = pMCPrimaryPY->at(iPrimary);
            simpleMCPrimary.m_momentum.m_z = pMCPrimaryPZ->at(iPrimary);
            simpleMCPrimary.m_vertex.m_x = pMCPrimaryVtxX->at(iPrimary);
            simpleMCPrimary.m_vertex.m_y = pMCPrimaryVtxY->at(iPrimary);
            simpleMCPrimary.m_vertex.m_z = pMCPrimaryVtxZ->at(iPrimary);
            simpleMCPrimary.m_endpoint.m_x = pMCPrimaryEndX->at(iPrimary);
            simpleMCPrimary.m_endpoint.m_y = pMCPrimaryEndY->at(iPrimary);
            simpleMCPrimary.m_endpoint.m_z = pMCPrimaryEndZ->at(iPrimary);
            simpleMCPrimary.m_nMCHitsTotal = pNMCHitsTotal->at(iPrimary);
            simpleMCPrimary.m_nMCHitsU = pNMCHitsU->at(iPrimary);
            simpleMCPrimary.m_nMCHitsV = pNMCHitsV->at(iPrimary);
            simpleMCPrimary.m_nMCHitsW = pNMCHitsW->at(iPrimary);
            simpleMCPrimary.m_nPrimaryMatchedPfos = pNPrimaryMatchedPfos->at(iPrimary);
            simpleMCPrimary.m_nPrimaryMatchedCRPfos = pNPrimaryMatchedCRPfos->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoId = pBestMatchPfoId->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoPdgCode = pBestMatchPfoPdg->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNHitsTotal = pBestMatchPfoNHitsTotal->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNHitsU = pBestMatchPfoNHitsU->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNHitsV = pBestMatchPfoNHitsV->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNHitsW = pBestMatchPfoNHitsW->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNSharedHitsTotal = pBestMatchPfoNSharedHitsTotal->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNSharedHitsU = pBestMatchPfoNSharedHitsU->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNSharedHitsV = pBestMatchPfoNSharedHitsV->at(iPrimary);
            simpleMCPrimary.m_bestMatchPfoNSharedHitsW = pBestMatchPfoNSharedHitsW->at(iPrimary);

            if (parameters.m_testBeamMode)
            {
                simpleMCPrimary.m_bestMatchPfoIsTestBeam = pBestMatchPfoIsTestBeam->at(iPrimary);
            }
            else
            {
                simpleMCPrimary.m_nPrimaryMatchedNuPfos = pNPrimaryMatchedNuPfos->at(iPrimary);
                simpleMCPrimary.m_bestMatchPfoIsRecoNu = pBestMatchPfoIsRecoNu->at(iPrimary);
                simpleMCPrimary.m_bestMatchPfoRecoNuId = pBestMatchPfoRecoNuId->at(iPrimary);
            }

            simpleMCTarget.m_mcPrimaryList.push_back(simpleMCPrimary);
        }

        simpleMCEvent.m_mcTargetList.push_back(simpleMCTarget);
        simpleMCEvent.m_nMCTargets = simpleMCEvent.m_mcTargetList.size();
    }

    pTChain->ResetBranchAddresses();
    return simpleMCEvent.m_nMCTargets;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters)
{
    std::cout << "---INTERPRETED-MATCHING-OUTPUT------------------------------------------------------------------" << std::endl;
    std::cout << "File " << simpleMCEvent.m_fileIdentifier << ", event " << simpleMCEvent.m_eventNumber << std::endl;

    int nCorrectNu(0), nTotalNu(0), nCorrectTB(0), nTotalTB(0), nCorrectCR(0), nTotalCR(0), nFakeNu(0), nFakeCR(0), nSplitNu(0), nSplitCR(0), nLost(0);

    for (const SimpleMCTarget &simpleMCTarget : simpleMCEvent.m_mcTargetList)
    {
        std::cout << std::endl << ToString(static_cast<InteractionType>(simpleMCTarget.m_interactionType))
                  << " (Nuance " << simpleMCTarget.m_mcNuanceCode << ", Nu " << simpleMCTarget.m_isNeutrino;
        if (!PassFiducialCut(simpleMCTarget, parameters) && simpleMCTarget.m_isNeutrino) std::cout << " [NonFid]";
        std::cout << ", TB " << simpleMCTarget.m_isBeamParticle << ", CR " << simpleMCTarget.m_isCosmicRay << ")" << std::endl;

        std::stringstream ss;
        if (simpleMCTarget.m_isCorrectNu) ss << "IsCorrectNu ";
        if (simpleMCTarget.m_isCorrectTB) ss << "IsCorrectTB ";
        if (simpleMCTarget.m_isCorrectCR) ss << "IsCorrectCR ";
        if (simpleMCTarget.m_isFakeNu) ss << "IsFakeNu ";
        if (simpleMCTarget.m_isFakeCR) ss << "IsFakeCR ";
        if (simpleMCTarget.m_isSplitNu) ss << "IsSplitNu ";
        if (simpleMCTarget.m_isSplitCR) ss << "IsSplitCR ";
        if (simpleMCTarget.m_isLost) ss << "IsLost ";
        if (simpleMCTarget.m_nTargetNuMatches > 0) ss << "(NNuMatches: " << simpleMCTarget.m_nTargetNuMatches << ") ";
        if (simpleMCTarget.m_nTargetNuSplits > 0) ss << "(NNuSplits: " << simpleMCTarget.m_nTargetNuSplits << ") ";
        if (simpleMCTarget.m_nTargetNuLosses > 0) ss << "(NNuLosses: " << simpleMCTarget.m_nTargetNuLosses << ") ";
        if (simpleMCTarget.m_nTargetCRMatches > 0) ss << "(NCRMatches: " << simpleMCTarget.m_nTargetCRMatches << ") ";
        std::cout << ss.str() << std::endl;

        if (simpleMCTarget.m_isNeutrino) ++nTotalNu;
        if (simpleMCTarget.m_isBeamParticle) ++nTotalTB;
        if (simpleMCTarget.m_isCosmicRay) ++nTotalCR;
        if (simpleMCTarget.m_isCorrectNu) ++nCorrectNu;
        if (simpleMCTarget.m_isCorrectTB) ++nCorrectTB;
        if (simpleMCTarget.m_isCorrectCR) ++nCorrectCR;
        if (simpleMCTarget.m_isFakeNu) ++nFakeNu;
        if (simpleMCTarget.m_isFakeCR) ++nFakeCR;
        if (simpleMCTarget.m_isSplitNu) ++nSplitNu;
        if (simpleMCTarget.m_isSplitCR) ++nSplitCR;
        if (simpleMCTarget.m_isLost) ++nLost;

        for (const SimpleMCPrimary &simpleMCPrimary : simpleMCTarget.m_mcPrimaryList)
        {
            std::cout << "PrimaryId " << simpleMCPrimary.m_primaryId
                      << ", Nu " << simpleMCTarget.m_isNeutrino
                      << ", TB " << simpleMCTarget.m_isBeamParticle
                      << ", CR " << simpleMCTarget.m_isCosmicRay
                      << ", MCPDG " << simpleMCPrimary.m_pdgCode
                      << ", Energy " << simpleMCPrimary.m_energy
                      << ", Dist. " << std::sqrt(
                            (simpleMCPrimary.m_vertex.m_x - simpleMCPrimary.m_endpoint.m_x) * (simpleMCPrimary.m_vertex.m_x - simpleMCPrimary.m_endpoint.m_x) +
                            (simpleMCPrimary.m_vertex.m_y - simpleMCPrimary.m_endpoint.m_y) * (simpleMCPrimary.m_vertex.m_y - simpleMCPrimary.m_endpoint.m_y) +
                            (simpleMCPrimary.m_vertex.m_z - simpleMCPrimary.m_endpoint.m_z) * (simpleMCPrimary.m_vertex.m_z - simpleMCPrimary.m_endpoint.m_z))
                      << ", nMCHits " << simpleMCPrimary.m_nMCHitsTotal
                      << " (" << simpleMCPrimary.m_nMCHitsU
                      << ", " << simpleMCPrimary.m_nMCHitsV
                      << ", " << simpleMCPrimary.m_nMCHitsW << ")" << std::endl;

            if (0 == simpleMCPrimary.m_nPrimaryMatchedPfos)
            {
                std::cout << "-No matched Pfo" << std::endl;
                continue;
            }

            std::cout << "-MatchedPfoId " << simpleMCPrimary.m_bestMatchPfoId;
            if (simpleMCPrimary.m_nPrimaryMatchedPfos > 1) std::cout << " (NMatches " << simpleMCPrimary.m_nPrimaryMatchedPfos << ")";
            std::cout << ", Nu " << simpleMCPrimary.m_bestMatchPfoIsRecoNu;
            if (simpleMCPrimary.m_bestMatchPfoIsRecoNu) std::cout << " [NuId: " << simpleMCPrimary.m_bestMatchPfoRecoNuId << "]";
            std::cout << ", TB " << (simpleMCPrimary.m_bestMatchPfoIsTestBeam)
                      << ", CR " << (!simpleMCPrimary.m_bestMatchPfoIsRecoNu && !simpleMCPrimary.m_bestMatchPfoIsTestBeam)
                      << ", PDG " << simpleMCPrimary.m_bestMatchPfoPdgCode
                      << ", nMatchedHits " << simpleMCPrimary.m_bestMatchPfoNSharedHitsTotal
                      << " (" << simpleMCPrimary.m_bestMatchPfoNSharedHitsU
                      << ", " << simpleMCPrimary.m_bestMatchPfoNSharedHitsV
                      << ", " << simpleMCPrimary.m_bestMatchPfoNSharedHitsW << ")"
                      << ", nPfoHits " << simpleMCPrimary.m_bestMatchPfoNHitsTotal
                      << " (" << simpleMCPrimary.m_bestMatchPfoNHitsU
                      << ", " << simpleMCPrimary.m_bestMatchPfoNHitsV
                      << ", " << simpleMCPrimary.m_bestMatchPfoNHitsW << ")" << std::endl;
        }
    }

    std::stringstream summarySS;
    summarySS << std::endl << "---SUMMARY--------------------------------------------------------------------------------------" << std::endl;
    if (nTotalNu > 0) summarySS << "#CorrectNu: " << nCorrectNu << "/" << nTotalNu << ", Fraction: " << (nTotalNu > 0 ? static_cast<float>(nCorrectNu) / static_cast<float>(nTotalNu) : 0.f) << std::endl;
    if (nTotalTB > 0) summarySS << "#CorrectTB: " << nCorrectTB << "/" << nTotalTB << ", Fraction: " << (nTotalTB > 0 ? static_cast<float>(nCorrectTB) / static_cast<float>(nTotalTB) : 0.f) << std::endl;
    if (nTotalCR > 0) summarySS << "#CorrectCR: " << nCorrectCR << "/" << nTotalCR << ", Fraction: " << (nTotalCR > 0 ? static_cast<float>(nCorrectCR) / static_cast<float>(nTotalCR) : 0.f) << std::endl;
    if (nFakeNu > 0) summarySS << "#FakeNu: " << nFakeNu << " ";
    if (nFakeCR > 0) summarySS << "#FakeCR: " << nFakeCR << " ";
    if (nSplitNu > 0) summarySS << "#SplitNu: " << nSplitNu << " ";
    if (nSplitCR > 0) summarySS << "#SplitCR: " << nSplitCR << " ";
    if (nLost > 0) summarySS << "#Lost: " << nLost << " ";
    if (nFakeNu || nFakeCR || nSplitNu || nSplitCR || nLost) summarySS << std::endl;
    std::cout << summarySS.str();
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, InteractionCountingMap &interactionCountingMap,
    InteractionTargetResultMap &interactionTargetResultMap)
{
    for (const SimpleMCTarget &simpleMCTarget : simpleMCEvent.m_mcTargetList)
    {
        if ((!PassFiducialCut(simpleMCTarget, parameters) && simpleMCTarget.m_isNeutrino) ||
            (parameters.m_triggeredBeamOnly && simpleMCTarget.m_isBeamParticle && simpleMCTarget.m_mcNuanceCode != 2001))
            continue;

        TargetResult targetResult;
        targetResult.m_fileIdentifier = simpleMCEvent.m_fileIdentifier;
        targetResult.m_eventNumber = simpleMCEvent.m_eventNumber;
        targetResult.m_isCorrect = (simpleMCTarget.m_isNeutrino && simpleMCTarget.m_isCorrectNu) ||
            (simpleMCTarget.m_isBeamParticle && simpleMCTarget.m_isCorrectTB) ||
            (simpleMCTarget.m_isCosmicRay && simpleMCTarget.m_isCorrectCR);

        if (simpleMCTarget.m_nTargetMatches > 0)
        {
            targetResult.m_hasRecoVertex = true;
            targetResult.m_vertexOffset = simpleMCTarget.m_recoVertex - simpleMCTarget.m_targetVertex;
            targetResult.m_vertexOffset.m_x = targetResult.m_vertexOffset.m_x - parameters.m_vertexXCorrection;
        }

        const InteractionType interactionType(static_cast<InteractionType>(simpleMCTarget.m_interactionType));

        for (const SimpleMCPrimary &simpleMCPrimary : simpleMCTarget.m_mcPrimaryList)
        {
            const ExpectedPrimary expectedPrimary(GetExpectedPrimary(simpleMCPrimary, simpleMCTarget.m_mcPrimaryList));

            PrimaryResult &primaryResult = targetResult.m_primaryResultMap[expectedPrimary];
            CountingDetails &countingDetails = interactionCountingMap[interactionType][expectedPrimary];
            ++countingDetails.m_nTotal;

            // ATTN Fail cosmic ray matches to neutrinos (or beam particles) and vice versa
            bool incorrectMatchToCR(parameters.m_testBeamMode ? (simpleMCTarget.m_isCosmicRay == simpleMCPrimary.m_bestMatchPfoIsTestBeam) : (simpleMCTarget.m_isCosmicRay == simpleMCPrimary.m_bestMatchPfoIsRecoNu));

            if ((simpleMCPrimary.m_bestMatchPfoId >= 0) && incorrectMatchToCR)
            {
                ++countingDetails.m_nMatch0;
                continue;
            }

            if (0 == simpleMCPrimary.m_nPrimaryMatchedPfos) ++countingDetails.m_nMatch0;
            else if (1 == simpleMCPrimary.m_nPrimaryMatchedPfos) ++countingDetails.m_nMatch1;
            else if (2 == simpleMCPrimary.m_nPrimaryMatchedPfos) ++countingDetails.m_nMatch2;
            else ++countingDetails.m_nMatch3Plus;

            primaryResult.m_nPfoMatches = simpleMCPrimary.m_nPrimaryMatchedPfos;
            primaryResult.m_nMCHitsTotal = simpleMCPrimary.m_nMCHitsTotal;
            primaryResult.m_nBestMatchSharedHitsTotal = simpleMCPrimary.m_bestMatchPfoNSharedHitsTotal;
            primaryResult.m_nBestMatchRecoHitsTotal = simpleMCPrimary.m_bestMatchPfoNHitsTotal;
            primaryResult.m_bestMatchCompleteness = (simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMCPrimary.m_bestMatchPfoNSharedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0.f;
            primaryResult.m_bestMatchPurity = (simpleMCPrimary.m_bestMatchPfoNHitsTotal > 0) ? static_cast<float>(simpleMCPrimary.m_bestMatchPfoNSharedHitsTotal) / static_cast<float>(simpleMCPrimary.m_bestMatchPfoNHitsTotal) : 0.f;
            primaryResult.m_isCorrectParticleId = IsGoodParticleIdMatch(simpleMCPrimary, simpleMCPrimary.m_bestMatchPfoPdgCode);

            if ((simpleMCPrimary.m_nPrimaryMatchedPfos > 0) && primaryResult.m_isCorrectParticleId)
                ++countingDetails.m_correctId;

            if (parameters.m_correctTrackShowerId && !primaryResult.m_isCorrectParticleId)
                targetResult.m_isCorrect = false;

            const float pTot(std::sqrt(simpleMCPrimary.m_momentum.m_x * simpleMCPrimary.m_momentum.m_x + simpleMCPrimary.m_momentum.m_y * simpleMCPrimary.m_momentum.m_y + simpleMCPrimary.m_momentum.m_z * simpleMCPrimary.m_momentum.m_z));
            primaryResult.m_trueMomentum = pTot;
        }

        interactionTargetResultMap[interactionType].push_back(targetResult);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassFiducialCut(const SimpleMCTarget &simpleMCTarget, const Parameters &parameters)
{
    if (parameters.m_applyUbooneFiducialCut && parameters.m_applySBNDFiducialCut)
      throw std::invalid_argument("Parameters has fiducial cuts for uBooNE and SBND");

    if (parameters.m_applyUbooneFiducialCut)
        return PassUbooneFiducialCut(simpleMCTarget);

    if (parameters.m_applySBNDFiducialCut)
        return PassSBNDFiducialCut(simpleMCTarget);

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassUbooneFiducialCut(const SimpleMCTarget &simpleMCTarget)
{
    const float eVx(256.35), eVy(233.), eVz(1036.8);
    const float xBorder(10.), yBorder(20.), zBorder(10.);

    if ((simpleMCTarget.m_targetVertex.m_x < (eVx - xBorder)) && (simpleMCTarget.m_targetVertex.m_x > xBorder) &&
        (simpleMCTarget.m_targetVertex.m_y < (eVy / 2. - yBorder)) && (simpleMCTarget.m_targetVertex.m_y > (-eVy / 2. + yBorder)) &&
        (simpleMCTarget.m_targetVertex.m_z < (eVz - zBorder)) && (simpleMCTarget.m_targetVertex.m_z > zBorder) )
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassSBNDFiducialCut(const SimpleMCTarget &simpleMCTarget)
{
    const float eVx(400.f), eVy(400.f), eVz(500.f);
    const float xBorder(10.f), yBorder(20.f), zBorder(10.f);

    // ATTN origin definition is different in SBND to uBooNE. Both x & y are centered in the middle of the face
    if ((simpleMCTarget.m_targetVertex.m_x < (eVx / 2. - xBorder)) && (simpleMCTarget.m_targetVertex.m_x > (-eVx / 2. + xBorder)) &&
        (simpleMCTarget.m_targetVertex.m_y < (eVy / 2. - yBorder)) && (simpleMCTarget.m_targetVertex.m_y > (-eVy / 2. + yBorder)) &&
        (simpleMCTarget.m_targetVertex.m_z < (eVz - zBorder)) && (simpleMCTarget.m_targetVertex.m_z > zBorder) )
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

ExpectedPrimary GetExpectedPrimary(const SimpleMCPrimary &simpleMCPrimary, const SimpleMCPrimaryList &simpleMCPrimaryList)
{
    // ATTN: Relies on fact that primary list is sorted by number of good true hits
    unsigned int nMuons(0), nElectrons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (const SimpleMCPrimary &simpleMCPrimaryInList : simpleMCPrimaryList)
    {
        if (&simpleMCPrimary == &simpleMCPrimaryInList)
        {
            if ((0 == nMuons) && (13 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return MUON;
            if ((0 == nElectrons) && (11 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return ELECTRON;
            if ((0 == nProtons) && (2212 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return PROTON1;
            if ((1 == nProtons) && (2212 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return PROTON2;
            if ((2 == nProtons) && (2212 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return PROTON3;
            if ((3 == nProtons) && (2212 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return PROTON4;
            if ((4 == nProtons) && (2212 == std::fabs(simpleMCPrimaryInList.m_pdgCode))) return PROTON5;
            if ((0 == nPiPlus) && (211 == simpleMCPrimaryInList.m_pdgCode)) return PIPLUS;
            if ((0 == nPiMinus) && (-211 == simpleMCPrimaryInList.m_pdgCode)) return PIMINUS;
            if ((0 == nPhotons) && (22 == simpleMCPrimaryInList.m_pdgCode)) return PHOTON1;
            if ((1 == nPhotons) && (22 == simpleMCPrimaryInList.m_pdgCode)) return PHOTON2;
        }

        if (13 == std::fabs(simpleMCPrimaryInList.m_pdgCode)) ++nMuons;
        else if (11 == std::fabs(simpleMCPrimaryInList.m_pdgCode)) ++nElectrons;
        else if (2212 == std::fabs(simpleMCPrimaryInList.m_pdgCode)) ++nProtons;
        else if (211 == simpleMCPrimaryInList.m_pdgCode) ++nPiPlus;
        else if (-211 == simpleMCPrimaryInList.m_pdgCode) ++nPiMinus;
        else if (2112 == std::fabs(simpleMCPrimaryInList.m_pdgCode)) ++nNeutrons;
        else if (22 == simpleMCPrimaryInList.m_pdgCode) ++nPhotons;
    }

    return OTHER_PRIMARY;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool IsGoodParticleIdMatch(const SimpleMCPrimary &simpleMCPrimary, const int bestMatchPfoPdgCode)
{
    const unsigned int absMCPdgCode(std::fabs(simpleMCPrimary.m_pdgCode));

    if (((absMCPdgCode == 13 || absMCPdgCode == 2212 || absMCPdgCode == 211) && (13 != std::fabs(bestMatchPfoPdgCode) && 211 != std::fabs(bestMatchPfoPdgCode))) ||
        ((absMCPdgCode == 22 || absMCPdgCode == 11) && (11 != std::fabs(bestMatchPfoPdgCode))) )
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplayInteractionCountingMap(const InteractionCountingMap &interactionCountingMap, const Parameters &parameters)
{
    std::cout << std::fixed;
    std::cout << std::setprecision(1);

    std::ofstream mapFile;
    if (!parameters.m_mapFileName.empty()) mapFile.open(parameters.m_mapFileName, ios::app);

    for (const InteractionCountingMap::value_type &interactionTypeMapEntry : interactionCountingMap)
    {
        const InteractionType interactionType(interactionTypeMapEntry.first);
        const CountingMap &countingMap(interactionTypeMapEntry.second);
        std::cout << std::endl << ToString(interactionType) << std::endl;

        if (!parameters.m_mapFileName.empty())
            mapFile << std::endl << ToString(interactionType) << std::endl;

        for (const CountingMap::value_type &countingMapEntry : countingMap)
        {
            const ExpectedPrimary expectedPrimary(countingMapEntry.first);
            const CountingDetails &countingDetails(countingMapEntry.second);

            std::cout << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal
                      << ", nPfos |0: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch0) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, |1: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch1) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, |2: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch2) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, |3+: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch3Plus) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, correctId " << ((countingDetails.m_nTotal - countingDetails.m_nMatch0 > 0) ? 100.f * static_cast<float>(countingDetails.m_correctId) / static_cast<float>(countingDetails.m_nTotal - countingDetails.m_nMatch0) : 0.f)
                      <<  "%" << std::endl;

            if (!parameters.m_mapFileName.empty())
            {
                mapFile << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal
                        << ", nPfos |0: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch0) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |1: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch1) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |2: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch2) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |3+: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch3Plus) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, correctId " << ((countingDetails.m_nTotal - countingDetails.m_nMatch0 > 0) ? 100.f * static_cast<float>(countingDetails.m_correctId) / static_cast<float>(countingDetails.m_nTotal - countingDetails.m_nMatch0) : 0.f)
                        <<  "%" << std::endl;
            }
        }
    }

    if (!parameters.m_mapFileName.empty())
        mapFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalyseInteractionTargetResultMap(const InteractionTargetResultMap &interactionTargetResultMap, const Parameters &parameters)
{
    // Intended for filling histograms, post-processing of information collected in main loop over ntuple, etc.
    std::ofstream mapFile, eventFile;
    if (!parameters.m_mapFileName.empty()) mapFile.open(parameters.m_mapFileName, ios::app);
    if (!parameters.m_eventFileName.empty()) eventFile.open(parameters.m_eventFileName, ios::app);

    std::cout << std::endl << "EVENT INFO " << std::endl;
    mapFile << std::endl << "EVENT INFO " << std::endl;

    InteractionPrimaryHistogramMap interactionPrimaryHistogramMap;
    InteractionTargetHistogramMap interactionTargetHistogramMap;

    for (const InteractionTargetResultMap::value_type &interactionMapEntry : interactionTargetResultMap)
    {
        const InteractionType interactionType(interactionMapEntry.first);
        const TargetResultList &targetResultList(interactionMapEntry.second);

        unsigned int nCorrectEvents(0);

        for (const TargetResult &targetResult : targetResultList)
        {
            if (targetResult.m_isCorrect)
            {
                ++nCorrectEvents;

                if (!parameters.m_eventFileName.empty())
                    eventFile << "Correct event: fileId: " << targetResult.m_fileIdentifier << ", eventNumber: " << targetResult.m_eventNumber << ", interactionType " << ToString(interactionType) << std::endl;
            }

            const PrimaryResultMap &primaryResultMap(targetResult.m_primaryResultMap);

            for (const PrimaryResultMap::value_type &primaryMapEntry : primaryResultMap)
            {
                const ExpectedPrimary expectedPrimary(primaryMapEntry.first);
                const PrimaryResult &primaryResult(primaryMapEntry.second);

                if (parameters.m_histogramOutput)
                {
                    const std::string histPrefix(parameters.m_histPrefix + ToString(interactionType) + "_" + ToString(expectedPrimary) + "_");
                    PrimaryHistogramCollection &histogramCollection(interactionPrimaryHistogramMap[interactionType][expectedPrimary]);
                    FillPrimaryHistogramCollection(histPrefix, parameters, primaryResult, histogramCollection);

                    const std::string histPrefixAll(parameters.m_histPrefix + ToString(ALL_INTERACTIONS) + "_" + ToString(expectedPrimary) + "_");
                    PrimaryHistogramCollection &histogramCollectionAll(interactionPrimaryHistogramMap[ALL_INTERACTIONS][expectedPrimary]);
                    FillPrimaryHistogramCollection(histPrefixAll, parameters, primaryResult, histogramCollectionAll);
                }
            }

            if (parameters.m_histogramOutput)
            {
                const std::string histPrefix(parameters.m_histPrefix + ToString(interactionType) + "_");
                TargetHistogramCollection &histogramCollection(interactionTargetHistogramMap[interactionType]);
                FillTargetHistogramCollection(histPrefix, targetResult, histogramCollection);

                const std::string histPrefixAll(parameters.m_histPrefix + ToString(ALL_INTERACTIONS) + "_");
                TargetHistogramCollection &histogramCollectionAll(interactionTargetHistogramMap[ALL_INTERACTIONS]);
                FillTargetHistogramCollection(histPrefixAll, targetResult, histogramCollectionAll);
            }
        }

        std::cout << ToString(interactionType) << std::endl << "-nEvents " << targetResultList.size() << ", nCorrect " << nCorrectEvents
                  << ", fCorrect " << 100.f * static_cast<float>(nCorrectEvents) / static_cast<float>(targetResultList.size()) << "%" << std::endl;

        if (!parameters.m_mapFileName.empty())
        {
            mapFile << ToString(interactionType) << std::endl << "-nEvents " << targetResultList.size() << ", nCorrect " << nCorrectEvents
                    << ", fCorrect " << 100.f * static_cast<float>(nCorrectEvents) / static_cast<float>(targetResultList.size()) << "%" << std::endl;
        }
    }

    if (parameters.m_histogramOutput)
        ProcessHistogramCollections(interactionPrimaryHistogramMap);

    if (!parameters.m_mapFileName.empty()) mapFile.close();
    if (!parameters.m_eventFileName.empty()) eventFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillTargetHistogramCollection(const std::string &histPrefix, const TargetResult &targetResult, TargetHistogramCollection &targetHistogramCollection)
{
    if (!targetHistogramCollection.m_hVtxDeltaX)
    {
        targetHistogramCollection.m_hVtxDeltaX = new TH1F((histPrefix + "VtxDeltaX").c_str(), "", 40000, -2000., 2000.);
        targetHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetRangeUser(-5., +5.);
        targetHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetTitle("Vertex #DeltaX [cm]");
        targetHistogramCollection.m_hVtxDeltaX->GetYaxis()->SetTitle("Number of Events");
    }

    if (!targetHistogramCollection.m_hVtxDeltaY)
    {
        targetHistogramCollection.m_hVtxDeltaY = new TH1F((histPrefix + "VtxDeltaY").c_str(), "", 40000, -2000., 2000.);
        targetHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetRangeUser(-5., +5.);
        targetHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetTitle("Vertex #DeltaY [cm]");
        targetHistogramCollection.m_hVtxDeltaY->GetYaxis()->SetTitle("Number of Events");
    }

    if (!targetHistogramCollection.m_hVtxDeltaZ)
    {
        targetHistogramCollection.m_hVtxDeltaZ = new TH1F((histPrefix + "VtxDeltaZ").c_str(), "", 40000, -2000., 2000.);
        targetHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetRangeUser(-5., +5.);
        targetHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetTitle("Vertex #DeltaZ [cm]");
        targetHistogramCollection.m_hVtxDeltaZ->GetYaxis()->SetTitle("Number of Events");
    }

    if (!targetHistogramCollection.m_hVtxDeltaR)
    {
        targetHistogramCollection.m_hVtxDeltaR = new TH1F((histPrefix + "VtxDeltaR").c_str(), "", 40000, -100., 1900.);
        targetHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetRangeUser(0., +5.);
        targetHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetTitle("Vertex #DeltaR [cm]");
        targetHistogramCollection.m_hVtxDeltaR->GetYaxis()->SetTitle("Number of Events");
    }

    targetHistogramCollection.m_hVtxDeltaX->Fill(targetResult.m_vertexOffset.m_x);
    targetHistogramCollection.m_hVtxDeltaY->Fill(targetResult.m_vertexOffset.m_y);
    targetHistogramCollection.m_hVtxDeltaZ->Fill(targetResult.m_vertexOffset.m_z);
    targetHistogramCollection.m_hVtxDeltaR->Fill(std::sqrt(targetResult.m_vertexOffset.m_x * targetResult.m_vertexOffset.m_x + targetResult.m_vertexOffset.m_y * targetResult.m_vertexOffset.m_y + targetResult.m_vertexOffset.m_z * targetResult.m_vertexOffset.m_z));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillPrimaryHistogramCollection(const std::string &histPrefix, const Parameters &parameters, const PrimaryResult &primaryResult, PrimaryHistogramCollection &primaryHistogramCollection)
{
    const int nHitBins(35); const int nHitBinEdges(nHitBins + 1);
    float hitsBinning[nHitBinEdges];
    for (int n = 0; n < nHitBinEdges; ++n) hitsBinning[n] = std::pow(10., 1 + static_cast<float>(n + 2) / 10.);

    if (!primaryHistogramCollection.m_hHitsAll)
    {
        primaryHistogramCollection.m_hHitsAll = new TH1F((histPrefix + "HitsAll").c_str(), "", nHitBins, hitsBinning);
        primaryHistogramCollection.m_hHitsAll->GetXaxis()->SetRangeUser(1., +6000);
        primaryHistogramCollection.m_hHitsAll->GetXaxis()->SetTitle("Number of Hits");
        primaryHistogramCollection.m_hHitsAll->GetYaxis()->SetTitle("Number of Events");
    }

    if (!primaryHistogramCollection.m_hHitsEfficiency)
    {
        primaryHistogramCollection.m_hHitsEfficiency = new TH1F((histPrefix + "HitsEfficiency").c_str(), "", nHitBins, hitsBinning);
        primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->SetRangeUser(1., +6000);
        primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->SetTitle("Number of Hits");
        primaryHistogramCollection.m_hHitsEfficiency->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hHitsEfficiency->GetYaxis()->SetTitle("Reconstruction Efficiency");
    }

    const int nMomentumBins(26); const int nMomentumBinEdges(nMomentumBins + 1);
    float momentumBinning[nMomentumBinEdges] = {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.4, 1.6, 2.0, 2.4, 2.8, 3.4, 4., 5., 10., 15., 20., 30., 40., 50.};

    if (!primaryHistogramCollection.m_hMomentumAll)
    {
        primaryHistogramCollection.m_hMomentumAll = new TH1F((histPrefix + "MomentumAll").c_str(), "", nMomentumBins, momentumBinning);
        primaryHistogramCollection.m_hMomentumAll->GetXaxis()->SetRangeUser(0., +5.5);
        primaryHistogramCollection.m_hMomentumAll->GetXaxis()->SetTitle("True Momentum [GeV]");
        primaryHistogramCollection.m_hMomentumAll->GetYaxis()->SetTitle("Number of Events");
    }

    if (!primaryHistogramCollection.m_hMomentumEfficiency)
    {
        primaryHistogramCollection.m_hMomentumEfficiency = new TH1F((histPrefix + "MomentumEfficiency").c_str(), "", nMomentumBins, momentumBinning);
        primaryHistogramCollection.m_hMomentumEfficiency->GetXaxis()->SetRangeUser(1., +5.5);
        primaryHistogramCollection.m_hMomentumEfficiency->GetXaxis()->SetTitle("True Momentum [GeV]");
        primaryHistogramCollection.m_hMomentumEfficiency->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hMomentumEfficiency->GetYaxis()->SetTitle("Reconstruction Efficiency");
    }

    if (!primaryHistogramCollection.m_hCompleteness)
    {
        primaryHistogramCollection.m_hCompleteness = new TH1F((histPrefix + "Completeness").c_str(), "", 51, -0.01, 1.01);
        primaryHistogramCollection.m_hCompleteness->GetXaxis()->SetTitle("Completeness");
        primaryHistogramCollection.m_hCompleteness->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hCompleteness->GetYaxis()->SetTitle("Fraction of Events");
    }

    if (!primaryHistogramCollection.m_hPurity)
    {
        primaryHistogramCollection.m_hPurity = new TH1F((histPrefix + "Purity").c_str(), "", 51, -0.01, 1.01);
        primaryHistogramCollection.m_hPurity->GetXaxis()->SetTitle("Purity");
        primaryHistogramCollection.m_hPurity->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hPurity->GetYaxis()->SetTitle("Fraction of Events");
    }

    primaryHistogramCollection.m_hHitsAll->Fill(primaryResult.m_nMCHitsTotal);
    primaryHistogramCollection.m_hMomentumAll->Fill(primaryResult.m_trueMomentum);

    if ((primaryResult.m_nPfoMatches > 0) &&
        (!parameters.m_correctTrackShowerId || primaryResult.m_isCorrectParticleId))
    {
        primaryHistogramCollection.m_hHitsEfficiency->Fill(primaryResult.m_nMCHitsTotal);
        primaryHistogramCollection.m_hMomentumEfficiency->Fill(primaryResult.m_trueMomentum);
        primaryHistogramCollection.m_hCompleteness->Fill(primaryResult.m_bestMatchCompleteness);
        primaryHistogramCollection.m_hPurity->Fill(primaryResult.m_bestMatchPurity);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ProcessHistogramCollections(const InteractionPrimaryHistogramMap &interactionPrimaryHistogramMap)
{
    for (InteractionPrimaryHistogramMap::const_iterator iter = interactionPrimaryHistogramMap.begin(), iterEnd = interactionPrimaryHistogramMap.end(); iter != iterEnd; ++iter)
    {
        const InteractionType interactionType(iter->first);
        const PrimaryHistogramMap &primaryHistogramMap(iter->second);

        for (PrimaryHistogramMap::const_iterator hIter = primaryHistogramMap.begin(), hIterEnd = primaryHistogramMap.end(); hIter != hIterEnd; ++hIter)
        {
            const ExpectedPrimary expectedPrimary(hIter->first);
            const PrimaryHistogramCollection &primaryHistogramCollection(hIter->second);

            for (int n = -1; n <= primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->GetNbins(); ++n)
            {
                const float found = primaryHistogramCollection.m_hHitsEfficiency->GetBinContent(n + 1);
                const float all = primaryHistogramCollection.m_hHitsAll->GetBinContent(n + 1);
                const float efficiency = (all > 0.f) ? found / all : 0.f;
                const float error = (all > found) ? std::sqrt(efficiency * (1. - efficiency) / all) : 0.f;
                primaryHistogramCollection.m_hHitsEfficiency->SetBinContent(n + 1, efficiency);
                primaryHistogramCollection.m_hHitsEfficiency->SetBinError(n + 1, error);
            }

            for (int n = -1; n <= primaryHistogramCollection.m_hMomentumEfficiency->GetXaxis()->GetNbins(); ++n)
            {
                const float found = primaryHistogramCollection.m_hMomentumEfficiency->GetBinContent(n + 1);
                const float all = primaryHistogramCollection.m_hMomentumAll->GetBinContent(n + 1);
                const float efficiency = (all > 0.f) ? found / all : 0.f;
                const float error = (all > found) ? std::sqrt(efficiency * (1. - efficiency) / all) : 0.f;
                primaryHistogramCollection.m_hMomentumEfficiency->SetBinContent(n + 1, efficiency);
                primaryHistogramCollection.m_hMomentumEfficiency->SetBinError(n + 1, error);
            }

            primaryHistogramCollection.m_hCompleteness->Scale(1. / static_cast<double>(primaryHistogramCollection.m_hCompleteness->GetEntries()));
            primaryHistogramCollection.m_hPurity->Scale(1. / static_cast<double>(primaryHistogramCollection.m_hPurity->GetEntries()));
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

std::string ToString(const ExpectedPrimary expectedPrimary)
{
    switch (expectedPrimary)
    {
    case MUON : return "MUON";
    case ELECTRON : return "ELECTRON";
    case PROTON1 : return "PROTON1";
    case PROTON2 : return "PROTON2";
    case PROTON3 : return "PROTON3";
    case PROTON4 : return "PROTON4";
    case PROTON5 : return "PROTON5";
    case PIPLUS : return "PIPLUS";
    case PIMINUS : return "PIMINUS";
    case NEUTRON : return "NEUTRON";
    case PHOTON1 : return "PHOTON1";
    case PHOTON2 : return "PHOTON2";
    case OTHER_PRIMARY: return "OTHER_PRIMARY";
    default: return "UNKNOWN";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

std::string ToString(const InteractionType interactionType)
{
    switch (interactionType)
    {
    case CCQEL_MU: return "CCQEL_MU";
    case CCQEL_MU_P: return "CCQEL_MU_P";
    case CCQEL_MU_P_P: return "CCQEL_MU_P_P";
    case CCQEL_MU_P_P_P: return "CCQEL_MU_P_P_P";
    case CCQEL_MU_P_P_P_P: return "CCQEL_MU_P_P_P_P";
    case CCQEL_MU_P_P_P_P_P: return "CCQEL_MU_P_P_P_P_P";
    case CCQEL_E: return "CCQEL_E";
    case CCQEL_E_P: return "CCQEL_E_P";
    case CCQEL_E_P_P: return "CCQEL_E_P_P";
    case CCQEL_E_P_P_P: return "CCQEL_E_P_P_P";
    case CCQEL_E_P_P_P_P: return "CCQEL_E_P_P_P_P";
    case CCQEL_E_P_P_P_P_P: return "CCQEL_E_P_P_P_P_P";
    case NCQEL_P: return "NCQEL_P";
    case NCQEL_P_P: return "NCQEL_P_P";
    case NCQEL_P_P_P: return "NCQEL_P_P_P";
    case NCQEL_P_P_P_P: return "NCQEL_P_P_P_P";
    case NCQEL_P_P_P_P_P: return "NCQEL_P_P_P_P_P";
    case CCRES_MU: return "CCRES_MU";
    case CCRES_MU_P: return "CCRES_MU_P";
    case CCRES_MU_P_P: return "CCRES_MU_P_P";
    case CCRES_MU_P_P_P: return "CCRES_MU_P_P_P";
    case CCRES_MU_P_P_P_P: return "CCRES_MU_P_P_P_P";
    case CCRES_MU_P_P_P_P_P: return "CCRES_MU_P_P_P_P_P";
    case CCRES_MU_PIPLUS: return "CCRES_MU_PIPLUS";
    case CCRES_MU_P_PIPLUS: return "CCRES_MU_P_PIPLUS";
    case CCRES_MU_P_P_PIPLUS: return "CCRES_MU_P_P_PIPLUS";
    case CCRES_MU_P_P_P_PIPLUS: return "CCRES_MU_P_P_P_PIPLUS";
    case CCRES_MU_P_P_P_P_PIPLUS: return "CCRES_MU_P_P_P_P_PIPLUS";
    case CCRES_MU_P_P_P_P_P_PIPLUS: return "CCRES_MU_P_P_P_P_P_PIPLUS";
    case CCRES_MU_PHOTON: return "CCRES_MU_PHOTON";
    case CCRES_MU_P_PHOTON: return "CCRES_MU_P_PHOTON";
    case CCRES_MU_P_P_PHOTON: return "CCRES_MU_P_P_PHOTON";
    case CCRES_MU_P_P_P_PHOTON: return "CCRES_MU_P_P_P_PHOTON";
    case CCRES_MU_P_P_P_P_PHOTON: return "CCRES_MU_P_P_P_P_PHOTON";
    case CCRES_MU_P_P_P_P_P_PHOTON: return "CCRES_MU_P_P_P_P_P_PHOTON";
    case CCRES_MU_PIZERO: return "CCRES_MU_PIZERO";
    case CCRES_MU_P_PIZERO: return "CCRES_MU_P_PIZERO";
    case CCRES_MU_P_P_PIZERO: return "CCRES_MU_P_P_PIZERO";
    case CCRES_MU_P_P_P_PIZERO: return "CCRES_MU_P_P_P_PIZERO";
    case CCRES_MU_P_P_P_P_PIZERO: return "CCRES_MU_P_P_P_P_PIZERO";
    case CCRES_MU_P_P_P_P_P_PIZERO: return "CCRES_MU_P_P_P_P_P_PIZERO";
    case CCRES_E: return "CCRES_E";
    case CCRES_E_P: return "CCRES_E_P";
    case CCRES_E_P_P: return "CCRES_E_P_P";
    case CCRES_E_P_P_P: return "CCRES_E_P_P_P";
    case CCRES_E_P_P_P_P: return "CCRES_E_P_P_P_P";
    case CCRES_E_P_P_P_P_P: return "CCRES_E_P_P_P_P_P";
    case CCRES_E_PIPLUS: return "CCRES_E_PIPLUS";
    case CCRES_E_P_PIPLUS: return "CCRES_E_P_PIPLUS";
    case CCRES_E_P_P_PIPLUS: return "CCRES_E_P_P_PIPLUS";
    case CCRES_E_P_P_P_PIPLUS: return "CCRES_E_P_P_P_PIPLUS";
    case CCRES_E_P_P_P_P_PIPLUS: return "CCRES_E_P_P_P_P_PIPLUS";
    case CCRES_E_P_P_P_P_P_PIPLUS: return "CCRES_E_P_P_P_P_P_PIPLUS";
    case CCRES_E_PHOTON: return "CCRES_E_PHOTON";
    case CCRES_E_P_PHOTON: return "CCRES_E_P_PHOTON";
    case CCRES_E_P_P_PHOTON: return "CCRES_E_P_P_PHOTON";
    case CCRES_E_P_P_P_PHOTON: return "CCRES_E_P_P_P_PHOTON";
    case CCRES_E_P_P_P_P_PHOTON: return "CCRES_E_P_P_P_P_PHOTON";
    case CCRES_E_P_P_P_P_P_PHOTON: return "CCRES_E_P_P_P_P_P_PHOTON";
    case CCRES_E_PIZERO: return "CCRES_E_PIZERO";
    case CCRES_E_P_PIZERO: return "CCRES_E_P_PIZERO";
    case CCRES_E_P_P_PIZERO: return "CCRES_E_P_P_PIZERO";
    case CCRES_E_P_P_P_PIZERO: return "CCRES_E_P_P_P_PIZERO";
    case CCRES_E_P_P_P_P_PIZERO: return "CCRES_E_P_P_P_P_PIZERO";
    case CCRES_E_P_P_P_P_P_PIZERO: return "CCRES_E_P_P_P_P_P_PIZERO";
    case NCRES_P: return "NCRES_P";
    case NCRES_P_P: return "NCRES_P_P";
    case NCRES_P_P_P: return "NCRES_P_P_P";
    case NCRES_P_P_P_P: return "NCRES_P_P_P_P";
    case NCRES_P_P_P_P_P: return "NCRES_P_P_P_P_P";
    case NCRES_PIPLUS: return "NCRES_PIPLUS";
    case NCRES_P_PIPLUS: return "NCRES_P_PIPLUS";
    case NCRES_P_P_PIPLUS: return "NCRES_P_P_PIPLUS";
    case NCRES_P_P_P_PIPLUS: return "NCRES_P_P_P_PIPLUS";
    case NCRES_P_P_P_P_PIPLUS: return "NCRES_P_P_P_P_PIPLUS";
    case NCRES_P_P_P_P_P_PIPLUS: return "NCRES_P_P_P_P_P_PIPLUS";
    case NCRES_PIMINUS: return "NCRES_PIMINUS";
    case NCRES_P_PIMINUS: return "NCRES_P_PIMINUS";
    case NCRES_P_P_PIMINUS: return "NCRES_P_P_PIMINUS";
    case NCRES_P_P_P_PIMINUS: return "NCRES_P_P_P_PIMINUS";
    case NCRES_P_P_P_P_PIMINUS: return "NCRES_P_P_P_P_PIMINUS";
    case NCRES_P_P_P_P_P_PIMINUS: return "NCRES_P_P_P_P_P_PIMINUS";
    case NCRES_PHOTON: return "NCRES_PHOTON";
    case NCRES_P_PHOTON: return "NCRES_P_PHOTON";
    case NCRES_P_P_PHOTON: return "NCRES_P_P_PHOTON";
    case NCRES_P_P_P_PHOTON: return "NCRES_P_P_P_PHOTON";
    case NCRES_P_P_P_P_PHOTON: return "NCRES_P_P_P_P_PHOTON";
    case NCRES_P_P_P_P_P_PHOTON: return "NCRES_P_P_P_P_P_PHOTON";
    case NCRES_PIZERO: return "NCRES_PIZERO";
    case NCRES_P_PIZERO: return "NCRES_P_PIZERO";
    case NCRES_P_P_PIZERO: return "NCRES_P_P_PIZERO";
    case NCRES_P_P_P_PIZERO: return "NCRES_P_P_P_PIZERO";
    case NCRES_P_P_P_P_PIZERO: return "NCRES_P_P_P_P_PIZERO";
    case NCRES_P_P_P_P_P_PIZERO: return "NCRES_P_P_P_P_P_PIZERO";
    case CCDIS_MU: return "CCDIS_MU";
    case CCDIS_MU_P: return "CCDIS_MU_P";
    case CCDIS_MU_P_P: return "CCDIS_MU_P_P";
    case CCDIS_MU_P_P_P: return "CCDIS_MU_P_P_P";
    case CCDIS_MU_P_P_P_P: return "CCDIS_MU_P_P_P_P";
    case CCDIS_MU_P_P_P_P_P: return "CCDIS_MU_P_P_P_P_P";
    case CCDIS_MU_PIPLUS: return "CCDIS_MU_PIPLUS";
    case CCDIS_MU_P_PIPLUS: return "CCDIS_MU_P_PIPLUS";
    case CCDIS_MU_P_P_PIPLUS: return "CCDIS_MU_P_P_PIPLUS";
    case CCDIS_MU_P_P_P_PIPLUS: return "CCDIS_MU_P_P_P_PIPLUS";
    case CCDIS_MU_P_P_P_P_PIPLUS: return "CCDIS_MU_P_P_P_P_PIPLUS";
    case CCDIS_MU_P_P_P_P_P_PIPLUS: return "CCDIS_MU_P_P_P_P_P_PIPLUS";
    case CCDIS_MU_PHOTON: return "CCDIS_MU_PHOTON";
    case CCDIS_MU_P_PHOTON: return "CCDIS_MU_P_PHOTON";
    case CCDIS_MU_P_P_PHOTON: return "CCDIS_MU_P_P_PHOTON";
    case CCDIS_MU_P_P_P_PHOTON: return "CCDIS_MU_P_P_P_PHOTON";
    case CCDIS_MU_P_P_P_P_PHOTON: return "CCDIS_MU_P_P_P_P_PHOTON";
    case CCDIS_MU_P_P_P_P_P_PHOTON: return "CCDIS_MU_P_P_P_P_P_PHOTON";
    case CCDIS_MU_PIZERO: return "CCDIS_MU_PIZERO";
    case CCDIS_MU_P_PIZERO: return "CCDIS_MU_P_PIZERO";
    case CCDIS_MU_P_P_PIZERO: return "CCDIS_MU_P_P_PIZERO";
    case CCDIS_MU_P_P_P_PIZERO: return "CCDIS_MU_P_P_P_PIZERO";
    case CCDIS_MU_P_P_P_P_PIZERO: return "CCDIS_MU_P_P_P_P_PIZERO";
    case CCDIS_MU_P_P_P_P_P_PIZERO: return "CCDIS_MU_P_P_P_P_P_PIZERO";
    case NCDIS_P: return "NCDIS_P";
    case NCDIS_P_P: return "NCDIS_P_P";
    case NCDIS_P_P_P: return "NCDIS_P_P_P";
    case NCDIS_P_P_P_P: return "NCDIS_P_P_P_P";
    case NCDIS_P_P_P_P_P: return "NCDIS_P_P_P_P_P";
    case NCDIS_PIPLUS: return "NCDIS_PIPLUS";
    case NCDIS_P_PIPLUS: return "NCDIS_P_PIPLUS";
    case NCDIS_P_P_PIPLUS: return "NCDIS_P_P_PIPLUS";
    case NCDIS_P_P_P_PIPLUS: return "NCDIS_P_P_P_PIPLUS";
    case NCDIS_P_P_P_P_PIPLUS: return "NCDIS_P_P_P_P_PIPLUS";
    case NCDIS_P_P_P_P_P_PIPLUS: return "NCDIS_P_P_P_P_P_PIPLUS";
    case NCDIS_PIMINUS: return "NCDIS_PIMINUS";
    case NCDIS_P_PIMINUS: return "NCDIS_P_PIMINUS";
    case NCDIS_P_P_PIMINUS: return "NCDIS_P_P_PIMINUS";
    case NCDIS_P_P_P_PIMINUS: return "NCDIS_P_P_P_PIMINUS";
    case NCDIS_P_P_P_P_PIMINUS: return "NCDIS_P_P_P_P_PIMINUS";
    case NCDIS_P_P_P_P_P_PIMINUS: return "NCDIS_P_P_P_P_P_PIMINUS";
    case NCDIS_PHOTON: return "NCDIS_PHOTON";
    case NCDIS_P_PHOTON: return "NCDIS_P_PHOTON";
    case NCDIS_P_P_PHOTON: return "NCDIS_P_P_PHOTON";
    case NCDIS_P_P_P_PHOTON: return "NCDIS_P_P_P_PHOTON";
    case NCDIS_P_P_P_P_PHOTON: return "NCDIS_P_P_P_P_PHOTON";
    case NCDIS_P_P_P_P_P_PHOTON: return "NCDIS_P_P_P_P_P_PHOTON";
    case NCDIS_PIZERO: return "NCDIS_PIZERO";
    case NCDIS_P_PIZERO: return "NCDIS_P_PIZERO";
    case NCDIS_P_P_PIZERO: return "NCDIS_P_P_PIZERO";
    case NCDIS_P_P_P_PIZERO: return "NCDIS_P_P_P_PIZERO";
    case NCDIS_P_P_P_P_PIZERO: return "NCDIS_P_P_P_P_PIZERO";
    case NCDIS_P_P_P_P_P_PIZERO: return "NCDIS_P_P_P_P_P_PIZERO";
    case CCCOH: return "CCCOH";
    case NCCOH: return "NCCOH";
    case COSMIC_RAY_MU: return "COSMIC_RAY_MU";
    case COSMIC_RAY_P: return "COSMIC_RAY_P";
    case COSMIC_RAY_E: return "COSMIC_RAY_E";
    case COSMIC_RAY_PHOTON: return "COSMIC_RAY_PHOTON";
    case COSMIC_RAY_OTHER: return "COSMIC_RAY_OTHER";
    case BEAM_PARTICLE_MU: return "BEAM_PARTICLE_MU";
    case BEAM_PARTICLE_P: return "BEAM_PARTICLE_P";
    case BEAM_PARTICLE_E: return "BEAM_PARTICLE_E";
    case BEAM_PARTICLE_PHOTON: return "BEAM_PARTICLE_PHOTON";
    case BEAM_PARTICLE_PI_PLUS: return "BEAM_PARTICLE_PI_PLUS";
    case BEAM_PARTICLE_PI_MINUS: return "BEAM_PARTICLE_PI_MINUS";
    case BEAM_PARTICLE_KAON_PLUS: return "BEAM_PARTICLE_KAON_PLUS";
    case BEAM_PARTICLE_KAON_MINUS: return "BEAM_PARTICLE_KAON_MINUS";
    case BEAM_PARTICLE_OTHER: return "BEAM_PARTICLE_OTHER";
    case OTHER_INTERACTION: return "OTHER_INTERACTION";
    case ALL_INTERACTIONS: return "ALL_INTERACTIONS";
    default: return "UNKNOWN";
    }
}
