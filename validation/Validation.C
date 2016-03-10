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

void Validation(const std::string &inputFiles, const bool shouldDisplayEvents, const bool shouldDisplayMatchedEvents, const int skipEvents,
    const int nEventsToProcess, const int primaryMinHits, const int minMatchedHits, const bool histogramOutput, const bool correctId,
    const bool applyFiducialCut, const std::string histPrefix, const std::string outputFileName, const std::string correctEventFileName)
{
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

        if (nProcessedEvents++ >= nEventsToProcess)
            break;

        if (shouldDisplayEvents)
            ValidationIO::DisplaySimpleMCEvent(simpleMCEvent);

        // Analysis code starts here
        const InteractionType interactionType(GetInteractionType(simpleMCEvent, primaryMinHits));

        PfoMatchingMap pfoMatchingMap;
        FinalisePfoMatching(simpleMCEvent, primaryMinHits, minMatchedHits, pfoMatchingMap);

        if (shouldDisplayMatchedEvents)
            DisplaySimpleMCEventMatches(simpleMCEvent, pfoMatchingMap, primaryMinHits);

        CountPfoMatches(simpleMCEvent, interactionType, pfoMatchingMap, primaryMinHits, correctId, applyFiducialCut, interactionCountingMap,
            interactionEventResultMap);
    }

    // Processing of final output
    DisplayInteractionCountingMap(primaryMinHits, minMatchedHits, interactionCountingMap, outputFileName);
    AnalyseInteractionEventResultMap(interactionEventResultMap, outputFileName, correctEventFileName, histogramOutput, histPrefix);
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

void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, PfoMatchingMap &pfoMatchingMap)
{
    // Get best matches, one-by-one, until no more strong matches possible
    IntSet usedMCIds, usedPfoIds;
    while (GetStrongestPfoMatch(simpleMCEvent, primaryMinHits, minMatchedHits, usedMCIds, usedPfoIds, pfoMatchingMap)) {}

    // Assign any remaining pfos to primaries, based on number of matched hits
    GetRemainingPfoMatches(simpleMCEvent, primaryMinHits, minMatchedHits, usedPfoIds, pfoMatchingMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, IntSet &usedMCIds, IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap)
{
    int bestPfoMatchId(-1);
    MatchingDetails bestMatchingDetails;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
            continue;

        if (usedMCIds.count(simpleMCPrimary.m_id))
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

        if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
            continue;

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

void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const int primaryMinHits, const bool correctId, const bool applyFiducialCut, InteractionCountingMap &interactionCountingMap,
    InteractionEventResultMap &interactionEventResultMap)
{
    if (applyFiducialCut && !PassFiducialCut(simpleMCEvent))
        return;

    EventResult eventResult;
    eventResult.m_fileIdentifier = simpleMCEvent.m_fileIdentifier;
    eventResult.m_eventNumber = simpleMCEvent.m_eventNumber;
    eventResult.m_mcNeutrinoNuance = simpleMCEvent.m_mcNeutrinoNuance;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);
        const ExpectedPrimary expectedPrimary(GetExpectedPrimary(simpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, primaryMinHits));

        if (OTHER_PRIMARY == expectedPrimary)
            continue;

        CountingDetails &countingDetails = interactionCountingMap[interactionType][expectedPrimary];
        PrimaryResult &primaryResult = eventResult.m_primaryResultMap[expectedPrimary];

        ++countingDetails.m_nTotal;
        unsigned int nMatches(0), nBestMatchedHits(0), nBestRecoHits(0);
        float bestCompleteness(0.f), bestMatchPurity(0.f);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                const unsigned int absMCPdgCode(std::abs(simpleMCPrimary.m_pdgCode));

                if (correctId &&
                    ((absMCPdgCode == 13 || absMCPdgCode == 2212 || absMCPdgCode == 211) && (13 != simpleMatchedPfo.m_pdgCode) ||
                    ((absMCPdgCode == 22 || absMCPdgCode == 11) && (11 != simpleMatchedPfo.m_pdgCode))) )
                {
                    continue;
                }

                ++nMatches;
                const float completeness((simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0);
                const float purity((simpleMatchedPfo.m_nPfoHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMatchedPfo.m_nPfoHitsTotal) : 0);

                if (completeness > bestCompleteness)
                {
                    bestCompleteness = completeness;
                    bestMatchPurity = purity;
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
        primaryResult.m_trueAngle = std::acos(simpleMCPrimary.m_momentum.m_z / pTot);
        primaryResult.m_nBestMatchedHits = nBestMatchedHits;
        primaryResult.m_nBestRecoHits = nBestRecoHits;
    }

    if ((1 == simpleMCEvent.m_nRecoNeutrinos) && (1 == simpleMCEvent.m_nMCNeutrinos))
        eventResult.m_vertexOffset = simpleMCEvent.m_recoNeutrinoVtx - simpleMCEvent.m_mcNeutrinoVtx;

    eventResult.m_nRecoNeutrinos = simpleMCEvent.m_nRecoNeutrinos;
    interactionEventResultMap[interactionType].push_back(eventResult);
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

ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList, const int primaryMinHits)
{
    // ATTN: Relies on fact that primary list is sorted by number of true hits
    unsigned int nMuons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator iter = simpleMCPrimaryList.begin(); iter != simpleMCPrimaryList.end(); ++iter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*iter);

        if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
            continue;

        if (primaryId == simpleMCPrimary.m_id)
        {
            if ((0 == nMuons) && (13 == simpleMCPrimary.m_pdgCode)) return MUON;
            if ((0 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON1;
            if ((1 == nProtons) && (2212 == simpleMCPrimary.m_pdgCode)) return PROTON2;
            if ((0 == nPiPlus) && (211 == simpleMCPrimary.m_pdgCode)) return PIPLUS;
            if ((0 == nPiMinus) && (-211 == simpleMCPrimary.m_pdgCode)) return PIMINUS;
            if ((0 == nPhotons) && (22 == simpleMCPrimary.m_pdgCode)) return PHOTON1;
            if ((1 == nPhotons) && (22 == simpleMCPrimary.m_pdgCode)) return PHOTON2;
            //if ((0 == nNeutrons) && (2112 == simpleMCPrimary.m_pdgCode)) return NEUTRON; 
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

void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const PfoMatchingMap &pfoMatchingMap, const int primaryMinHits)
{
    std::cout << "---PROCESSED-MATCHING-OUTPUT--------------------------------------------------------------------" << std::endl;
    bool isCorrect(true);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (simpleMCPrimary.m_nMCHitsTotal < primaryMinHits)
            continue;

        std::cout << std::endl << "MCPrimary " << simpleMCPrimary.m_id << ", PDG " << simpleMCPrimary.m_pdgCode
                  << ", nMCHits " << simpleMCPrimary.m_nMCHitsTotal << " (" << simpleMCPrimary.m_nMCHitsU
                  << ", " << simpleMCPrimary.m_nMCHitsV << ", " << simpleMCPrimary.m_nMCHitsW << ")" << std::endl;

        unsigned int nMatches(0);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                std::cout << "-MatchedPfo " << simpleMatchedPfo.m_id;
                ++nMatches;

                if (simpleMatchedPfo.m_parentId >= 0)
                    std::cout << ", ParentPfo " << simpleMatchedPfo.m_parentId;

                std::cout << ", PDG " << simpleMatchedPfo.m_pdgCode
                          << ", nMatchedHits " << simpleMatchedPfo.m_nMatchedHitsTotal << " (" << simpleMatchedPfo.m_nMatchedHitsU
                          << ", " << simpleMatchedPfo.m_nMatchedHitsV << ", " << simpleMatchedPfo.m_nMatchedHitsW << ")"
                          << ", nPfoHits " << simpleMatchedPfo.m_nPfoHitsTotal << " (" << simpleMatchedPfo.m_nPfoHitsU
                          << ", " << simpleMatchedPfo.m_nPfoHitsV << ", " << simpleMatchedPfo.m_nPfoHitsW << ")" << std::endl;
            }
        }

        if (1 != nMatches)
            isCorrect = false;
    }

    std::cout << std::endl << "Is correct? " << isCorrect << std::endl;
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplayInteractionCountingMap(const int primaryMinHits, const int minMatchedHits, const InteractionCountingMap &interactionCountingMap,
    const std::string &outputFileName)
{
    std::cout << "MinPrimaryHits " << primaryMinHits << ", MinMatchedHits " << minMatchedHits << std::endl;
    std::ofstream outputFile;

    if (!outputFileName.empty())
    {
        outputFile.open(outputFileName, ios::app);
        outputFile << "MinPrimaryHits " << primaryMinHits << ", MinMatchedHits " << minMatchedHits << std::endl;
    }

    for (InteractionCountingMap::const_iterator iter = interactionCountingMap.begin(); iter != interactionCountingMap.end(); ++iter)
    {
        const InteractionType interactionType(iter->first);
        const CountingMap &countingMap(iter->second);
        std::cout << std::endl << ToString(interactionType) << std::endl;

        if (!outputFileName.empty())
            outputFile << std::endl << ToString(interactionType) << std::endl;

        for (CountingMap::const_iterator cIter = countingMap.begin(); cIter != countingMap.end(); ++cIter)
        {
            const ExpectedPrimary expectedPrimary(cIter->first);
            const CountingDetails &countingDetails(cIter->second);

            std::cout << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal << ", nPfos |0: " << countingDetails.m_nMatch0
                      << "|, |1: " << countingDetails.m_nMatch1 << "|, |2: " << countingDetails.m_nMatch2 << "|, |3+: " << countingDetails.m_nMatch3Plus << "|" << std::endl;

            if (!outputFileName.empty())
            {
                outputFile << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal << ", nPfos |0: " << countingDetails.m_nMatch0
                           << "|, |1: " << countingDetails.m_nMatch1 << "|, |2: " << countingDetails.m_nMatch2 << "|, |3+: " << countingDetails.m_nMatch3Plus << "|" << std::endl;
            }
        }
    }

    if (!outputFileName.empty())
        outputFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalyseInteractionEventResultMap(const InteractionEventResultMap &interactionEventResultMap, const std::string &outputFileName,
    const std::string &correctEventFileName, const bool histogramOutput, const std::string &prefix)
{
    // Intended for filling histograms, post-processing of information collected in main loop over ntuple, etc.
    std::cout << std::endl << "EVENT INFO " << std::endl;

    std::ofstream outputFile, correctEventFile;
    if (!outputFileName.empty()) outputFile.open(outputFileName, ios::app);
    if (!correctEventFileName.empty()) correctEventFile.open(correctEventFileName, ios::app);

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

                if (!correctEventFileName.empty())
                    correctEventFile << "Correct event: fileId: " << eIter->m_fileIdentifier << ", eventNumber: " << eIter->m_eventNumber << ", nuance: " << eIter->m_mcNeutrinoNuance << std::endl;
            }
        }

        std::cout << ToString(interactionType) << std::endl << "-nEvents " << eventResultList.size() << ", nCorrect " << nCorrectEvents
                  << ", fCorrect " << static_cast<float>(nCorrectEvents) / static_cast<float>(eventResultList.size()) << std::endl;

        if (!outputFileName.empty())
        {
            outputFile << std::endl << ToString(interactionType) << std::endl << "-nEvents " << eventResultList.size() << ", nCorrect " << nCorrectEvents
                       << ", fCorrect " << static_cast<float>(nCorrectEvents) / static_cast<float>(eventResultList.size()) << std::endl;
        }
    }

    if (histogramOutput)
        ProcessHistogramCollections(interactionPrimaryHistogramMap);

    if (!outputFileName.empty()) outputFile.close();
    if (!correctEventFileName.empty()) correctEventFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillEventHistogramCollection(const std::string &histPrefix, const EventResult &eventResult, EventHistogramCollection &eventHistogramCollection)
{
    if (!eventHistogramCollection.m_hVtxDeltaX)
    {
        eventHistogramCollection.m_hVtxDeltaX = new TH1F((histPrefix + "VtxDeltaX").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetRange(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetTitle("Vertex #DeltaX [cm]");
        eventHistogramCollection.m_hVtxDeltaX->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_hVtxDeltaY)
    {
        eventHistogramCollection.m_hVtxDeltaY = new TH1F((histPrefix + "VtxDeltaY").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetRange(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetTitle("Vertex #DeltaY [cm]");
        eventHistogramCollection.m_hVtxDeltaY->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_hVtxDeltaZ)
    {
        eventHistogramCollection.m_hVtxDeltaZ = new TH1F((histPrefix + "VtxDeltaZ").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetRange(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetTitle("Vertex #DeltaZ [cm]");
        eventHistogramCollection.m_hVtxDeltaZ->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_hVtxDeltaR)
    {
        eventHistogramCollection.m_hVtxDeltaR = new TH1F((histPrefix + "VtxDeltaR").c_str(), "", 40000, -100., 1900.);
        eventHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetRange(-0.1, +5.);
        eventHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetTitle("Vertex #DeltaR [cm]");
        eventHistogramCollection.m_hVtxDeltaR->GetYaxis()->SetTitle("nEvents");
    }

    if (!eventHistogramCollection.m_nRecoNeutrinos)
    {
        eventHistogramCollection.m_nRecoNeutrinos = new TH1F((histPrefix + "NRecoNeutrinos").c_str(), "", 11, -0.5, 10.5);
        eventHistogramCollection.m_nRecoNeutrinos->GetXaxis()->SetTitle("nRecoNeutrinos");
        eventHistogramCollection.m_nRecoNeutrinos->GetYaxis()->SetTitle("nEvents");
    }

    eventHistogramCollection.m_hVtxDeltaX->Fill(eventResult.m_vertexOffset.m_x);
    eventHistogramCollection.m_hVtxDeltaY->Fill(eventResult.m_vertexOffset.m_y);
    eventHistogramCollection.m_hVtxDeltaZ->Fill(eventResult.m_vertexOffset.m_z);
    eventHistogramCollection.m_hVtxDeltaR->Fill(std::sqrt(eventResult.m_vertexOffset.m_x * eventResult.m_vertexOffset.m_x + eventResult.m_vertexOffset.m_y * eventResult.m_vertexOffset.m_y + eventResult.m_vertexOffset.m_z * eventResult.m_vertexOffset.m_z));
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
        primaryHistogramCollection.m_hHitsAll->GetXaxis()->SetRange(1., +6000);
        primaryHistogramCollection.m_hHitsAll->GetXaxis()->SetTitle("nTrueHits");
        primaryHistogramCollection.m_hHitsAll->GetYaxis()->SetTitle("nEvents");
    }

    if (!primaryHistogramCollection.m_hHitsEfficiency)
    {
        primaryHistogramCollection.m_hHitsEfficiency = new TH1F((histPrefix + "HitsEfficiency").c_str(), "", nHitBins, hitsBinning);
        primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->SetRange(1., +6000);
        primaryHistogramCollection.m_hHitsEfficiency->GetXaxis()->SetTitle("nTrueHits");
        primaryHistogramCollection.m_hHitsEfficiency->GetYaxis()->SetRange(0., +1.01);
        primaryHistogramCollection.m_hHitsEfficiency->GetYaxis()->SetTitle("Efficiency");
    }

    if (!primaryHistogramCollection.m_hAngleAll)
    {
        primaryHistogramCollection.m_hAngleAll = new TH1F((histPrefix + "AngleAll").c_str(), "", 64, -M_PI, M_PI);
        primaryHistogramCollection.m_hAngleAll->GetXaxis()->SetRange(0., +3.2);
        primaryHistogramCollection.m_hAngleAll->GetXaxis()->SetTitle("#theta_{z}");
        primaryHistogramCollection.m_hAngleAll->GetYaxis()->SetTitle("nEvents");
    }

    if (!primaryHistogramCollection.m_hAngleEfficiency)
    {
        primaryHistogramCollection.m_hAngleEfficiency = new TH1F((histPrefix + "AngleEfficiency").c_str(), "", 64, -M_PI, M_PI);
        primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->SetRange(0., +3.2);
        primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->SetTitle("#theta_{z}");
        primaryHistogramCollection.m_hAngleEfficiency->GetYaxis()->SetRange(0., +1.01);
        primaryHistogramCollection.m_hAngleEfficiency->GetYaxis()->SetTitle("Efficiency");
    }

    if (!primaryHistogramCollection.m_hCompleteness)
    {
        primaryHistogramCollection.m_hCompleteness = new TH1F((histPrefix + "Completeness").c_str(), "", 51, -0.01, 1.01);
        primaryHistogramCollection.m_hCompleteness->GetXaxis()->SetTitle("Completeness");
        primaryHistogramCollection.m_hCompleteness->GetYaxis()->SetRange(0., +1.01);
        primaryHistogramCollection.m_hCompleteness->GetYaxis()->SetTitle("Fraction of Particles");
    }

    if (!primaryHistogramCollection.m_hPurity)
    {
        primaryHistogramCollection.m_hPurity = new TH1F((histPrefix + "Purity").c_str(), "", 51, -0.01, 1.01);
        primaryHistogramCollection.m_hPurity->GetXaxis()->SetTitle("Purity");
        primaryHistogramCollection.m_hPurity->GetYaxis()->SetRange(0., +1.01);
        primaryHistogramCollection.m_hPurity->GetYaxis()->SetTitle("Fraction of Particles");
    }

    primaryHistogramCollection.m_hHitsAll->Fill(primaryResult.m_nTrueHits);
    primaryHistogramCollection.m_hAngleAll->Fill(primaryResult.m_trueAngle);

    if (primaryResult.m_nPfoMatches > 0)
    {
        primaryHistogramCollection.m_hHitsEfficiency->Fill(primaryResult.m_nTrueHits);
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
