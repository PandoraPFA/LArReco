#include "TChain.h"

#include "Validation.h"

void Validation(const std::string &inputFiles, const bool shouldDisplay, const int skipEvents, const int nEventsToProcess,
    const int primaryMinHits, const int minMatchedHits)
{
    TChain *pTChain = new TChain("Validation", "pTChain");
    pTChain->Add(inputFiles.c_str());

    // To store final output
    InteractionTypeToCountingMap interactionTypeToCountingMap;
    InteractionTypeToEventOutcomeMap interactionTypeToEventOutcomeMap;

    int nEvents(0), nProcessedEvents(0);

    for (int iEntry = 0; iEntry < pTChain->GetEntries(); )
    {
        SimpleMCEvent simpleMCEvent;
        iEntry += ReadNextEvent(pTChain, iEntry, simpleMCEvent);

        if (nEvents++ < skipEvents)
            continue;

        if (nProcessedEvents++ >= nEventsToProcess)
            break;

        if (shouldDisplay)
            DisplaySimpleMCEvent(simpleMCEvent, primaryMinHits, minMatchedHits);

        // Analysis code starts here
        const InteractionType interactionType(GetInteractionType(simpleMCEvent, primaryMinHits));

        if (OTHER_INTERACTION == interactionType)
            continue;

        PfoMatchingMap pfoMatchingMap;
        FinalisePfoMatching(simpleMCEvent, primaryMinHits, minMatchedHits, pfoMatchingMap);

        CountPfoMatches(simpleMCEvent, interactionType, pfoMatchingMap, primaryMinHits, interactionTypeToCountingMap, interactionTypeToEventOutcomeMap);
    }

    // Print-out summary of interactionTypeToCountingMap here
    std::cout << "MinPrimaryHits " << primaryMinHits << ", MinMatchedHits " << minMatchedHits << std::endl;
    DisplayInteractionTypeToCountingMap(interactionTypeToCountingMap);

    // Intended for filling histograms
    std::cout << std::endl << "EVENT INFO " << std::endl;

    for (InteractionTypeToEventOutcomeMap::const_iterator iter = interactionTypeToEventOutcomeMap.begin(), iterEnd = interactionTypeToEventOutcomeMap.end(); iter != iterEnd; ++iter)
    {
        const InteractionType interactionType(iter->first);
        const EventOutcomeList &eventOutcomeList(iter->second);
        std::cout << ToString(interactionType) << ", nEvents " << eventOutcomeList.size() << std::endl;

        for (EventOutcomeList::const_iterator eIter = eventOutcomeList.begin(), eIterEnd = eventOutcomeList.end(); eIter != eIterEnd; ++eIter)
        {
            const EventOutcome &eventOutcome(*eIter);
            const PrimaryResultMap &primaryResultMap(eventOutcome.m_primaryResultMap);

            for (PrimaryResultMap::const_iterator pIter = primaryResultMap.begin(), pIterEnd = primaryResultMap.end(); pIter != pIterEnd; ++pIter)
            {
                const ExpectedPrimary expectedPrimary(pIter->first);
                const PrimaryResult &primaryResult(pIter->second);

                std::cout << "-" << ToString(expectedPrimary) << ": nMatches: " << primaryResult.m_nPfoMatches << ", bestComp: "
                          << primaryResult.m_bestCompleteness << ", bestMatchPur: " << primaryResult.m_bestMatchPurity << std::endl;
            }

            std::cout << std::endl;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

InteractionType GetInteractionType(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits)
{
    unsigned int nSignificantPrimaries(0), nNonNeutrons(0), nMuons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
            continue;

        ++nSignificantPrimaries;

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

    if ((1001 == simpleMCEvent.m_mcNeutrinoNuance) && (1 == nNonNeutrons) && (1 == nMuons)) interactionType = CCQEL_MU;
    else if ((1001 == simpleMCEvent.m_mcNeutrinoNuance) && (2 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons)) interactionType = CCQEL_MU_P;
    else if ((1001 == simpleMCEvent.m_mcNeutrinoNuance) && (3 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons)) interactionType = CCQEL_MU_P_P;
    else if ((1002 == simpleMCEvent.m_mcNeutrinoNuance) && (1 == nNonNeutrons) && (1 == nProtons)) interactionType = NCQEL_P;
    else if ((1002 == simpleMCEvent.m_mcNeutrinoNuance) && (2 == nNonNeutrons) && (2 == nProtons)) interactionType = NCQEL_P_P;
    else if ((1003 == simpleMCEvent.m_mcNeutrinoNuance) && (2 == nNonNeutrons) && (1 == nMuons) && (1 == nPiPlus)) interactionType = CCRES_MU_PIPLUS;
    else if ((1003 == simpleMCEvent.m_mcNeutrinoNuance) && (3 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (1 == nPiPlus)) interactionType = CCRES_MU_P_PIPLUS;
    else if ((1003 == simpleMCEvent.m_mcNeutrinoNuance) && (4 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (1 == nPiPlus)) interactionType = CCRES_MU_P_P_PIPLUS;
    else if ((1004 == simpleMCEvent.m_mcNeutrinoNuance) && (3 == nNonNeutrons) && (1 == nMuons) && (2 == nPhotons)) interactionType = CCRES_MU_PIZERO;
    else if ((1004 == simpleMCEvent.m_mcNeutrinoNuance) && (4 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (2 == nPhotons)) interactionType = CCRES_MU_P_PIZERO;
    else if ((1004 == simpleMCEvent.m_mcNeutrinoNuance) && (5 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (2 == nPhotons)) interactionType = CCRES_MU_P_P_PIZERO;
    else if ((1007 == simpleMCEvent.m_mcNeutrinoNuance) && (2 == nNonNeutrons) && (1 == nProtons) && (1 == nPiPlus)) interactionType = NCRES_P_PIPLUS;
    else if ((1007 == simpleMCEvent.m_mcNeutrinoNuance) && (3 == nNonNeutrons) && (2 == nProtons) && (1 == nPiPlus)) interactionType = NCRES_P_P_PIPLUS;
    else if ((1006 == simpleMCEvent.m_mcNeutrinoNuance) && (3 == nNonNeutrons) && (1 == nProtons) && (2 == nPhotons)) interactionType = NCRES_P_PIZERO;
    else if ((1006 == simpleMCEvent.m_mcNeutrinoNuance) && (4 == nNonNeutrons) && (2 == nProtons) && (2 == nPhotons)) interactionType = NCRES_P_P_PIZERO;

    return interactionType;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, IntSet &usedMCIds,
    IntSet &usedPfoIds, PfoMatchingMap &pfoMatchingMap)
{
    int bestPfoMatchId(-1);
    MatchingDetails bestMatchingDetails;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (usedMCIds.count(simpleMCPrimary.m_id))// || (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits))
            continue;

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);
            
            if (usedPfoIds.count(simpleMatchedPfo.m_id) || (simpleMatchedPfo.m_nMatchedHitsTotal < minMatchedHits))
                continue;

            if (simpleMatchedPfo.m_nMatchedHitsTotal > bestMatchingDetails.m_nMatchedHits)
            {
                bestPfoMatchId = simpleMatchedPfo.m_id;
                bestMatchingDetails.m_matchedPrimaryId = simpleMCPrimary.m_id;
                bestMatchingDetails.m_nMatchedHits = simpleMatchedPfo.m_nMatchedHitsTotal;
                bestMatchingDetails.m_completeness = static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal);
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

void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap)
{
    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        //if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
        //    continue;

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);
            
            if (usedPfoIds.count(simpleMatchedPfo.m_id) || (simpleMatchedPfo.m_nMatchedHitsTotal < minMatchedHits))
                continue;
            
            MatchingDetails &matchingDetails(pfoMatchingMap[simpleMatchedPfo.m_id]);

            if (simpleMatchedPfo.m_nMatchedHitsTotal > matchingDetails.m_nMatchedHits)
            {
                matchingDetails.m_matchedPrimaryId = simpleMCPrimary.m_id;
                matchingDetails.m_nMatchedHits = simpleMatchedPfo.m_nMatchedHitsTotal;
                matchingDetails.m_completeness = static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, PfoMatchingMap &pfoMatchingMap)
{
    // Get best matches, one-by-one, until no more strong matches possible
    IntSet usedMCIds, usedPfoIds;
    while (GetStrongestPfoMatch(simpleMCEvent, primaryMinHits, minMatchedHits, usedMCIds, usedPfoIds, pfoMatchingMap)) {}

    // Assign any remaining pfos to primaries, based on number of matched hits
    GetRemainingPfoMatches(simpleMCEvent, primaryMinHits, minMatchedHits, usedPfoIds, pfoMatchingMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList, const int primaryMinHits)
{
    unsigned int nMuons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator iter = simpleMCPrimaryList.begin(); iter != simpleMCPrimaryList.end(); ++iter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*iter);

        if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
            continue;

        if (primaryId == simpleMCPrimary.m_id)
        {
            if ((0 == nMuons) && (13 == simpleMCPrimary.m_pdgCode))
                return MUON;

            if ((0 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode))
                return PROTON1;

            if ((1 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode))
                return PROTON2;

            if ((0 == nPiPlus) && (211 == simpleMCPrimary.m_pdgCode))
                return PIPLUS;

            if ((0 == nPiMinus) && (-211 == simpleMCPrimary.m_pdgCode))
                return PIMINUS;

 //           if ((0 == nNeutrons) && (2112 == simpleMCPrimary.m_pdgCode))
 //               return NEUTRON;

            if ((0 == nPhotons) && (22 == simpleMCPrimary.m_pdgCode))
                return PHOTON1;

            if ((1 == nPhotons) && (22 == simpleMCPrimary.m_pdgCode))
                return PHOTON2;
        }

        if (13 == simpleMCPrimary.m_pdgCode) ++nMuons;
        else if (2212 == simpleMCPrimary.m_pdgCode) ++nProtons;
        else if (211 == simpleMCPrimary.m_pdgCode) ++nPiPlus;
        else if (-211 == simpleMCPrimary.m_pdgCode) ++nPiMinus;
        else if (2112 == simpleMCPrimary.m_pdgCode) ++nNeutrons;
        else if (22 == simpleMCPrimary.m_pdgCode) ++nPhotons;
    }

    return OTHER_PRIMARY;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const int primaryMinHits, InteractionTypeToCountingMap &interactionTypeToCountingMap, InteractionTypeToEventOutcomeMap &interactionTypeToEventOutcomeMap)
{
    EventOutcome eventOutcome;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);
        const ExpectedPrimary expectedPrimary(GetExpectedPrimary(simpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, primaryMinHits));

        if (OTHER_PRIMARY == expectedPrimary)
            continue;

        CountingDetails &countingDetails = interactionTypeToCountingMap[interactionType][expectedPrimary];
        PrimaryResult &primaryResult = eventOutcome.m_primaryResultMap[expectedPrimary];

        ++countingDetails.m_nTotal;
        unsigned int nMatches(0);
        float bestCompleteness(0.f), bestMatchPurity(0.f);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                ++nMatches;
                const float completeness((simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0);
                const float purity((simpleMatchedPfo.m_nPfoHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMatchedPfo.m_nPfoHitsTotal) : 0);

                if (completeness > bestCompleteness)
                {
                    bestCompleteness = completeness;
                    bestMatchPurity = purity;
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
    }

    interactionTypeToEventOutcomeMap[interactionType].push_back(eventOutcome);
}
