/**
 *  @file   LArReco/validation/Validation.C
 *
 *  @brief  Implementation of validation functionality
 *
 *  $Log: $
 */
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"

#include "Validation.h"

#include <iostream>
#include <iomanip>

void Validation(const std::string inputFiles, const bool shouldDisplayEvents, const bool shouldDisplayMatchedEvents, const int skipEvents,
    const int nEventsToProcess, const int minPrimaryHits, const int minSharedHits, const bool histogramOutput, const bool correctTrackShowerId,
    const bool applyFiducialCut, const std::string histPrefix, const std::string mapFileName, const std::string eventFileName,
    const bool useSmallPrimaries, const float minCompleteness, const float minPurity) // TODO Reduce number of arguments to avoid confusion
{
    const MatchingParameters matchingParameters(minPrimaryHits, useSmallPrimaries, minSharedHits, minCompleteness, minPurity, applyFiducialCut, correctTrackShowerId);

    TChain *pTChain = new TChain("Validation", "pTChain");
    pTChain->Add(inputFiles.c_str());

    // To store final output
    InteractionCountingMap interactionCountingMap;
    InteractionEventResultMap interactionEventResultMap;

    int nEvents(0), nProcessedEvents(0);

    for (int iEntry = 0; iEntry < pTChain->GetEntries(); )
    {
        SimpleMCEvent simpleMCEvent;
        iEntry += ValidationIO::ReadNextEvent(pTChain, iEntry, simpleMCEvent);

        if (nEvents++ < skipEvents)
            continue;

        if (nEvents % 50 == 0)
            std::cout << "nEvents " << nEvents << "\r" << std::flush;

        if (nProcessedEvents++ >= nEventsToProcess)
            break;

        if (shouldDisplayEvents)
            ValidationIO::DisplaySimpleMCEvent(simpleMCEvent);

        // Analysis code starts here
        PfoMatchingMap pfoMatchingMap;
        FinalisePfoMatching(simpleMCEvent, matchingParameters, pfoMatchingMap);

        if (shouldDisplayMatchedEvents)
            DisplaySimpleMCEventMatches(simpleMCEvent, pfoMatchingMap, matchingParameters);

        const InteractionType interactionType(GetInteractionType(simpleMCEvent, matchingParameters));
        CountPfoMatches(simpleMCEvent, interactionType, pfoMatchingMap, matchingParameters, interactionCountingMap, interactionEventResultMap);
    }

    // Processing of final output
    DisplayInteractionCountingMap(interactionCountingMap, matchingParameters, mapFileName);
    AnalyseInteractionEventResultMap(interactionEventResultMap, mapFileName, eventFileName, histogramOutput, histPrefix);
}

//------------------------------------------------------------------------------------------------------------------------------------------

InteractionType GetInteractionType(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters)
{
    unsigned int nNonNeutrons(0), nMuons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (simpleMCPrimary.m_nGoodMCHitsTotal < matchingParameters.m_minPrimaryHits)
            continue;

        if (2112 != simpleMCPrimary.m_pdgCode)
            ++nNonNeutrons;

        if (13 == simpleMCPrimary.m_pdgCode) ++nMuons;
        else if (2212 == simpleMCPrimary.m_pdgCode) ++nProtons;
        else if (22 == simpleMCPrimary.m_pdgCode) ++nPhotons;
        else if (211 == simpleMCPrimary.m_pdgCode) ++nPiPlus;
        else if (-211 == simpleMCPrimary.m_pdgCode) ++nPiMinus;
        else if (2112 == simpleMCPrimary.m_pdgCode) ++nNeutrons;
    }

    InteractionType interactionType(OTHER_INTERACTION);

    if (1001 == simpleMCEvent.m_mcNeutrinoNuance)
    {
        if ((1 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons)) return CCQEL_MU;
        if ((2 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons)) return CCQEL_MU_P;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons)) return CCQEL_MU_P_P;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons)) return CCQEL_MU_P_P_P;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons)) return CCQEL_MU_P_P_P_P;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons)) return CCQEL_MU_P_P_P_P_P;
    }

    if (1002 == simpleMCEvent.m_mcNeutrinoNuance)
    {
        if ((1 == nNonNeutrons) && (1 == nProtons)) return NCQEL_P;
        if ((2 == nNonNeutrons) && (2 == nProtons)) return NCQEL_P_P;
        if ((3 == nNonNeutrons) && (3 == nProtons)) return NCQEL_P_P_P;
        if ((4 == nNonNeutrons) && (4 == nProtons)) return NCQEL_P_P_P_P;
        if ((5 == nNonNeutrons) && (5 == nProtons)) return NCQEL_P_P_P_P_P;
    }

    if ((simpleMCEvent.m_mcNeutrinoNuance >= 1003) && (simpleMCEvent.m_mcNeutrinoNuance <= 1005))
    {
        if ((1 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons)) return CCRES_MU;
        if ((2 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons)) return CCRES_MU_P;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons)) return CCRES_MU_P_P;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons)) return CCRES_MU_P_P_P;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons)) return CCRES_MU_P_P_P_P;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons)) return CCRES_MU_P_P_P_P_P;

        if ((2 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons) && (1 == nPiPlus)) return CCRES_MU_PIPLUS;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (1 == nPiPlus)) return CCRES_MU_P_PIPLUS;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (1 == nPiPlus)) return CCRES_MU_P_P_PIPLUS;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons) && (1 == nPiPlus)) return CCRES_MU_P_P_P_PIPLUS;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons) && (1 == nPiPlus)) return CCRES_MU_P_P_P_P_PIPLUS;
        if ((7 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons) && (1 == nPiPlus)) return CCRES_MU_P_P_P_P_P_PIPLUS;

        if ((2 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons) && (1 == nPhotons)) return CCRES_MU_PHOTON;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (1 == nPhotons)) return CCRES_MU_P_PHOTON;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (1 == nPhotons)) return CCRES_MU_P_P_PHOTON;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons) && (1 == nPhotons)) return CCRES_MU_P_P_P_PHOTON;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons) && (1 == nPhotons)) return CCRES_MU_P_P_P_P_PHOTON;
        if ((7 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons) && (1 == nPhotons)) return CCRES_MU_P_P_P_P_P_PHOTON;

        if ((3 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons) && (2 == nPhotons)) return CCRES_MU_PIZERO;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (2 == nPhotons)) return CCRES_MU_P_PIZERO;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (2 == nPhotons)) return CCRES_MU_P_P_PIZERO;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons) && (2 == nPhotons)) return CCRES_MU_P_P_P_PIZERO;
        if ((7 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons) && (2 == nPhotons)) return CCRES_MU_P_P_P_P_PIZERO;
        if ((8 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons) && (2 == nPhotons)) return CCRES_MU_P_P_P_P_P_PIZERO;
    }

    if ((simpleMCEvent.m_mcNeutrinoNuance >= 1006) && (simpleMCEvent.m_mcNeutrinoNuance <= 1009))
    {
        if ((1 == nNonNeutrons) && (0 == nProtons) && (1 == nPiPlus)) return NCRES_PIPLUS;
        if ((2 == nNonNeutrons) && (1 == nProtons) && (1 == nPiPlus)) return NCRES_P_PIPLUS;
        if ((3 == nNonNeutrons) && (2 == nProtons) && (1 == nPiPlus)) return NCRES_P_P_PIPLUS;
        if ((4 == nNonNeutrons) && (3 == nProtons) && (1 == nPiPlus)) return NCRES_P_P_P_PIPLUS;
        if ((5 == nNonNeutrons) && (4 == nProtons) && (1 == nPiPlus)) return NCRES_P_P_P_P_PIPLUS;
        if ((6 == nNonNeutrons) && (5 == nProtons) && (1 == nPiPlus)) return NCRES_P_P_P_P_P_PIPLUS;

        if ((1 == nNonNeutrons) && (0 == nProtons) && (1 == nPiMinus)) return NCRES_PIMINUS;
        if ((2 == nNonNeutrons) && (1 == nProtons) && (1 == nPiMinus)) return NCRES_P_PIMINUS;
        if ((3 == nNonNeutrons) && (2 == nProtons) && (1 == nPiMinus)) return NCRES_P_P_PIMINUS;
        if ((4 == nNonNeutrons) && (3 == nProtons) && (1 == nPiMinus)) return NCRES_P_P_P_PIMINUS;
        if ((5 == nNonNeutrons) && (4 == nProtons) && (1 == nPiMinus)) return NCRES_P_P_P_P_PIMINUS;
        if ((6 == nNonNeutrons) && (5 == nProtons) && (1 == nPiMinus)) return NCRES_P_P_P_P_P_PIMINUS;

        if ((1 == nNonNeutrons) && (0 == nProtons) && (1 == nPhotons)) return NCRES_PHOTON;
        if ((2 == nNonNeutrons) && (1 == nProtons) && (1 == nPhotons)) return NCRES_P_PHOTON;
        if ((3 == nNonNeutrons) && (2 == nProtons) && (1 == nPhotons)) return NCRES_P_P_PHOTON;
        if ((4 == nNonNeutrons) && (3 == nProtons) && (1 == nPhotons)) return NCRES_P_P_P_PHOTON;
        if ((5 == nNonNeutrons) && (4 == nProtons) && (1 == nPhotons)) return NCRES_P_P_P_P_PHOTON;
        if ((6 == nNonNeutrons) && (5 == nProtons) && (1 == nPhotons)) return NCRES_P_P_P_P_P_PHOTON;

        if ((2 == nNonNeutrons) && (0 == nProtons) && (2 == nPhotons)) return NCRES_PIZERO;
        if ((3 == nNonNeutrons) && (1 == nProtons) && (2 == nPhotons)) return NCRES_P_PIZERO;
        if ((4 == nNonNeutrons) && (2 == nProtons) && (2 == nPhotons)) return NCRES_P_P_PIZERO;
        if ((5 == nNonNeutrons) && (3 == nProtons) && (2 == nPhotons)) return NCRES_P_P_P_PIZERO;
        if ((6 == nNonNeutrons) && (4 == nProtons) && (2 == nPhotons)) return NCRES_P_P_P_P_PIZERO;
        if ((7 == nNonNeutrons) && (5 == nProtons) && (2 == nPhotons)) return NCRES_P_P_P_P_P_PIZERO;
    }

    if (simpleMCEvent.m_mcNeutrinoNuance == 1091) return CCDIS;
    if (simpleMCEvent.m_mcNeutrinoNuance == 1092) return NCDIS;
    if (simpleMCEvent.m_mcNeutrinoNuance == 1096) return CCCOH;
    if (simpleMCEvent.m_mcNeutrinoNuance == 1097) return NCCOH;

    return OTHER_INTERACTION;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters, PfoMatchingMap &pfoMatchingMap)
{
    // Get best matches, one-by-one, until no more strong matches possible
    IntSet usedMCIds, usedPfoIds;
    while (GetStrongestPfoMatch(simpleMCEvent, matchingParameters, usedMCIds, usedPfoIds, pfoMatchingMap)) {}

    // Assign any remaining pfos to primaries, based on number of matched hits
    GetRemainingPfoMatches(simpleMCEvent, matchingParameters, usedPfoIds, pfoMatchingMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters, IntSet &usedMCIds, IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap)
{
    int bestPfoMatchId(-1);
    MatchingDetails bestMatchingDetails;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (!matchingParameters.m_useSmallPrimaries && (simpleMCPrimary.m_nGoodMCHitsTotal < matchingParameters.m_minPrimaryHits))
            continue;

        if (usedMCIds.count(simpleMCPrimary.m_id))
            continue;

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);
            
            if (usedPfoIds.count(simpleMatchedPfo.m_id))
                continue;

            if (!IsGoodMatch(simpleMCPrimary, simpleMatchedPfo, matchingParameters))
                continue;

            if (simpleMatchedPfo.m_nMatchedHitsTotal > bestMatchingDetails.m_nMatchedHits)
            {
                bestPfoMatchId = simpleMatchedPfo.m_id;
                bestMatchingDetails.m_matchedPrimaryId = simpleMCPrimary.m_id;
                bestMatchingDetails.m_nMatchedHits = simpleMatchedPfo.m_nMatchedHitsTotal;
            }
        }
    }

    if (bestPfoMatchId > -1)
    {
        pfoMatchingMap[bestPfoMatchId] = bestMatchingDetails;
        usedMCIds.insert(bestMatchingDetails.m_matchedPrimaryId);
        usedPfoIds.insert(bestPfoMatchId);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap)
{
    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (!matchingParameters.m_useSmallPrimaries && (simpleMCPrimary.m_nGoodMCHitsTotal < matchingParameters.m_minPrimaryHits))
            continue;

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (usedPfoIds.count(simpleMatchedPfo.m_id))
                continue;

            MatchingDetails &matchingDetails(pfoMatchingMap[simpleMatchedPfo.m_id]);

            if (simpleMatchedPfo.m_nMatchedHitsTotal > matchingDetails.m_nMatchedHits)
            {
                matchingDetails.m_matchedPrimaryId = simpleMCPrimary.m_id;
                matchingDetails.m_nMatchedHits = simpleMatchedPfo.m_nMatchedHitsTotal;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool HasMatch(const SimpleMCPrimary &simpleMCPrimary, const PfoMatchingMap &pfoMatchingMap, const MatchingParameters &matchingParameters)
{
    for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
    {
        const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

        if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool IsGoodMatch(const SimpleMCPrimary &simpleMCPrimary, const SimpleMatchedPfo &simpleMatchedPfo, const MatchingParameters &matchingParameters)
{
    const float purity((simpleMatchedPfo.m_nPfoHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMatchedPfo.m_nPfoHitsTotal) : 0.f);
    const float completeness((simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0.f);

    return ((simpleMatchedPfo.m_nMatchedHitsTotal >= matchingParameters.m_minSharedHits) && (purity >= matchingParameters.m_minPurity) && (completeness >= matchingParameters.m_minCompleteness));
}

//------------------------------------------------------------------------------------------------------------------------------------------

ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList)
{
    // ATTN: Relies on fact that primary list is sorted by number of true hits
    unsigned int nMuons(0), nElectrons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator iter = simpleMCPrimaryList.begin(); iter != simpleMCPrimaryList.end(); ++iter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*iter);

        if (primaryId == simpleMCPrimary.m_id)
        {
            if ((0 == nMuons) && (13 == simpleMCPrimary.m_pdgCode)) return MUON;
            if ((0 == nElectrons) && (11 == simpleMCPrimary.m_pdgCode)) return ELECTRON;
            if ((0 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON1;
            if ((1 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON2;
            if ((2 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON3;
            if ((3 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON4;
            if ((4 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON5;
            if ((0 == nPiPlus) && (211 == simpleMCPrimary.m_pdgCode)) return PIPLUS;
            if ((0 == nPiMinus) && (-211 == simpleMCPrimary.m_pdgCode)) return PIMINUS;
            if ((0 == nPhotons) && (22 == simpleMCPrimary.m_pdgCode)) return PHOTON1;
            if ((1 == nPhotons) && (22 == simpleMCPrimary.m_pdgCode)) return PHOTON2;
        }

        if (13 == simpleMCPrimary.m_pdgCode) ++nMuons;
        else if (11 == simpleMCPrimary.m_pdgCode) ++nElectrons;
        else if (2212 == simpleMCPrimary.m_pdgCode) ++nProtons;
        else if (211 == simpleMCPrimary.m_pdgCode) ++nPiPlus;
        else if (-211 == simpleMCPrimary.m_pdgCode) ++nPiMinus;
        else if (2112 == simpleMCPrimary.m_pdgCode) ++nNeutrons;
        else if (22 == simpleMCPrimary.m_pdgCode) ++nPhotons;
    }

    return OTHER_PRIMARY;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassFiducialCut(const SimpleMCEvent &simpleMCEvent)
{
    // MicroBooNE
    const float eVx(256.35), eVy(233.), eVz(1036.8);
    const float xBorder(10.), yBorder(20.), zBorder(10.);

    if ((simpleMCEvent.m_mcNeutrinoVtx.m_x < (eVx - xBorder)) && (simpleMCEvent.m_mcNeutrinoVtx.m_x > xBorder) &&
        (simpleMCEvent.m_mcNeutrinoVtx.m_y < (eVy / 2. - yBorder)) && (simpleMCEvent.m_mcNeutrinoVtx.m_y > (-eVy / 2. + yBorder)) &&
        (simpleMCEvent.m_mcNeutrinoVtx.m_z < (eVz - zBorder)) && (simpleMCEvent.m_mcNeutrinoVtx.m_z > zBorder) )
    {
        return true;
    }

    // DUNE 4APA
    // if (((simpleMCEvent.m_mcNeutrinoVtx.m_x > -349.f) && (simpleMCEvent.m_mcNeutrinoVtx.m_x < -10.f)) &&
    //     ((simpleMCEvent.m_mcNeutrinoVtx.m_y > -290.f) && (simpleMCEvent.m_mcNeutrinoVtx.m_y < 290.f)) &&
    //     ((simpleMCEvent.m_mcNeutrinoVtx.m_z > 70.f) && (simpleMCEvent.m_mcNeutrinoVtx.m_z < 414.f)) )
    // {
    //     return true;
    // }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const MatchingParameters &matchingParameters, InteractionCountingMap &interactionCountingMap, InteractionEventResultMap &interactionEventResultMap)
{
    if (matchingParameters.m_applyFiducialCut && !PassFiducialCut(simpleMCEvent))
        return;

    EventResult eventResult;
    eventResult.m_fileIdentifier = simpleMCEvent.m_fileIdentifier;
    eventResult.m_eventNumber = simpleMCEvent.m_eventNumber;
    eventResult.m_mcNeutrinoNuance = simpleMCEvent.m_mcNeutrinoNuance;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);
        const ExpectedPrimary expectedPrimary(GetExpectedPrimary(simpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList));

        const bool isTargetPrimary((simpleMCPrimary.m_nGoodMCHitsTotal >= matchingParameters.m_minPrimaryHits) && (2112 != simpleMCPrimary.m_pdgCode));

        if (!isTargetPrimary)
            continue;

        CountingDetails &countingDetails = interactionCountingMap[interactionType][expectedPrimary];
        PrimaryResult &primaryResult = eventResult.m_primaryResultMap[expectedPrimary];

        ++countingDetails.m_nTotal;
        unsigned int nMatches(0), nBestMatchedHits(0), nBestRecoHits(0);
        float bestMatchPurity(0.f), bestCompleteness(0.f);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                if (!IsGoodMatch(simpleMCPrimary, simpleMatchedPfo, matchingParameters))
                    continue;

                const unsigned int absMCPdgCode(std::abs(simpleMCPrimary.m_pdgCode));

                if (matchingParameters.m_correctTrackShowerId && (
                    ((absMCPdgCode == 13 || absMCPdgCode == 2212 || absMCPdgCode == 211) && (13 != simpleMatchedPfo.m_pdgCode)) ||
                    ((absMCPdgCode == 22 || absMCPdgCode == 11) && (11 != simpleMatchedPfo.m_pdgCode)) ))
                {
                    continue;
                }

                ++nMatches;
                const float purity((simpleMatchedPfo.m_nPfoHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMatchedPfo.m_nPfoHitsTotal) : 0);
                const float completeness((simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0);

                //primaryResult.m_allCompletenesses.push_back(completeness); // ATTN Expensive?

                if (completeness > bestCompleteness)
                {
                    bestMatchPurity = purity;
                    bestCompleteness = completeness;
                    nBestMatchedHits = simpleMatchedPfo.m_nMatchedHitsTotal;
                    nBestRecoHits = simpleMatchedPfo.m_nPfoHitsTotal;
                }
            }
        }

        if (0 == nMatches) ++countingDetails.m_nMatch0;
        else if (1 == nMatches) ++countingDetails.m_nMatch1;
        else if (2 == nMatches) ++countingDetails.m_nMatch2;
        else ++countingDetails.m_nMatch3Plus;

        primaryResult.m_nPfoMatches = nMatches;
        primaryResult.m_bestCompleteness = bestCompleteness;
        primaryResult.m_bestMatchPurity = bestMatchPurity;
        primaryResult.m_nTrueHits = simpleMCPrimary.m_nMCHitsTotal;

        const float pTot(std::sqrt(simpleMCPrimary.m_momentum.m_x * simpleMCPrimary.m_momentum.m_x + simpleMCPrimary.m_momentum.m_y * simpleMCPrimary.m_momentum.m_y + simpleMCPrimary.m_momentum.m_z * simpleMCPrimary.m_momentum.m_z));
        primaryResult.m_trueMomentum = pTot;

        primaryResult.m_trueAngle = std::acos(simpleMCPrimary.m_momentum.m_z / pTot);
        primaryResult.m_nBestMatchedHits = nBestMatchedHits;
        primaryResult.m_nBestRecoHits = nBestRecoHits;
    }

    if ((0 < simpleMCEvent.m_nRecoNeutrinos) && (1 == simpleMCEvent.m_nMCNeutrinos))
        eventResult.m_vertexOffset = simpleMCEvent.m_recoNeutrinoVtx - simpleMCEvent.m_mcNeutrinoVtx;

    eventResult.m_nRecoNeutrinos = simpleMCEvent.m_nRecoNeutrinos;
    interactionEventResultMap[interactionType].push_back(eventResult);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const PfoMatchingMap &pfoMatchingMap, const MatchingParameters &matchingParameters)
{
    std::cout << "---PROCESSED-MATCHING-OUTPUT--------------------------------------------------------------------" << std::endl;
    std::cout << "MinGoodPrimaryHits " << matchingParameters.m_minPrimaryHits << ", MinSharedHits " << matchingParameters.m_minSharedHits
              << ", UseSmallPrimaries " << matchingParameters.m_useSmallPrimaries << ", MinCompleteness " << matchingParameters.m_minCompleteness
              << ", MinPurity " << matchingParameters.m_minPurity << std::endl;

    bool isCorrect(true), isCalculable(false);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);
        const bool hasMatch(HasMatch(simpleMCPrimary, pfoMatchingMap, matchingParameters));
        const bool isTargetPrimary((simpleMCPrimary.m_nGoodMCHitsTotal >= matchingParameters.m_minPrimaryHits) && (2112 != simpleMCPrimary.m_pdgCode));

        if (!hasMatch && !isTargetPrimary)
            continue;

        std::cout << std::endl << (!isTargetPrimary ? "(Non target) " : "") << "Primary " << simpleMCPrimary.m_id
                  << ", PDG " << simpleMCPrimary.m_pdgCode << ", nMCHits " << simpleMCPrimary.m_nMCHitsTotal
                  << " (" << simpleMCPrimary.m_nMCHitsU << ", " << simpleMCPrimary.m_nMCHitsV << ", " << simpleMCPrimary.m_nMCHitsW << "),"
                  << " [nGood " << simpleMCPrimary.m_nGoodMCHitsTotal << " (" << simpleMCPrimary.m_nGoodMCHitsU << ", " << simpleMCPrimary.m_nGoodMCHitsV
                  << ", " << simpleMCPrimary.m_nGoodMCHitsW << ")]" << std::endl;

        if (2112 != simpleMCPrimary.m_pdgCode)
            isCalculable = true;

        unsigned int nMatches(0);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                const bool isGoodMatch(IsGoodMatch(simpleMCPrimary, simpleMatchedPfo, matchingParameters));

                if (isGoodMatch) ++nMatches;
                std::cout << "-" << (!isGoodMatch ? "(Below threshold) " : "") << "MatchedPfo " << simpleMatchedPfo.m_id;

                if (simpleMatchedPfo.m_parentId >= 0) std::cout << ", ParentPfo " << simpleMatchedPfo.m_parentId;

                std::cout << ", PDG " << simpleMatchedPfo.m_pdgCode << ", nMatchedHits " << simpleMatchedPfo.m_nMatchedHitsTotal
                          << " (" << simpleMatchedPfo.m_nMatchedHitsU << ", " << simpleMatchedPfo.m_nMatchedHitsV << ", " << simpleMatchedPfo.m_nMatchedHitsW << ")"
                          << ", nPfoHits " << simpleMatchedPfo.m_nPfoHitsTotal
                          << " (" << simpleMatchedPfo.m_nPfoHitsU << ", " << simpleMatchedPfo.m_nPfoHitsV << ", " << simpleMatchedPfo.m_nPfoHitsW << ")" << std::endl;
            }
        }

        if (isTargetPrimary && (1 != nMatches))
            isCorrect = false;
    }

    std::cout << std::endl << "Is correct? " << (isCorrect && isCalculable) << std::endl;
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplayInteractionCountingMap(const InteractionCountingMap &interactionCountingMap, const MatchingParameters &matchingParameters, const std::string &mapFileName)
{
    std::cout << "MinGoodPrimaryHits " << matchingParameters.m_minPrimaryHits << ", MinSharedHits " << matchingParameters.m_minSharedHits
              << ", UseSmallPrimaries " << matchingParameters.m_useSmallPrimaries << ", MinCompleteness " << matchingParameters.m_minCompleteness
              << ", MinPurity " << matchingParameters.m_minPurity << std::endl;

    std::ofstream mapFile;
    if (!mapFileName.empty())
    {
        mapFile.open(mapFileName, ios::app);
        mapFile << "MinGoodPrimaryHits " << matchingParameters.m_minPrimaryHits << ", MinSharedHits " << matchingParameters.m_minSharedHits
                << ", UseSmallPrimaries " << matchingParameters.m_useSmallPrimaries << ", MinCompleteness " << matchingParameters.m_minCompleteness
                << ", MinPurity " << matchingParameters.m_minPurity << std::endl;
    }

    std::cout << std::fixed;
    std::cout << std::setprecision(1);

    for (InteractionCountingMap::const_iterator iter = interactionCountingMap.begin(); iter != interactionCountingMap.end(); ++iter)
    {
        const InteractionType interactionType(iter->first);
        const CountingMap &countingMap(iter->second);
        std::cout << std::endl << ToString(interactionType) << std::endl;

        if (!mapFileName.empty())
            mapFile << std::endl << ToString(interactionType) << std::endl;

        for (CountingMap::const_iterator cIter = countingMap.begin(); cIter != countingMap.end(); ++cIter)
        {
            const ExpectedPrimary expectedPrimary(cIter->first);
            const CountingDetails &countingDetails(cIter->second);

            std::cout << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal
                      << ", nPfos |0: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch0) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, |1: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch1) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, |2: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch2) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|, |3+: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch3Plus) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                      << "%|" << std::endl;

            if (!mapFileName.empty())
            {
                mapFile << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal
                        << ", nPfos |0: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch0) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |1: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch1) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |2: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch2) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |3+: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch3Plus) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|" << std::endl;
            }
        }
    }

    if (!mapFileName.empty())
        mapFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalyseInteractionEventResultMap(const InteractionEventResultMap &interactionEventResultMap, const std::string &mapFileName,
    const std::string &eventFileName, const bool histogramOutput, const std::string &prefix)
{
    // Intended for filling histograms, post-processing of information collected in main loop over ntuple, etc.
    std::cout << std::endl << "EVENT INFO " << std::endl;

    std::ofstream mapFile, eventFile;
    if (!mapFileName.empty()) mapFile.open(mapFileName, ios::app);
    if (!eventFileName.empty()) eventFile.open(eventFileName, ios::app);

    InteractionPrimaryHistogramMap interactionPrimaryHistogramMap;
    InteractionEventHistogramMap interactionEventHistogramMap;

    for (InteractionEventResultMap::const_iterator iter = interactionEventResultMap.begin(), iterEnd = interactionEventResultMap.end(); iter != iterEnd; ++iter)
    {
        const InteractionType interactionType(iter->first);
        const EventResultList &eventResultList(iter->second);

        unsigned int nCorrectEvents(0);

        for (EventResultList::const_iterator eIter = eventResultList.begin(), eIterEnd = eventResultList.end(); eIter != eIterEnd; ++eIter)
        {
            const PrimaryResultMap &primaryResultMap(eIter->m_primaryResultMap);
            bool isCorrect(!primaryResultMap.empty());

            if (histogramOutput)
            {
                const std::string histPrefix(prefix + ToString(interactionType) + "_");
                EventHistogramCollection &histogramCollection(interactionEventHistogramMap[interactionType]);
                FillEventHistogramCollection(histPrefix, *eIter, histogramCollection);

                const std::string histPrefixAll(prefix + ToString(ALL_INTERACTIONS) + "_");
                EventHistogramCollection &histogramCollectionAll(interactionEventHistogramMap[ALL_INTERACTIONS]);
                FillEventHistogramCollection(histPrefixAll, *eIter, histogramCollectionAll);
            }

            for (PrimaryResultMap::const_iterator pIter = primaryResultMap.begin(), pIterEnd = primaryResultMap.end(); pIter != pIterEnd; ++pIter)
            {
                const ExpectedPrimary expectedPrimary(pIter->first);
                const PrimaryResult &primaryResult(pIter->second);

                if (primaryResult.m_nPfoMatches != 1)
                    isCorrect = false;

                if (histogramOutput)
                {
                    const std::string histPrefix(prefix + ToString(interactionType) + "_" + ToString(expectedPrimary) + "_");
                    PrimaryHistogramCollection &histogramCollection(interactionPrimaryHistogramMap[interactionType][expectedPrimary]);
                    FillPrimaryHistogramCollection(histPrefix, primaryResult, histogramCollection);

                    const std::string histPrefixAll(prefix + ToString(ALL_INTERACTIONS) + "_" + ToString(expectedPrimary) + "_");
                    PrimaryHistogramCollection &histogramCollectionAll(interactionPrimaryHistogramMap[ALL_INTERACTIONS][expectedPrimary]);
                    FillPrimaryHistogramCollection(histPrefixAll, primaryResult, histogramCollectionAll);
                }
            }

            if (isCorrect)
            {
                ++nCorrectEvents;

                if (!eventFileName.empty())
                    eventFile << "Correct event: fileId: " << eIter->m_fileIdentifier << ", eventNumber: " << eIter->m_eventNumber << ", nuance: " << eIter->m_mcNeutrinoNuance << std::endl;
            }
        }

        std::cout << ToString(interactionType) << std::endl << "-nEvents " << eventResultList.size() << ", nCorrect " << nCorrectEvents
                  << ", fCorrect " << 100.f * static_cast<float>(nCorrectEvents) / static_cast<float>(eventResultList.size()) << "%" << std::endl;

        if (!mapFileName.empty())
        {
            mapFile << ToString(interactionType) << std::endl << "-nEvents " << eventResultList.size() << ", nCorrect " << nCorrectEvents
                    << ", fCorrect " << 100.f * static_cast<float>(nCorrectEvents) / static_cast<float>(eventResultList.size()) << "%" << std::endl;
        }
    }

    if (histogramOutput)
        ProcessHistogramCollections(interactionPrimaryHistogramMap);

    if (!mapFileName.empty()) mapFile.close();
    if (!eventFileName.empty()) eventFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillEventHistogramCollection(const std::string &histPrefix, const EventResult &eventResult, EventHistogramCollection &eventHistogramCollection)
{
    if (!eventHistogramCollection.m_hVtxDeltaX)
    {
        eventHistogramCollection.m_hVtxDeltaX = new TH1F((histPrefix + "VtxDeltaX").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetRangeUser(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetTitle("Vertex #DeltaX [cm]");
        eventHistogramCollection.m_hVtxDeltaX->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_hVtxDeltaY)
    {
        eventHistogramCollection.m_hVtxDeltaY = new TH1F((histPrefix + "VtxDeltaY").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetRangeUser(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetTitle("Vertex #DeltaY [cm]");
        eventHistogramCollection.m_hVtxDeltaY->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_hVtxDeltaZ)
    {
        eventHistogramCollection.m_hVtxDeltaZ = new TH1F((histPrefix + "VtxDeltaZ").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetRangeUser(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetTitle("Vertex #DeltaZ [cm]");
        eventHistogramCollection.m_hVtxDeltaZ->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_hVtxDeltaR)
    {
        eventHistogramCollection.m_hVtxDeltaR = new TH1F((histPrefix + "VtxDeltaR").c_str(), "", 40000, -100., 1900.);
        eventHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetRangeUser(0., +5.);
        eventHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetTitle("Vertex #DeltaR [cm]");
        eventHistogramCollection.m_hVtxDeltaR->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_nRecoNeutrinos)
    {
        eventHistogramCollection.m_nRecoNeutrinos = new TH1F((histPrefix + "NRecoNeutrinos").c_str(), "", 11, -0.5, 10.5);
        eventHistogramCollection.m_nRecoNeutrinos->GetXaxis()->SetTitle("nRecoNeutrinos");
        eventHistogramCollection.m_nRecoNeutrinos->GetYaxis()->SetTitle("nEvents");
    }

    const float xCorrection(4.95694e-01);
    eventHistogramCollection.m_hVtxDeltaX->Fill(eventResult.m_vertexOffset.m_x - xCorrection);
    eventHistogramCollection.m_hVtxDeltaY->Fill(eventResult.m_vertexOffset.m_y);
    eventHistogramCollection.m_hVtxDeltaZ->Fill(eventResult.m_vertexOffset.m_z);
    eventHistogramCollection.m_hVtxDeltaR->Fill(std::sqrt((eventResult.m_vertexOffset.m_x - xCorrection) * (eventResult.m_vertexOffset.m_x - xCorrection) + eventResult.m_vertexOffset.m_y * eventResult.m_vertexOffset.m_y + eventResult.m_vertexOffset.m_z * eventResult.m_vertexOffset.m_z));
    eventHistogramCollection.m_nRecoNeutrinos->Fill(eventResult.m_nRecoNeutrinos);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillPrimaryHistogramCollection(const std::string &histPrefix, const PrimaryResult &primaryResult, PrimaryHistogramCollection &primaryHistogramCollection)
{
    const int nHitBins(35); const int nHitBinEdges(nHitBins + 1);
    float hitsBinning[nHitBinEdges];
    for (int n = 0; n < nHitBinEdges; ++n) hitsBinning[n] = std::pow(10., 1 + static_cast<float>(n + 2) / 10.);

    if (!primaryHistogramCollection.m_hHitsAll)
    {
        primaryHistogramCollection.m_hHitsAll = new TH1F((histPrefix + "HitsAll").c_str(), "", nHitBins, hitsBinning);
        primaryHistogramCollection.m_hHitsAll->GetXaxis()->SetRangeUser(1., +6000);
        primaryHistogramCollection.m_hHitsAll->GetXaxis()->SetTitle("nTrueHits");
        primaryHistogramCollection.m_hHitsAll->GetYaxis()->SetTitle("nEvents");
    }

    if (!primaryHistogramCollection.m_hHitsEfficiency)
    {
        primaryHistogramCollection.m_hHitsEfficiency = new TH1F((histPrefix + "HitsEfficiency").c_str(), "", nHitBins, hitsBinning);
        primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->SetRangeUser(1., +6000);
        primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->SetTitle("nTrueHits");
        primaryHistogramCollection.m_hHitsEfficiency->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hHitsEfficiency->GetYaxis()->SetTitle("Efficiency");
    }

    const int nMomentumBins(20); const int nMomentumBinEdges(nMomentumBins + 1);
    float momentumBinning[nMomentumBinEdges] = {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.4, 1.6, 2.0, 2.4, 2.8, 3.4, 4., 5.};

    if (!primaryHistogramCollection.m_hMomentumAll)
    {
        primaryHistogramCollection.m_hMomentumAll = new TH1F((histPrefix + "MomentumAll").c_str(), "", nMomentumBins, momentumBinning);
        primaryHistogramCollection.m_hMomentumAll->GetXaxis()->SetRangeUser(0., +5.5);
        primaryHistogramCollection.m_hMomentumAll->GetXaxis()->SetTitle("True Momentum [GeV]");
        primaryHistogramCollection.m_hMomentumAll->GetYaxis()->SetTitle("nEvents");
    }

    if (!primaryHistogramCollection.m_hMomentumEfficiency)
    {
        primaryHistogramCollection.m_hMomentumEfficiency = new TH1F((histPrefix + "MomentumEfficiency").c_str(), "", nMomentumBins, momentumBinning);
        primaryHistogramCollection.m_hMomentumEfficiency->GetXaxis()->SetRangeUser(1., +5.5);
        primaryHistogramCollection.m_hMomentumEfficiency->GetXaxis()->SetTitle("True Momentum [GeV]");
        primaryHistogramCollection.m_hMomentumEfficiency->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hMomentumEfficiency->GetYaxis()->SetTitle("Efficiency");
    }

    if (!primaryHistogramCollection.m_hAngleAll)
    {
        primaryHistogramCollection.m_hAngleAll = new TH1F((histPrefix + "AngleAll").c_str(), "", 64, -M_PI, M_PI);
        primaryHistogramCollection.m_hAngleAll->GetXaxis()->SetRangeUser(0., +3.141);
        primaryHistogramCollection.m_hAngleAll->GetXaxis()->SetTitle("#theta_{z}");
        primaryHistogramCollection.m_hAngleAll->GetYaxis()->SetTitle("nEvents");
    }

    if (!primaryHistogramCollection.m_hAngleEfficiency)
    {
        primaryHistogramCollection.m_hAngleEfficiency = new TH1F((histPrefix + "AngleEfficiency").c_str(), "", 64, -M_PI, M_PI);
        primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->SetRangeUser(0., +3.141);
        primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->SetTitle("#theta_{z}");
        primaryHistogramCollection.m_hAngleEfficiency->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hAngleEfficiency->GetYaxis()->SetTitle("Efficiency");
    }

    if (!primaryHistogramCollection.m_hCompleteness)
    {
        primaryHistogramCollection.m_hCompleteness = new TH1F((histPrefix + "Completeness").c_str(), "", 51, -0.01, 1.01);
        primaryHistogramCollection.m_hCompleteness->GetXaxis()->SetTitle("Completeness");
        primaryHistogramCollection.m_hCompleteness->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hCompleteness->GetYaxis()->SetTitle("Fraction of Particles");
    }

    if (!primaryHistogramCollection.m_hPurity)
    {
        primaryHistogramCollection.m_hPurity = new TH1F((histPrefix + "Purity").c_str(), "", 51, -0.01, 1.01);
        primaryHistogramCollection.m_hPurity->GetXaxis()->SetTitle("Purity");
        primaryHistogramCollection.m_hPurity->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hPurity->GetYaxis()->SetTitle("Fraction of Particles");
    }

    if (!primaryHistogramCollection.m_hNPfosVsPTot)
    {
        primaryHistogramCollection.m_hNPfosVsPTot = new TH2F((histPrefix + "NMatchedPfosVsPTrue").c_str(), "", 21, -0.05, 2.05, 51, -0.5, 50.5);
        primaryHistogramCollection.m_hNPfosVsPTot->GetXaxis()->SetTitle("P_{True} [GeV]");
        primaryHistogramCollection.m_hNPfosVsPTot->GetXaxis()->SetRangeUser(0., +2.01);
        primaryHistogramCollection.m_hNPfosVsPTot->GetYaxis()->SetRangeUser(0., +10.);
        primaryHistogramCollection.m_hNPfosVsPTot->GetYaxis()->SetTitle("Number of Matched Pfos");
    }

    if (!primaryHistogramCollection.m_hBestCompVsPTot)
    {
        primaryHistogramCollection.m_hBestCompVsPTot = new TH2F((histPrefix + "BestCompletenessVsPTrue").c_str(), "", 21, -0.05, 2.05, 26, -0.02, 1.02);
        primaryHistogramCollection.m_hBestCompVsPTot->GetXaxis()->SetTitle("P_{True} [GeV]");
        primaryHistogramCollection.m_hBestCompVsPTot->GetXaxis()->SetRangeUser(0., +2.01);
        primaryHistogramCollection.m_hBestCompVsPTot->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hBestCompVsPTot->GetYaxis()->SetTitle("Best Completeness");
    }

    if (!primaryHistogramCollection.m_hAllCompVsPTot)
    {
        primaryHistogramCollection.m_hAllCompVsPTot = new TH2F((histPrefix + "AllCompletenessesVsPTrue").c_str(), "", 21, -0.05, 2.05, 26, -0.02, 1.02);
        primaryHistogramCollection.m_hAllCompVsPTot->GetXaxis()->SetTitle("P_{True} [GeV]");
        primaryHistogramCollection.m_hAllCompVsPTot->GetXaxis()->SetRangeUser(0., +2.01);
        primaryHistogramCollection.m_hAllCompVsPTot->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hAllCompVsPTot->GetYaxis()->SetTitle("All Matched Pfo Completenesses");
    }

    primaryHistogramCollection.m_hNPfosVsPTot->Fill(primaryResult.m_trueMomentum, primaryResult.m_nPfoMatches);
    primaryHistogramCollection.m_hBestCompVsPTot->Fill(primaryResult.m_trueMomentum, primaryResult.m_bestCompleteness);

    for (FloatVector::const_iterator iter = primaryResult.m_allCompletenesses.begin(); iter != primaryResult.m_allCompletenesses.end(); ++iter)
        primaryHistogramCollection.m_hAllCompVsPTot->Fill(primaryResult.m_trueMomentum, *iter);

    primaryHistogramCollection.m_hHitsAll->Fill(primaryResult.m_nTrueHits);
    primaryHistogramCollection.m_hMomentumAll->Fill(primaryResult.m_trueMomentum);
    primaryHistogramCollection.m_hAngleAll->Fill(primaryResult.m_trueAngle);

    if (primaryResult.m_nPfoMatches > 0)
    {
        primaryHistogramCollection.m_hHitsEfficiency->Fill(primaryResult.m_nTrueHits);
        primaryHistogramCollection.m_hMomentumEfficiency->Fill(primaryResult.m_trueMomentum);
        primaryHistogramCollection.m_hAngleEfficiency->Fill(primaryResult.m_trueAngle);
        primaryHistogramCollection.m_hCompleteness->Fill(primaryResult.m_bestCompleteness);
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

            for (int n = 0; n < primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->GetNbins(); ++n)
            {
                const float found = primaryHistogramCollection.m_hHitsEfficiency->GetBinContent(n + 1);
                const float all = primaryHistogramCollection.m_hHitsAll->GetBinContent(n + 1);
                const float efficiency = (all > 0.f) ? found / all : 0.f;
                const float error = (all > found) ? std::sqrt(efficiency * (1. - efficiency) / all) : 0.f;
                primaryHistogramCollection.m_hHitsEfficiency->SetBinContent(n + 1, efficiency);
                primaryHistogramCollection.m_hHitsEfficiency->SetBinError(n + 1, error);
            }

            for (int n = 0; n < primaryHistogramCollection.m_hMomentumEfficiency->GetXaxis()->GetNbins(); ++n)
            {
                const float found = primaryHistogramCollection.m_hMomentumEfficiency->GetBinContent(n + 1);
                const float all = primaryHistogramCollection.m_hMomentumAll->GetBinContent(n + 1);
                const float efficiency = (all > 0.f) ? found / all : 0.f;
                const float error = (all > found) ? std::sqrt(efficiency * (1. - efficiency) / all) : 0.f;
                primaryHistogramCollection.m_hMomentumEfficiency->SetBinContent(n + 1, efficiency);
                primaryHistogramCollection.m_hMomentumEfficiency->SetBinError(n + 1, error);
            }

            for (int n = 0; n < primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->GetNbins(); ++n)
            {
                const float found = primaryHistogramCollection.m_hAngleEfficiency->GetBinContent(n + 1);
                const float all = primaryHistogramCollection.m_hAngleAll->GetBinContent(n + 1);
                const float efficiency = (all > 0.f) ? found / all : 0.f;
                const float error = (all > found) ? std::sqrt(efficiency * (1. - efficiency) / all) : 0.f;
                primaryHistogramCollection.m_hAngleEfficiency->SetBinContent(n + 1, efficiency);
                primaryHistogramCollection.m_hAngleEfficiency->SetBinError(n + 1, error);
            }

            primaryHistogramCollection.m_hCompleteness->Scale(1. / static_cast<double>(primaryHistogramCollection.m_hCompleteness->GetEntries()));
            primaryHistogramCollection.m_hPurity->Scale(1. / static_cast<double>(primaryHistogramCollection.m_hPurity->GetEntries()));
        }
    }
}
