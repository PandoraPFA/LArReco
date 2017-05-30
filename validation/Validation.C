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

#include <fstream>
#include <sstream>

void Validation(const std::string &inputFiles)
{
    Validation(inputFiles, Parameters(), InteractionTypeMap());
}

//------------------------------------------------------------------------------------------------------------------------------------------

void Validation(const std::string &inputFiles, const Parameters &parameters)
{
    Validation(inputFiles, parameters, InteractionTypeMap());
}

//------------------------------------------------------------------------------------------------------------------------------------------

void Validation(const std::string &inputFiles, const Parameters &parameters, const InteractionTypeMap &interactionTypeMap)
{
    TChain *pTChain = new TChain("Validation", "pTChain");
    pTChain->Add(inputFiles.c_str());

    HitCountingMap hitCountingMap;

    if (!parameters.m_hitCountingFileName.empty())
        FillHitCountingMap(parameters, hitCountingMap);

    // To store final output
    InteractionCountingMap interactionCountingMap;
    InteractionEventResultMap interactionEventResultMap;

    int nEvents(0), nProcessedEvents(0);

    for (int iEntry = 0; iEntry < pTChain->GetEntries(); )
    {
        SimpleMCEvent simpleMCEvent;
        iEntry += ValidationIO::ReadNextEvent(pTChain, iEntry, simpleMCEvent);

        if (nEvents++ < parameters.m_skipEvents)
            continue;

        if (!interactionTypeMap.empty() && !PassesInteractionTypeCheck(simpleMCEvent, parameters, interactionTypeMap))
            continue;

        if (!hitCountingMap.empty() && !PassesHitCountingCheck(simpleMCEvent, parameters, hitCountingMap))
            continue;

        if (nEvents % 50 == 0)
            std::cout << "nEvents " << nEvents << "\r" << std::flush;

        if (nProcessedEvents++ >= parameters.m_nEventsToProcess)
            break;

        if (parameters.m_displayEvents)
            ValidationIO::DisplaySimpleMCEvent(simpleMCEvent);

        // Analysis code starts here
        PfoMatchingMap pfoMatchingMap;
        FinalisePfoMatching(simpleMCEvent, parameters, pfoMatchingMap);

	 if (parameters.m_displayMatchedEvents)
            DisplaySimpleMCEventMatches(simpleMCEvent, pfoMatchingMap, parameters);

        CountPfoMatches(simpleMCEvent, pfoMatchingMap, parameters, interactionCountingMap, interactionEventResultMap);
    }

    // Processing of final output
    DisplayInteractionCountingMap(interactionCountingMap, parameters);
    AnalyseInteractionEventResultMap(interactionEventResultMap, parameters);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void WriteHitCountingMap(const std::string inputFiles, const std::string outputFile)
{
    TChain *pTChain = new TChain("Validation", "pTChain");
    pTChain->Add(inputFiles.c_str());

    HitCountingMap hitCountingMap;
    int nEvents(0);
    for (int iEntry = 0; iEntry < pTChain->GetEntries(); )
    {
        SimpleMCEvent simpleMCEvent;
        iEntry += ValidationIO::ReadNextEvent(pTChain, iEntry, simpleMCEvent);

        if (nEvents++ % 50 == 0)
            std::cout << "nEvents " << nEvents << "\r" << std::flush;

        hitCountingMap[simpleMCEvent.m_fileIdentifier][simpleMCEvent.m_eventNumber] = simpleMCEvent.m_nEventNeutrinoHitsTotal;
    }

    std::ofstream myfile;
    myfile.open(outputFile);

    for (HitCountingMap::const_iterator fIter = hitCountingMap.begin(), fIterEnd = hitCountingMap.end(); fIter != fIterEnd; ++fIter)
    {
        for (EventToNHitsMap::const_iterator eIter = fIter->second.begin(), eIterEnd = fIter->second.end(); eIter != eIterEnd; ++eIter)
            myfile << fIter->first << ", " << eIter->first << ", " << eIter->second << std::endl;
    }

    myfile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillHitCountingMap(const Parameters &parameters, HitCountingMap &hitCountingMap)
{
    std::ifstream myfile;
    myfile.open(parameters.m_hitCountingFileName);
    std::string line;

    while (std::getline(myfile, line))
    {
        int fileId(-1), eventNumber(-1), nHits(-1), counter(0);
        std::istringstream linestream(line);
        std::string value;

        while (std::getline(linestream, value, ','))
        {
            int &output((0 == counter) ? fileId : (1 == counter) ? eventNumber : nHits);
            std::istringstream iss(value);
            iss >> output;
            ++counter;
        }

        if (counter != 3)
        {
            std::cout << "Invalid entries in hit counting map " << std::endl;
            throw;
        }

        hitCountingMap[fileId][eventNumber] = nHits;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassesHitCountingCheck(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, const HitCountingMap &hitCountingMap)
{
    HitCountingMap::const_iterator fIter = hitCountingMap.find(simpleMCEvent.m_fileIdentifier);

    if (hitCountingMap.end() == fIter)
        return false;

    EventToNHitsMap::const_iterator eIter = fIter->second.find(simpleMCEvent.m_eventNumber);

    if (fIter->second.end() == eIter)
        return false;

    const float hitFraction((eIter->second > 0) ? static_cast<float>(simpleMCEvent.m_nEventNeutrinoHitsTotal) / static_cast<float>(eIter->second) : 0.f);
    return (hitFraction > parameters.m_minFractionOfAllHits);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PopulateInteractionTypeMap(const std::string inputFiles, const Parameters &parameters, InteractionTypeMap &interactionTypeMap)
{
    TChain *pTChain = new TChain("Validation", "pTChain");
    pTChain->Add(inputFiles.c_str());

    HitCountingMap hitCountingMap;
    int nEvents(0);

    for (int iEntry = 0; iEntry < pTChain->GetEntries(); )
    {
        SimpleMCEvent simpleMCEvent;
        iEntry += ValidationIO::ReadNextEvent(pTChain, iEntry, simpleMCEvent);

        if (nEvents++ % 50 == 0)
            std::cout << "nEvents " << nEvents << "\r" << std::flush;

        if (parameters.m_applyFiducialCut && !PassFiducialCut(simpleMCEvent, parameters))
            continue;

        const InteractionType interactionType(GetInteractionType(simpleMCEvent, parameters));

        if (interactionTypeMap.count(simpleMCEvent.m_fileIdentifier) && interactionTypeMap[simpleMCEvent.m_fileIdentifier].count(simpleMCEvent.m_eventNumber))
            std::cout << "File id and event number already present " << simpleMCEvent.m_fileIdentifier << ", " << simpleMCEvent.m_eventNumber << std::endl;

        int nPrimaryHits(0);

        for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
            nPrimaryHits += pIter->m_nMCHitsTotal;

        interactionTypeMap[simpleMCEvent.m_fileIdentifier][simpleMCEvent.m_eventNumber] = IntTypeAndHits(interactionType, nPrimaryHits);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassesInteractionTypeCheck(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, const InteractionTypeMap &origInteractionTypeMap)
{
    InteractionTypeMap::const_iterator origFIter(origInteractionTypeMap.find(simpleMCEvent.m_fileIdentifier));

    if (origInteractionTypeMap.end() == origFIter)
        return false;

    EventToInteractionTypeMap::const_iterator origEIter(origFIter->second.find(simpleMCEvent.m_eventNumber));

    if (origFIter->second.end() == origEIter)
        return false;

    InteractionType originalInteractionType(origEIter->second.first);
    const int nOriginalPrimaryHits(origEIter->second.second);

    const InteractionType newInteractionType(GetInteractionType(simpleMCEvent, parameters));

    int nNewPrimaryHits(0);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
        nNewPrimaryHits += pIter->m_nMCHitsTotal;

    if ((newInteractionType != originalInteractionType) || (static_cast<float>(nNewPrimaryHits) < parameters.m_minPrimaryHitsFraction * static_cast<float>(nOriginalPrimaryHits)))
    {
        std::cout << "File " << simpleMCEvent.m_fileIdentifier << ", Evt " << simpleMCEvent.m_eventNumber << ", From " << ToString(originalInteractionType)
                  << ", To " << ToString(newInteractionType) << ", OrigHits " << nOriginalPrimaryHits << ", CurrentHits " << nNewPrimaryHits << std::endl;
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CompareInteractionTypeMaps(const InteractionTypeMap &originalInteractionTypeMap, const InteractionTypeMap &newInteractionTypeMap, const Parameters &parameters)
{
    TH2F *const pComparison = new TH2F((parameters.m_histPrefix + "InteractionTypeMixing").c_str(), "", ALL_INTERACTIONS + 1, 0, ALL_INTERACTIONS, ALL_INTERACTIONS + 1, 0, ALL_INTERACTIONS);
    pComparison->GetXaxis()->SetTitle("Original Interaction Type");
    pComparison->GetYaxis()->SetTitle("New Interaction Type");

    for (unsigned int i = 0; i < ALL_INTERACTIONS; ++i)
    {
        pComparison->GetXaxis()->SetBinLabel(i + 1, ToString(static_cast<InteractionType>(i)).c_str());
        pComparison->GetYaxis()->SetBinLabel(i + 1, ToString(static_cast<InteractionType>(i)).c_str());
    }

    for (InteractionTypeMap::const_iterator origFIter = originalInteractionTypeMap.begin(), origFIterEnd = originalInteractionTypeMap.end(); origFIter != origFIterEnd; ++origFIter)
    {
        const int fileId(origFIter->first);

        for (EventToInteractionTypeMap::const_iterator origEIter = origFIter->second.begin(), origEIterEnd = origFIter->second.end(); origEIter != origEIterEnd; ++origEIter)
        {
            const int eventNumber(origEIter->first);
            const InteractionType originalInteractionType(origEIter->second.first);
            const int nOriginalPrimaryHits(origEIter->second.second);

            InteractionTypeMap::const_iterator newFIter(newInteractionTypeMap.find(fileId));

            if (newInteractionTypeMap.end() == newFIter)
                continue;

            EventToInteractionTypeMap::const_iterator newEIter(newFIter->second.find(eventNumber));

            if (newFIter->second.end() == newEIter)
                continue;

            InteractionType newInteractionType(newEIter->second.first);
            const int nNewPrimaryHits(newEIter->second.second);

            if ((newInteractionType != originalInteractionType) || (static_cast<float>(nNewPrimaryHits) < parameters.m_minPrimaryHitsFraction * static_cast<float>(nOriginalPrimaryHits)))
            {
                std::cout << "File " << fileId << ", Evt " << eventNumber << ", From " << ToString(originalInteractionType) << ", To " << ToString(newInteractionType)
                          << ", OrigHits " << nOriginalPrimaryHits << ", CurrentHits " << nNewPrimaryHits << std::endl;
                newInteractionType = OTHER_INTERACTION;
            }

            pComparison->Fill(originalInteractionType, newInteractionType, 1.);
        }
    }

//    // Derive nicer version here
//    for (unsigned int i = 0; i < ALL_INTERACTIONS; ++i)
//    {
//        const int nEntries(pComparison->Integral(i + 1, i + 1, 0, ALL_INTERACTIONS));
//
//        if (0 == nEntries)
//            continue;
//
//        for (unsigned int j = 0; j < ALL_INTERACTIONS; ++j)
//            pComparison->SetBinContent(i + 1, j + 1, 100. * pComparison->GetBinContent(i + 1, j + 1) / static_cast<float>(nEntries));
//    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, PfoMatchingMap &pfoMatchingMap)
{
    // Get best matches, one-by-one, until no more strong matches possible
    IntSet usedMCIds, usedPfoIds;
    while (GetStrongestPfoMatch(simpleMCEvent, parameters, usedMCIds, usedPfoIds, pfoMatchingMap)) {}

    // Assign any remaining pfos to primaries, based on number of matched hits
    GetRemainingPfoMatches(simpleMCEvent, parameters, usedPfoIds, pfoMatchingMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, IntSet &usedMCIds, IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap)
{
    int bestPfoMatchId(-1);
    MatchingDetails bestMatchingDetails;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (!parameters.m_useSmallPrimaries && !IsGoodMCPrimary(simpleMCPrimary, parameters))
            continue;

        if (usedMCIds.count(simpleMCPrimary.m_id))
            continue;

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);
            
            if (usedPfoIds.count(simpleMatchedPfo.m_id))
                continue;

            if (!IsGoodMatch(simpleMCPrimary, simpleMatchedPfo, parameters))
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

void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap)
{
    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (!parameters.m_useSmallPrimaries && !IsGoodMCPrimary(simpleMCPrimary, parameters))
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

bool IsGoodMCPrimary(const SimpleMCPrimary &simpleMCPrimary, const Parameters &parameters)
{
    if (simpleMCPrimary.m_nGoodMCHitsTotal < parameters.m_minPrimaryGoodHits)
        return false;

    int nGoodViews(0);
    if (simpleMCPrimary.m_nGoodMCHitsU >= parameters.m_minHitsForGoodView) ++nGoodViews;
    if (simpleMCPrimary.m_nGoodMCHitsV >= parameters.m_minHitsForGoodView) ++nGoodViews;
    if (simpleMCPrimary.m_nGoodMCHitsW >= parameters.m_minHitsForGoodView) ++nGoodViews;

    if (nGoodViews < parameters.m_minPrimaryGoodViews)
        return false;

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool HasMatch(const SimpleMCPrimary &simpleMCPrimary, const PfoMatchingMap &pfoMatchingMap, const Parameters &parameters)
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

bool IsGoodMatch(const SimpleMCPrimary &simpleMCPrimary, const SimpleMatchedPfo &simpleMatchedPfo, const Parameters &parameters)
{
    const unsigned int absMCPdgCode(std::abs(simpleMCPrimary.m_pdgCode));

    if (parameters.m_correctTrackShowerId && (
        ((absMCPdgCode == 13 || absMCPdgCode == 2212 || absMCPdgCode == 211) && (13 != simpleMatchedPfo.m_pdgCode)) ||
        ((absMCPdgCode == 22 || absMCPdgCode == 11) && (11 != simpleMatchedPfo.m_pdgCode)) ))
    {
        return false;
    }

    const float purity((simpleMatchedPfo.m_nPfoHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMatchedPfo.m_nPfoHitsTotal) : 0.f);
    const float completeness((simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0.f);

    return ((simpleMatchedPfo.m_nMatchedHitsTotal >= parameters.m_minSharedHits) && (purity >= parameters.m_minPurity) && (completeness >= parameters.m_minCompleteness));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const PfoMatchingMap &pfoMatchingMap, const Parameters &parameters)
{
    std::cout << "---PROCESSED-MATCHING-OUTPUT--------------------------------------------------------------------" << std::endl;
    std::cout << "MinPrimaryGoodHits " << parameters.m_minPrimaryGoodHits << ", MinHitsForGoodView " << parameters.m_minHitsForGoodView << ", MinPrimaryGoodViews " << parameters.m_minPrimaryGoodViews << std::endl;
    std::cout << "UseSmallPrimaries " << parameters.m_useSmallPrimaries << ", MinSharedHits " << parameters.m_minSharedHits << ", MinCompleteness " << parameters.m_minCompleteness << ", MinPurity " << parameters.m_minPurity << std::endl;

    bool isCorrect(true), isCalculable(false);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);
        const bool hasMatch(HasMatch(simpleMCPrimary, pfoMatchingMap, parameters));
        const bool isTargetPrimary(IsGoodMCPrimary(simpleMCPrimary, parameters) && (2112 != simpleMCPrimary.m_pdgCode));
        unsigned int absMCPdgCode(std::abs(simpleMCPrimary.m_pdgCode));                                                    
        bool isTrackLike(((absMCPdgCode == 22 || absMCPdgCode == 11)) ? false : true);                                         
        bool isMisID(false);                                                           

        if (!hasMatch && !isTargetPrimary)
            continue;

        std::cout << std::endl << (!isTargetPrimary ? "(Non target) " : "") << "Primary " << simpleMCPrimary.m_id
                  << ", PDG " << simpleMCPrimary.m_pdgCode << ", nMCHits " << simpleMCPrimary.m_nMCHitsTotal
                  << " (" << simpleMCPrimary.m_nMCHitsU << ", " << simpleMCPrimary.m_nMCHitsV << ", " << simpleMCPrimary.m_nMCHitsW << "),"
                  << " [nGood " << simpleMCPrimary.m_nGoodMCHitsTotal << " (" << simpleMCPrimary.m_nGoodMCHitsU << ", " << simpleMCPrimary.m_nGoodMCHitsV
                  << ", " << simpleMCPrimary.m_nGoodMCHitsW << ")]" << " and isTrackLike?" << isTrackLike << std::endl;

        if (2112 != simpleMCPrimary.m_pdgCode)
            isCalculable = true;

        unsigned int nMatches(0);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                const bool isGoodMatch(IsGoodMatch(simpleMCPrimary, simpleMatchedPfo, parameters));
		
                if (isGoodMatch) ++nMatches;
                std::cout << "-" << (!isGoodMatch ? "(Below threshold) " : "") << "MatchedPfo " << simpleMatchedPfo.m_id;

		unsigned int absPfoPdgCode(std::abs(simpleMatchedPfo.m_pdgCode));                                                                       
                bool isPfoTrackLike(((absPfoPdgCode == 22 || absPfoPdgCode == 11)) ? false : true);       
		isMisID = ((isTrackLike!=isPfoTrackLike) ? true : false);

                if (simpleMatchedPfo.m_parentId >= 0) std::cout << ", ParentPfo " << simpleMatchedPfo.m_parentId;

                std::cout << ", PDG " << simpleMatchedPfo.m_pdgCode << ", nMatchedHits " << simpleMatchedPfo.m_nMatchedHitsTotal
                          << " (" << simpleMatchedPfo.m_nMatchedHitsU << ", " << simpleMatchedPfo.m_nMatchedHitsV << ", " << simpleMatchedPfo.m_nMatchedHitsW << ")"
                          << ", nPfoHits " << simpleMatchedPfo.m_nPfoHitsTotal
                          << " (" << simpleMatchedPfo.m_nPfoHitsU << ", " << simpleMatchedPfo.m_nPfoHitsV << ", " << simpleMatchedPfo.m_nPfoHitsW << ")" << " and isPfoTrackLike^" << isPfoTrackLike << " and isMisID? " << isMisID << std::endl;
            }
        }

        if (isTargetPrimary && (1 != nMatches))
            isCorrect = false;
    }

    std::cout << std::endl << "Is correct? " << (isCorrect && isCalculable) << std::endl;
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const PfoMatchingMap &pfoMatchingMap, const Parameters &parameters,
    InteractionCountingMap &interactionCountingMap, InteractionEventResultMap &interactionEventResultMap)
{
    if (parameters.m_applyFiducialCut && !PassFiducialCut(simpleMCEvent, parameters))
        return;

    const int nRecoHitsTotal(simpleMCEvent.m_nRecoNeutrinoHitsTotal + simpleMCEvent.m_nRecoOtherHitsTotal);
    const float neutrinoPurity((nRecoHitsTotal > 0) ? static_cast<float>(simpleMCEvent.m_nRecoNeutrinoHitsTotal) / static_cast<float>(nRecoHitsTotal) : -1.f); // ATTN Think about where to place null value in histogram
    const float neutrinoCompleteness((simpleMCEvent.m_nEventNeutrinoHitsTotal > 0) ? static_cast<float>(simpleMCEvent.m_nRecoNeutrinoHitsTotal) / static_cast<float>(simpleMCEvent.m_nEventNeutrinoHitsTotal) : 0.f);

    if ((neutrinoPurity < parameters.m_minNeutrinoPurity) || (neutrinoCompleteness < parameters.m_minNeutrinoCompleteness))
        return;

    bool hasTargetPrimary(false);
    const InteractionType interactionType(GetInteractionType(simpleMCEvent, parameters));

    EventResult eventResult;
    eventResult.m_fileIdentifier = simpleMCEvent.m_fileIdentifier;
    eventResult.m_eventNumber = simpleMCEvent.m_eventNumber;
    eventResult.m_mcNeutrinoNuance = simpleMCEvent.m_mcNeutrinoNuance;
    eventResult.m_nRecoNeutrinos = simpleMCEvent.m_nRecoNeutrinos;
    eventResult.m_nTrueNeutrinos = simpleMCEvent.m_nMCNeutrinos;
    eventResult.m_neutrinoPurity = neutrinoPurity;
    eventResult.m_neutrinoCompleteness = neutrinoCompleteness;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);
        const ExpectedPrimary expectedPrimary(GetExpectedPrimary(simpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, parameters));

        const bool isTargetPrimary(IsGoodMCPrimary(simpleMCPrimary, parameters) && (2112 != simpleMCPrimary.m_pdgCode));

        if (!isTargetPrimary)
            continue;

        hasTargetPrimary = true;
        CountingDetails &countingDetails = interactionCountingMap[interactionType][expectedPrimary];
        PrimaryResult &primaryResult = eventResult.m_primaryResultMap[expectedPrimary];

        ++countingDetails.m_nTotal;
        unsigned int nMatches(0), nBestMatchedHits(0), nBestRecoHits(0);
        float bestMatchPurity(0.f), bestCompleteness(0.f);
        unsigned int absMCPdgCode(std::abs(simpleMCPrimary.m_pdgCode));
	bool isTrackLike(((absMCPdgCode == 22 || absMCPdgCode == 11)) ? false : true);
	bool isMisID(false);

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (simpleMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
            {
                if (!IsGoodMatch(simpleMCPrimary, simpleMatchedPfo, parameters))
                    continue;

                ++nMatches;
                const float purity((simpleMatchedPfo.m_nPfoHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMatchedPfo.m_nPfoHitsTotal) : 0);
                const float completeness((simpleMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(simpleMCPrimary.m_nMCHitsTotal) : 0);

		unsigned int absPfoPdgCode(std::abs(simpleMatchedPfo.m_pdgCode));
		bool isPfoTrackLike(((absPfoPdgCode == 22 || absPfoPdgCode == 11)) ? false : true);

                if (completeness > bestCompleteness)
                {
                    bestMatchPurity = purity;
                    bestCompleteness = completeness;
                    nBestMatchedHits = simpleMatchedPfo.m_nMatchedHitsTotal;
                    nBestRecoHits = simpleMatchedPfo.m_nPfoHitsTotal;
		    isMisID = ((isTrackLike!=isPfoTrackLike) ? true : false);
		}
            }
        }

        if (0 == nMatches) ++countingDetails.m_nMatch0;
        else if (1 == nMatches) ++countingDetails.m_nMatch1;
        else if (2 == nMatches) ++countingDetails.m_nMatch2;
        else ++countingDetails.m_nMatch3Plus;

	if(isMisID)
	  ++countingDetails.m_misID;

        primaryResult.m_nPfoMatches = nMatches;
        primaryResult.m_bestCompleteness = bestCompleteness;
        primaryResult.m_bestMatchPurity = bestMatchPurity;
        primaryResult.m_nTrueHits = simpleMCPrimary.m_nMCHitsTotal;

        const float pTot(std::sqrt(simpleMCPrimary.m_momentum.m_x * simpleMCPrimary.m_momentum.m_x + simpleMCPrimary.m_momentum.m_y * simpleMCPrimary.m_momentum.m_y + simpleMCPrimary.m_momentum.m_z * simpleMCPrimary.m_momentum.m_z));
        primaryResult.m_trueMomentum = pTot;
        primaryResult.m_trueAngle = GetTrueAngle(simpleMCEvent, parameters, interactionType, expectedPrimary);

        primaryResult.m_nBestMatchedHits = nBestMatchedHits;
        primaryResult.m_nBestRecoHits = nBestRecoHits;

	primaryResult.m_isTrackLike = isTrackLike;
	primaryResult.m_isMisID = isMisID;
    }

    if ((0 < simpleMCEvent.m_nRecoNeutrinos) && (1 == simpleMCEvent.m_nMCNeutrinos))
    {
        eventResult.m_vertexOffset = simpleMCEvent.m_recoNeutrinoVtx - simpleMCEvent.m_mcNeutrinoVtx;
        eventResult.m_vertexOffset.m_x = eventResult.m_vertexOffset.m_x - parameters.m_vertexXCorrection;
        GetVtxShwDistance(simpleMCEvent, parameters, interactionType, pfoMatchingMap, eventResult);
    }

    if (hasTargetPrimary)
        interactionEventResultMap[interactionType].push_back(eventResult);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PassFiducialCut(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters)
{
    // DUNE 10kt
    if (parameters.m_useDune10ktFidVol)
    {
        // DUNE 10kt
        const float eVx(1490.), eVy(1207.85), eVz(5809.75);
        const float xBorder(10.), yBorder(20.), zBorder(10.);

        if ((simpleMCEvent.m_mcNeutrinoVtx.m_x < (eVx / 2. - xBorder)) && (simpleMCEvent.m_mcNeutrinoVtx.m_x > (-eVx / 2. + xBorder)) &&
            (simpleMCEvent.m_mcNeutrinoVtx.m_y < (eVy / 2. - yBorder)) && (simpleMCEvent.m_mcNeutrinoVtx.m_y > (-eVy / 2. + yBorder)) &&
            (simpleMCEvent.m_mcNeutrinoVtx.m_z < (eVz - zBorder)) && (simpleMCEvent.m_mcNeutrinoVtx.m_z > zBorder) )
        {
            return true;
        }
    }
    else
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
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------

InteractionType GetInteractionType(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters)
{
    unsigned int nNonNeutrons(0), nMuons(0), nElectrons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (!IsGoodMCPrimary(simpleMCPrimary, parameters))
            continue;

        if (2112 != simpleMCPrimary.m_pdgCode)
            ++nNonNeutrons;

        if (13 == simpleMCPrimary.m_pdgCode) ++nMuons;
        if (11 == simpleMCPrimary.m_pdgCode) ++nElectrons;
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

        if ((1 == nNonNeutrons) && (1 == nElectrons) && (0 == nProtons)) return CCQEL_E;
        if ((2 == nNonNeutrons) && (1 == nElectrons) && (1 == nProtons)) return CCQEL_E_P;
        if ((3 == nNonNeutrons) && (1 == nElectrons) && (2 == nProtons)) return CCQEL_E_P_P;
        if ((4 == nNonNeutrons) && (1 == nElectrons) && (3 == nProtons)) return CCQEL_E_P_P_P;
        if ((5 == nNonNeutrons) && (1 == nElectrons) && (4 == nProtons)) return CCQEL_E_P_P_P_P;
        if ((6 == nNonNeutrons) && (1 == nElectrons) && (5 == nProtons)) return CCQEL_E_P_P_P_P_P;
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

        if ((1 == nNonNeutrons) && (1 == nElectrons) && (0 == nProtons)) return CCRES_E;
        if ((2 == nNonNeutrons) && (1 == nElectrons) && (1 == nProtons)) return CCRES_E_P;
        if ((3 == nNonNeutrons) && (1 == nElectrons) && (2 == nProtons)) return CCRES_E_P_P;
        if ((4 == nNonNeutrons) && (1 == nElectrons) && (3 == nProtons)) return CCRES_E_P_P_P;
        if ((5 == nNonNeutrons) && (1 == nElectrons) && (4 == nProtons)) return CCRES_E_P_P_P_P;
        if ((6 == nNonNeutrons) && (1 == nElectrons) && (5 == nProtons)) return CCRES_E_P_P_P_P_P;

        if ((2 == nNonNeutrons) && (1 == nElectrons) && (0 == nProtons) && (1 == nPiPlus)) return CCRES_E_PIPLUS;
        if ((3 == nNonNeutrons) && (1 == nElectrons) && (1 == nProtons) && (1 == nPiPlus)) return CCRES_E_P_PIPLUS;
        if ((4 == nNonNeutrons) && (1 == nElectrons) && (2 == nProtons) && (1 == nPiPlus)) return CCRES_E_P_P_PIPLUS;
        if ((5 == nNonNeutrons) && (1 == nElectrons) && (3 == nProtons) && (1 == nPiPlus)) return CCRES_E_P_P_P_PIPLUS;
        if ((6 == nNonNeutrons) && (1 == nElectrons) && (4 == nProtons) && (1 == nPiPlus)) return CCRES_E_P_P_P_P_PIPLUS;
        if ((7 == nNonNeutrons) && (1 == nElectrons) && (5 == nProtons) && (1 == nPiPlus)) return CCRES_E_P_P_P_P_P_PIPLUS;

        if ((2 == nNonNeutrons) && (1 == nElectrons) && (0 == nProtons) && (1 == nPhotons)) return CCRES_E_PHOTON;
        if ((3 == nNonNeutrons) && (1 == nElectrons) && (1 == nProtons) && (1 == nPhotons)) return CCRES_E_P_PHOTON;
        if ((4 == nNonNeutrons) && (1 == nElectrons) && (2 == nProtons) && (1 == nPhotons)) return CCRES_E_P_P_PHOTON;
        if ((5 == nNonNeutrons) && (1 == nElectrons) && (3 == nProtons) && (1 == nPhotons)) return CCRES_E_P_P_P_PHOTON;
        if ((6 == nNonNeutrons) && (1 == nElectrons) && (4 == nProtons) && (1 == nPhotons)) return CCRES_E_P_P_P_P_PHOTON;
        if ((7 == nNonNeutrons) && (1 == nElectrons) && (5 == nProtons) && (1 == nPhotons)) return CCRES_E_P_P_P_P_P_PHOTON;

        if ((3 == nNonNeutrons) && (1 == nElectrons) && (0 == nProtons) && (2 == nPhotons)) return CCRES_E_PIZERO;
        if ((4 == nNonNeutrons) && (1 == nElectrons) && (1 == nProtons) && (2 == nPhotons)) return CCRES_E_P_PIZERO;
        if ((5 == nNonNeutrons) && (1 == nElectrons) && (2 == nProtons) && (2 == nPhotons)) return CCRES_E_P_P_PIZERO;
        if ((6 == nNonNeutrons) && (1 == nElectrons) && (3 == nProtons) && (2 == nPhotons)) return CCRES_E_P_P_P_PIZERO;
        if ((7 == nNonNeutrons) && (1 == nElectrons) && (4 == nProtons) && (2 == nPhotons)) return CCRES_E_P_P_P_P_PIZERO;
        if ((8 == nNonNeutrons) && (1 == nElectrons) && (5 == nProtons) && (2 == nPhotons)) return CCRES_E_P_P_P_P_P_PIZERO;
    }

    if ((simpleMCEvent.m_mcNeutrinoNuance >= 1006) && (simpleMCEvent.m_mcNeutrinoNuance <= 1009))
    {
        if ((1 == nNonNeutrons) && (1 == nProtons)) return NCRES_P;
        if ((2 == nNonNeutrons) && (2 == nProtons)) return NCRES_P_P;
        if ((3 == nNonNeutrons) && (3 == nProtons)) return NCRES_P_P_P;
        if ((4 == nNonNeutrons) && (4 == nProtons)) return NCRES_P_P_P_P;
        if ((5 == nNonNeutrons) && (5 == nProtons)) return NCRES_P_P_P_P_P;

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

    if (simpleMCEvent.m_mcNeutrinoNuance == 1091)
    {
        if ((1 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons)) return CCDIS_MU;
        if ((2 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons)) return CCDIS_MU_P;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons)) return CCDIS_MU_P_P;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons)) return CCDIS_MU_P_P_P;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons)) return CCDIS_MU_P_P_P_P;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons)) return CCDIS_MU_P_P_P_P_P;

        if ((2 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons) && (1 == nPiPlus)) return CCDIS_MU_PIPLUS;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (1 == nPiPlus)) return CCDIS_MU_P_PIPLUS;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (1 == nPiPlus)) return CCDIS_MU_P_P_PIPLUS;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons) && (1 == nPiPlus)) return CCDIS_MU_P_P_P_PIPLUS;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons) && (1 == nPiPlus)) return CCDIS_MU_P_P_P_P_PIPLUS;
        if ((7 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons) && (1 == nPiPlus)) return CCDIS_MU_P_P_P_P_P_PIPLUS;

        if ((2 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons) && (1 == nPhotons)) return CCDIS_MU_PHOTON;
        if ((3 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (1 == nPhotons)) return CCDIS_MU_P_PHOTON;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (1 == nPhotons)) return CCDIS_MU_P_P_PHOTON;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons) && (1 == nPhotons)) return CCDIS_MU_P_P_P_PHOTON;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons) && (1 == nPhotons)) return CCDIS_MU_P_P_P_P_PHOTON;
        if ((7 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons) && (1 == nPhotons)) return CCDIS_MU_P_P_P_P_P_PHOTON;

        if ((3 == nNonNeutrons) && (1 == nMuons) && (0 == nProtons) && (2 == nPhotons)) return CCDIS_MU_PIZERO;
        if ((4 == nNonNeutrons) && (1 == nMuons) && (1 == nProtons) && (2 == nPhotons)) return CCDIS_MU_P_PIZERO;
        if ((5 == nNonNeutrons) && (1 == nMuons) && (2 == nProtons) && (2 == nPhotons)) return CCDIS_MU_P_P_PIZERO;
        if ((6 == nNonNeutrons) && (1 == nMuons) && (3 == nProtons) && (2 == nPhotons)) return CCDIS_MU_P_P_P_PIZERO;
        if ((7 == nNonNeutrons) && (1 == nMuons) && (4 == nProtons) && (2 == nPhotons)) return CCDIS_MU_P_P_P_P_PIZERO;
        if ((8 == nNonNeutrons) && (1 == nMuons) && (5 == nProtons) && (2 == nPhotons)) return CCDIS_MU_P_P_P_P_P_PIZERO;
    }

    if (simpleMCEvent.m_mcNeutrinoNuance == 1092)
    {
        if ((1 == nNonNeutrons) && (1 == nProtons)) return NCDIS_P;
        if ((2 == nNonNeutrons) && (2 == nProtons)) return NCDIS_P_P;
        if ((3 == nNonNeutrons) && (3 == nProtons)) return NCDIS_P_P_P;
        if ((4 == nNonNeutrons) && (4 == nProtons)) return NCDIS_P_P_P_P;
        if ((5 == nNonNeutrons) && (5 == nProtons)) return NCDIS_P_P_P_P_P;

        if ((1 == nNonNeutrons) && (0 == nProtons) && (1 == nPiPlus)) return NCDIS_PIPLUS;
        if ((2 == nNonNeutrons) && (1 == nProtons) && (1 == nPiPlus)) return NCDIS_P_PIPLUS;
        if ((3 == nNonNeutrons) && (2 == nProtons) && (1 == nPiPlus)) return NCDIS_P_P_PIPLUS;
        if ((4 == nNonNeutrons) && (3 == nProtons) && (1 == nPiPlus)) return NCDIS_P_P_P_PIPLUS;
        if ((5 == nNonNeutrons) && (4 == nProtons) && (1 == nPiPlus)) return NCDIS_P_P_P_P_PIPLUS;
        if ((6 == nNonNeutrons) && (5 == nProtons) && (1 == nPiPlus)) return NCDIS_P_P_P_P_P_PIPLUS;

        if ((1 == nNonNeutrons) && (0 == nProtons) && (1 == nPiMinus)) return NCDIS_PIMINUS;
        if ((2 == nNonNeutrons) && (1 == nProtons) && (1 == nPiMinus)) return NCDIS_P_PIMINUS;
        if ((3 == nNonNeutrons) && (2 == nProtons) && (1 == nPiMinus)) return NCDIS_P_P_PIMINUS;
        if ((4 == nNonNeutrons) && (3 == nProtons) && (1 == nPiMinus)) return NCDIS_P_P_P_PIMINUS;
        if ((5 == nNonNeutrons) && (4 == nProtons) && (1 == nPiMinus)) return NCDIS_P_P_P_P_PIMINUS;
        if ((6 == nNonNeutrons) && (5 == nProtons) && (1 == nPiMinus)) return NCDIS_P_P_P_P_P_PIMINUS;

        if ((1 == nNonNeutrons) && (0 == nProtons) && (1 == nPhotons)) return NCDIS_PHOTON;
        if ((2 == nNonNeutrons) && (1 == nProtons) && (1 == nPhotons)) return NCDIS_P_PHOTON;
        if ((3 == nNonNeutrons) && (2 == nProtons) && (1 == nPhotons)) return NCDIS_P_P_PHOTON;
        if ((4 == nNonNeutrons) && (3 == nProtons) && (1 == nPhotons)) return NCDIS_P_P_P_PHOTON;
        if ((5 == nNonNeutrons) && (4 == nProtons) && (1 == nPhotons)) return NCDIS_P_P_P_P_PHOTON;
        if ((6 == nNonNeutrons) && (5 == nProtons) && (1 == nPhotons)) return NCDIS_P_P_P_P_P_PHOTON;

        if ((2 == nNonNeutrons) && (0 == nProtons) && (2 == nPhotons)) return NCDIS_PIZERO;
        if ((3 == nNonNeutrons) && (1 == nProtons) && (2 == nPhotons)) return NCDIS_P_PIZERO;
        if ((4 == nNonNeutrons) && (2 == nProtons) && (2 == nPhotons)) return NCDIS_P_P_PIZERO;
        if ((5 == nNonNeutrons) && (3 == nProtons) && (2 == nPhotons)) return NCDIS_P_P_P_PIZERO;
        if ((6 == nNonNeutrons) && (4 == nProtons) && (2 == nPhotons)) return NCDIS_P_P_P_P_PIZERO;
        if ((7 == nNonNeutrons) && (5 == nProtons) && (2 == nPhotons)) return NCDIS_P_P_P_P_P_PIZERO;
    }

    if (simpleMCEvent.m_mcNeutrinoNuance == 1096) return NCCOH;
    if (simpleMCEvent.m_mcNeutrinoNuance == 1097) return CCCOH;

    return OTHER_INTERACTION;
}

//------------------------------------------------------------------------------------------------------------------------------------------

ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList, const Parameters &parameters)
{
    // ATTN: Relies on fact that primary list is sorted by number of good true hits
    unsigned int nMuons(0), nElectrons(0), nProtons(0), nPiPlus(0), nPiMinus(0), nNeutrons(0), nPhotons(0);

    for (SimpleMCPrimaryList::const_iterator iter = simpleMCPrimaryList.begin(); iter != simpleMCPrimaryList.end(); ++iter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*iter);

        if (!IsGoodMCPrimary(simpleMCPrimary, parameters))
            continue;

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

float GetTrueAngle(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, const InteractionType interactionType, const ExpectedPrimary expectedPrimary)
{
    if (CCQEL_MU_P == interactionType)
    {
        const SimpleMCPrimary *pPrimary0(nullptr);
        const SimpleMCPrimary *pPrimary1(nullptr);

        for (SimpleMCPrimaryList::const_iterator pIter2 = simpleMCEvent.m_mcPrimaryList.begin(); pIter2 != simpleMCEvent.m_mcPrimaryList.end(); ++pIter2)
        {
            const SimpleMCPrimary &thisSimpleMCPrimary(*pIter2);
            const ExpectedPrimary thisExpectedPrimary(GetExpectedPrimary(thisSimpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, parameters));
            if (thisExpectedPrimary == MUON) pPrimary0 = &thisSimpleMCPrimary;
            if (thisExpectedPrimary == PROTON1) pPrimary1 = &thisSimpleMCPrimary;
        }

        if (pPrimary0 && pPrimary1)
        {
            const float pTot0(std::sqrt(pPrimary0->m_momentum.m_x * pPrimary0->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary0->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary0->m_momentum.m_z));
            const float pTot1(std::sqrt(pPrimary1->m_momentum.m_x * pPrimary1->m_momentum.m_x + pPrimary1->m_momentum.m_y * pPrimary1->m_momentum.m_y + pPrimary1->m_momentum.m_z * pPrimary1->m_momentum.m_z));
            return std::acos((pPrimary0->m_momentum.m_x * pPrimary1->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary1->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary1->m_momentum.m_z) / (pTot0 * pTot1));
        }
    }
    else if (CCRES_MU_P_PIZERO == interactionType)
    {
        const SimpleMCPrimary *pPrimary0(nullptr);
        const SimpleMCPrimary *pPrimary1(nullptr);

        for (SimpleMCPrimaryList::const_iterator pIter2 = simpleMCEvent.m_mcPrimaryList.begin(); pIter2 != simpleMCEvent.m_mcPrimaryList.end(); ++pIter2)
        {
            const SimpleMCPrimary &thisSimpleMCPrimary(*pIter2);
            const ExpectedPrimary thisExpectedPrimary(GetExpectedPrimary(thisSimpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, parameters));
            if (thisExpectedPrimary == PHOTON1) pPrimary0 = &thisSimpleMCPrimary;
            if (thisExpectedPrimary == PHOTON2) pPrimary1 = &thisSimpleMCPrimary;
        }

        if (pPrimary0 && pPrimary1)
        {
            const float pTot0(std::sqrt(pPrimary0->m_momentum.m_x * pPrimary0->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary0->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary0->m_momentum.m_z));
            const float pTot1(std::sqrt(pPrimary1->m_momentum.m_x * pPrimary1->m_momentum.m_x + pPrimary1->m_momentum.m_y * pPrimary1->m_momentum.m_y + pPrimary1->m_momentum.m_z * pPrimary1->m_momentum.m_z));
            return std::acos((pPrimary0->m_momentum.m_x * pPrimary1->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary1->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary1->m_momentum.m_z) / (pTot0 * pTot1));
        }
    }
    else if (CCRES_MU_P_PIPLUS == interactionType)
    {
        const SimpleMCPrimary *pPrimary0(nullptr);
        const SimpleMCPrimary *pPrimary1(nullptr);
        const SimpleMCPrimary *pPrimary2(nullptr);

        for (SimpleMCPrimaryList::const_iterator pIter2 = simpleMCEvent.m_mcPrimaryList.begin(); pIter2 != simpleMCEvent.m_mcPrimaryList.end(); ++pIter2)
        {
            const SimpleMCPrimary &thisSimpleMCPrimary(*pIter2);
            const ExpectedPrimary thisExpectedPrimary(GetExpectedPrimary(thisSimpleMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, parameters));
            if (thisExpectedPrimary == MUON) pPrimary0 = &thisSimpleMCPrimary;
            if (thisExpectedPrimary == PROTON1) pPrimary1 = &thisSimpleMCPrimary;
            if (thisExpectedPrimary == PIPLUS) pPrimary2 = &thisSimpleMCPrimary;
        }

        if (pPrimary0 && pPrimary1 && pPrimary2)
        {
            const float pTot0(std::sqrt(pPrimary0->m_momentum.m_x * pPrimary0->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary0->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary0->m_momentum.m_z));
            const float pTot1(std::sqrt(pPrimary1->m_momentum.m_x * pPrimary1->m_momentum.m_x + pPrimary1->m_momentum.m_y * pPrimary1->m_momentum.m_y + pPrimary1->m_momentum.m_z * pPrimary1->m_momentum.m_z));
            const float pTot2(std::sqrt(pPrimary2->m_momentum.m_x * pPrimary2->m_momentum.m_x + pPrimary2->m_momentum.m_y * pPrimary2->m_momentum.m_y + pPrimary2->m_momentum.m_z * pPrimary2->m_momentum.m_z));

            const float angle01(std::acos((pPrimary0->m_momentum.m_x * pPrimary1->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary1->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary1->m_momentum.m_z) / (pTot0 * pTot1)));
            const float angle02(std::acos((pPrimary0->m_momentum.m_x * pPrimary2->m_momentum.m_x + pPrimary0->m_momentum.m_y * pPrimary2->m_momentum.m_y + pPrimary0->m_momentum.m_z * pPrimary2->m_momentum.m_z) / (pTot0 * pTot2)));
            const float angle12(std::acos((pPrimary1->m_momentum.m_x * pPrimary2->m_momentum.m_x + pPrimary1->m_momentum.m_y * pPrimary2->m_momentum.m_y + pPrimary1->m_momentum.m_z * pPrimary2->m_momentum.m_z) / (pTot1 * pTot2)));

            float chosenAngle(-1.f);
            if (MUON == expectedPrimary) chosenAngle = std::min(angle01, angle02);
            else if (PROTON1 == expectedPrimary) chosenAngle = std::min(angle01, angle12);
            else if (PIPLUS == expectedPrimary) chosenAngle = std::min(angle02, angle12);

            return chosenAngle;
        }
    }

    return std::numeric_limits<float>::max();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void GetVtxShwDistance(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, const InteractionType interactionType,
    const PfoMatchingMap &pfoMatchingMap, EventResult &eventResult)
{
    const bool isElectronShower((interactionType == CCQEL_E) || (interactionType == CCQEL_E_P) || (interactionType == CCQEL_E_P_P) ||
        (interactionType == CCRES_E) || (interactionType == CCRES_E_P) || (interactionType == CCRES_E_P_P));
    const bool isPhotonShower((interactionType == CCRES_MU_PHOTON) || (interactionType == CCRES_MU_P_PHOTON) || (interactionType == CCRES_MU_P_P_PHOTON) ||
        (interactionType == NCRES_PHOTON) || (interactionType == NCRES_P_PHOTON) || (interactionType == NCRES_P_P_PHOTON));

    if (!isElectronShower && !isPhotonShower)
        return;

    SimpleMCPrimary showerMCPrimary;
    bool trueShowerVertexFound(false);
    SimpleThreeVector trueShowerVertex;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        showerMCPrimary = (*pIter);
        const ExpectedPrimary expectedPrimary(GetExpectedPrimary(showerMCPrimary.m_id, simpleMCEvent.m_mcPrimaryList, parameters));

        if (!IsGoodMCPrimary(showerMCPrimary, parameters) && (2112 != showerMCPrimary.m_pdgCode))
            continue;

        if ((isElectronShower && (ELECTRON == expectedPrimary)) || (isPhotonShower && (PHOTON1 == expectedPrimary)))
        {
            trueShowerVertex = (ELECTRON == expectedPrimary) ? showerMCPrimary.m_vertex : showerMCPrimary.m_endpoint;
            trueShowerVertexFound = true;
            break;
        }
    }

    float bestCompleteness(0.f);
    bool recoShowerVertexFound(false);
    SimpleThreeVector recoShowerVertex;

    for (SimpleMatchedPfoList::const_iterator mIter = showerMCPrimary.m_matchedPfoList.begin(); mIter != showerMCPrimary.m_matchedPfoList.end(); ++mIter)
    {
        const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

        if (pfoMatchingMap.count(simpleMatchedPfo.m_id) && (showerMCPrimary.m_id == pfoMatchingMap.at(simpleMatchedPfo.m_id).m_matchedPrimaryId))
        {
            if (!IsGoodMatch(showerMCPrimary, simpleMatchedPfo, parameters))
                continue;

            const float completeness((showerMCPrimary.m_nMCHitsTotal > 0) ? static_cast<float>(simpleMatchedPfo.m_nMatchedHitsTotal) / static_cast<float>(showerMCPrimary.m_nMCHitsTotal) : 0);

            if (completeness > bestCompleteness)
            {
                recoShowerVertex = simpleMatchedPfo.m_vertex;
                recoShowerVertexFound = true;
            }
        }
    }

    if (trueShowerVertexFound && recoShowerVertexFound)
    {
        const SimpleThreeVector trueVector(trueShowerVertex - simpleMCEvent.m_mcNeutrinoVtx);
        const SimpleThreeVector recoVector(recoShowerVertex - simpleMCEvent.m_recoNeutrinoVtx);
        eventResult.m_trueVtxShwDistance = std::sqrt(trueVector.m_x * trueVector.m_x + trueVector.m_y * trueVector.m_y + trueVector.m_z * trueVector.m_z);
        eventResult.m_recoVtxShwDistance = std::sqrt(recoVector.m_x * recoVector.m_x + recoVector.m_y * recoVector.m_y + recoVector.m_z * recoVector.m_z);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplayInteractionCountingMap(const InteractionCountingMap &interactionCountingMap, const Parameters &parameters)
{
    std::cout << "MinPrimaryGoodHits " << parameters.m_minPrimaryGoodHits << ", MinHitsForGoodView " << parameters.m_minHitsForGoodView << ", MinPrimaryGoodViews " << parameters.m_minPrimaryGoodViews << std::endl;
    std::cout << "UseSmallPrimaries " << parameters.m_useSmallPrimaries << ", MinSharedHits " << parameters.m_minSharedHits << ", MinCompleteness " << parameters.m_minCompleteness << ", MinPurity " << parameters.m_minPurity << std::endl;

    std::ofstream mapFile;
    if (!parameters.m_mapFileName.empty())
    {
        mapFile.open(parameters.m_mapFileName, ios::app);
        mapFile << "MinPrimaryGoodHits " << parameters.m_minPrimaryGoodHits << ", MinHitsForGoodView " << parameters.m_minHitsForGoodView << ", MinPrimaryGoodViews " << parameters.m_minPrimaryGoodViews << std::endl;
        mapFile << "UseSmallPrimaries " << parameters.m_useSmallPrimaries << ", MinSharedHits " << parameters.m_minSharedHits << ", MinCompleteness " << parameters.m_minCompleteness << ", MinPurity " << parameters.m_minPurity << std::endl;
    }

    std::cout << std::fixed;
    std::cout << std::setprecision(1);

    for (InteractionCountingMap::const_iterator iter = interactionCountingMap.begin(); iter != interactionCountingMap.end(); ++iter)
    {
        const InteractionType interactionType(iter->first);
        const CountingMap &countingMap(iter->second);
        std::cout << std::endl << ToString(interactionType) << std::endl;

        if (!parameters.m_mapFileName.empty())
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
                      << "%|, misID " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_misID) / static_cast<float>(countingDetails.m_nTotal-countingDetails.m_nMatch0) : 0.f) <<  "%" << std::endl;
					   

            if (!parameters.m_mapFileName.empty())
            {
                mapFile << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal
                        << ", nPfos |0: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch0) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |1: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch1) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |2: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch2) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
                        << "%|, |3+: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_nMatch3Plus) / static_cast<float>(countingDetails.m_nTotal) : 0.f)
			<< "%|, misID: " << ((countingDetails.m_nTotal > 0) ? 100.f * static_cast<float>(countingDetails.m_misID) / static_cast<float>(countingDetails.m_nTotal-countingDetails.m_nMatch0) : 0.f) <<  "%" << std::endl;
            }
        }
    }

    if (!parameters.m_mapFileName.empty())
        mapFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalyseInteractionEventResultMap(const InteractionEventResultMap &interactionEventResultMap, const Parameters &parameters)
{
    // Intended for filling histograms, post-processing of information collected in main loop over ntuple, etc.
    std::cout << std::endl << "EVENT INFO " << std::endl;

    std::ofstream mapFile, eventFile;
    if (!parameters.m_mapFileName.empty()) mapFile.open(parameters.m_mapFileName, ios::app);
    if (!parameters.m_eventFileName.empty()) eventFile.open(parameters.m_eventFileName, ios::app);

    InteractionPrimaryHistogramMap interactionPrimaryHistogramMap;
    InteractionEventHistogramMap interactionEventHistogramMap;

    for (InteractionEventResultMap::const_iterator iter = interactionEventResultMap.begin(), iterEnd = interactionEventResultMap.end(); iter != iterEnd; ++iter)
    {
        const InteractionType interactionType(iter->first);
        const EventResultList &eventResultList(iter->second);

	unsigned int nCorrectEvents(0);
        for (EventResultList::const_iterator eIter = eventResultList.begin(), eIterEnd = eventResultList.end(); eIter != eIterEnd; ++eIter)
        {

	  unsigned int  nTracksMisID(0), nShowersMisID(0), nTracks(0), nShowers(0);

            const PrimaryResultMap &primaryResultMap(eIter->m_primaryResultMap);
            bool isCorrect(!primaryResultMap.empty()), isTrackLike(true), isMisID(false);

            for (PrimaryResultMap::const_iterator pIter = primaryResultMap.begin(), pIterEnd = primaryResultMap.end(); pIter != pIterEnd; ++pIter)
            {
                const ExpectedPrimary expectedPrimary(pIter->first);
                const PrimaryResult &primaryResult(pIter->second);

                if (primaryResult.m_nPfoMatches != 1)
                    isCorrect = false;

		if((primaryResult.m_isTrackLike)&&(primaryResult.m_nPfoMatches != 0))
		  {
		    ++nTracks;
		    isTrackLike = true;
		  }
		if((!primaryResult.m_isTrackLike)&&(primaryResult.m_nPfoMatches != 0))
		  {
		    ++nShowers;
		    isTrackLike = false;
		  }

		if(primaryResult.m_isMisID)
		  {
		    isMisID = true;
		    if(primaryResult.m_isTrackLike)
		      ++nTracksMisID;
		    else
		      ++nShowersMisID;
		  }
                if (parameters.m_histogramOutput)
                {
                    const std::string histPrefix(parameters.m_histPrefix + ToString(interactionType) + "_" + ToString(expectedPrimary) + "_");
                    PrimaryHistogramCollection &histogramCollection(interactionPrimaryHistogramMap[interactionType][expectedPrimary]);
                    FillPrimaryHistogramCollection(histPrefix, primaryResult, histogramCollection);

                    const std::string histPrefixAll(parameters.m_histPrefix + ToString(ALL_INTERACTIONS) + "_" + ToString(expectedPrimary) + "_");
                    PrimaryHistogramCollection &histogramCollectionAll(interactionPrimaryHistogramMap[ALL_INTERACTIONS][expectedPrimary]);
                    FillPrimaryHistogramCollection(histPrefixAll, primaryResult, histogramCollectionAll);
                }
            }

            if (parameters.m_histogramOutput)
	      {
                const std::string histPrefix(parameters.m_histPrefix + ToString(interactionType) + "_");
                EventHistogramCollection &histogramCollection(interactionEventHistogramMap[interactionType]);
                FillEventHistogramCollection(histPrefix, isCorrect, *eIter, histogramCollection, nTracks, nShowers, nTracksMisID, nShowersMisID);
		
                const std::string histPrefixAll(parameters.m_histPrefix + ToString(ALL_INTERACTIONS) + "_");
                EventHistogramCollection &histogramCollectionAll(interactionEventHistogramMap[ALL_INTERACTIONS]);
                FillEventHistogramCollection(histPrefixAll, isCorrect, *eIter, histogramCollectionAll, nTracks, nShowers, nTracksMisID, nShowersMisID);
	      }
	    
	    if (isCorrect)
            {
                ++nCorrectEvents;

                if (!parameters.m_eventFileName.empty())
                    eventFile << "Correct event: fileId: " << eIter->m_fileIdentifier << ", eventNumber: " << eIter->m_eventNumber << ", nuance: " << eIter->m_mcNeutrinoNuance << std::endl;
            }
        }

        std::cout << ToString(interactionType) << std::endl << "-nEvents " << eventResultList.size() << ", nCorrect " << nCorrectEvents
                  << ", fCorrect " << 100.f * static_cast<float>(nCorrectEvents) / static_cast<float>(eventResultList.size()) << "%" << std::endl;

        if (!parameters.m_mapFileName.empty())
        {
            mapFile << ToString(interactionType) << std::endl << "-nEvents " << eventResultList.size() << ", nCorrect " << nCorrectEvents
                    << ", fCorrect " << 100.f * static_cast<float>(nCorrectEvents) / static_cast<float>(eventResultList.size()) << "%" << std::endl;
        }
    }

    if (parameters.m_histogramOutput)
        ProcessHistogramCollections(interactionPrimaryHistogramMap);

    if (parameters.m_histogramOutput)
      {
	for (InteractionEventResultMap::const_iterator iter = interactionEventResultMap.begin(), iterEnd = interactionEventResultMap.end(); iter != iterEnd; ++iter)
	  {
	    const InteractionType interactionType(iter->first);
	    EventHistogramCollection &histogramCollection(interactionEventHistogramMap[interactionType]);
	    ProcessHistogramCollections(histogramCollection);
	  }
      }


    if (!parameters.m_mapFileName.empty()) mapFile.close();
    if (!parameters.m_eventFileName.empty()) eventFile.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void FillEventHistogramCollection(const std::string &histPrefix, const bool isCorrect, const EventResult &eventResult, EventHistogramCollection &eventHistogramCollection, const int nTracks, const int nShowers, const int nTracksMisID, const int nShowersMisID)
{

  if(!eventHistogramCollection.m_hNEvents)                                                                                                            
    {                                                                                                                                                
      eventHistogramCollection.m_hNEvents = new TH1F((histPrefix + "NEvents").c_str(), "", 1, 0., 1.0);                                               
    }   
  if(!eventHistogramCollection.m_hNCorrectEvents)                                                                                                   
    {                                                                                                                                               
      eventHistogramCollection.m_hNCorrectEvents = new TH1F((histPrefix + "NCorrectEvents").c_str(), "", 1, 0., 1.0);                               
    }                                                                                                                                               
  if(!eventHistogramCollection.m_hNTracks)                                                                                                   
    {                                                                                                                                               
      eventHistogramCollection.m_hNTracks = new TH1F((histPrefix + "NTracks").c_str(), "", 1, 0., 1.0);                               
    }                                                                                                                                               
  if(!eventHistogramCollection.m_hNShowers)                                                                                                   
    {                                                                                                                                               
      eventHistogramCollection.m_hNShowers = new TH1F((histPrefix + "NShowers").c_str(), "", 1, 0., 1.0);                               
    }                                                                                                                                               
  if(!eventHistogramCollection.m_hNTracksMisID)                                                                                                   
    {                                                                                                                                               
      eventHistogramCollection.m_hNTracksMisID = new TH1F((histPrefix + "NTracksMisID").c_str(), "", 1, 0., 1.0);                               
    }                                                                                                                                               
  if(!eventHistogramCollection.m_hNShowersMisID)                                                                                                   
    {                                                                                                                                               
      eventHistogramCollection.m_hNShowersMisID = new TH1F((histPrefix + "NShowersMisID").c_str(), "", 1, 0., 1.0);                               
    }                                                                                                                                               


    if (!eventHistogramCollection.m_hVtxDeltaX)
    {
        eventHistogramCollection.m_hVtxDeltaX = new TH1F((histPrefix + "VtxDeltaX").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetRangeUser(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaX->GetXaxis()->SetTitle("Vertex #DeltaX [cm]");
        eventHistogramCollection.m_hVtxDeltaX->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hVtxDeltaY)
    {
        eventHistogramCollection.m_hVtxDeltaY = new TH1F((histPrefix + "VtxDeltaY").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetRangeUser(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaY->GetXaxis()->SetTitle("Vertex #DeltaY [cm]");
        eventHistogramCollection.m_hVtxDeltaY->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hVtxDeltaZ)
    {
        eventHistogramCollection.m_hVtxDeltaZ = new TH1F((histPrefix + "VtxDeltaZ").c_str(), "", 40000, -2000., 2000.);
        eventHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetRangeUser(-5., +5.);
        eventHistogramCollection.m_hVtxDeltaZ->GetXaxis()->SetTitle("Vertex #DeltaZ [cm]");
        eventHistogramCollection.m_hVtxDeltaZ->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hVtxDeltaR)
    {
        eventHistogramCollection.m_hVtxDeltaR = new TH1F((histPrefix + "VtxDeltaR").c_str(), "", 40000, -100., 1900.);
        eventHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetRangeUser(0., +5.);
        eventHistogramCollection.m_hVtxDeltaR->GetXaxis()->SetTitle("Vertex #DeltaR [cm]");
        eventHistogramCollection.m_hVtxDeltaR->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hNeutrinoPurity)
    {
        eventHistogramCollection.m_hNeutrinoPurity = new TH1F((histPrefix + "NeutrinoPurity").c_str(), "", 51, -0.01, 1.01);
        eventHistogramCollection.m_hNeutrinoPurity->GetXaxis()->SetRangeUser(0., +1.01);
        eventHistogramCollection.m_hNeutrinoPurity->GetXaxis()->SetTitle("Reco Neutrino Purity");
        eventHistogramCollection.m_hNeutrinoPurity->GetYaxis()->SetTitle("Number of Reconstructed Neutrinos");
    }

    if (!eventHistogramCollection.m_hNuPurityCorrect)
    {
        eventHistogramCollection.m_hNuPurityCorrect = new TH1F((histPrefix + "NeutrinoPurityCorrect").c_str(), "", 51, -0.01, 1.01);
        eventHistogramCollection.m_hNuPurityCorrect->GetXaxis()->SetRangeUser(0., +1.01);
        eventHistogramCollection.m_hNuPurityCorrect->GetXaxis()->SetTitle("Reco Neutrino Purity");
        eventHistogramCollection.m_hNuPurityCorrect->GetYaxis()->SetTitle("Number of Correctly Reconstructed Neutrinos");
    }

    if (!eventHistogramCollection.m_hCosmicFraction)
    {
        eventHistogramCollection.m_hCosmicFraction = new TH1F((histPrefix + "CosmicFraction").c_str(), "", 51, -0.01, 1.01);
        eventHistogramCollection.m_hCosmicFraction->GetXaxis()->SetRangeUser(0., +1.01);
        eventHistogramCollection.m_hCosmicFraction->GetXaxis()->SetTitle("Cosmic Contamination");
        eventHistogramCollection.m_hCosmicFraction->GetYaxis()->SetTitle("Number of Reconstructed Neutrinos");
    }

    if (!eventHistogramCollection.m_hNeutrinoCompleteness)
    {
        eventHistogramCollection.m_hNeutrinoCompleteness = new TH1F((histPrefix + "NeutrinoCompleteness").c_str(), "", 51, -0.01, 1.01);
        eventHistogramCollection.m_hNeutrinoCompleteness->GetXaxis()->SetRangeUser(0., +1.01);
        eventHistogramCollection.m_hNeutrinoCompleteness->GetXaxis()->SetTitle("Reconstructed Neutrino Completeness");
        eventHistogramCollection.m_hNeutrinoCompleteness->GetYaxis()->SetTitle("Number of Reconstructed Neutrinos");
    }

    if (!eventHistogramCollection.m_hNuCompletenessCorrect)
    {
        eventHistogramCollection.m_hNuCompletenessCorrect = new TH1F((histPrefix + "NeutrinoCompletenessCorrect").c_str(), "", 51, -0.01, 1.01);
        eventHistogramCollection.m_hNuCompletenessCorrect->GetXaxis()->SetRangeUser(0., +1.01);
        eventHistogramCollection.m_hNuCompletenessCorrect->GetXaxis()->SetTitle("Reconstructed Neutrino Completeness");
        eventHistogramCollection.m_hNuCompletenessCorrect->GetYaxis()->SetTitle("Number of Correctly Reconstructed Neutrinos");
    }

    if (!eventHistogramCollection.m_hNRecoNeutrinos)
    {
        eventHistogramCollection.m_hNRecoNeutrinos = new TH1F((histPrefix + "NRecoNeutrinos").c_str(), "", 11, -0.5, 10.5);
        eventHistogramCollection.m_hNRecoNeutrinos->GetXaxis()->SetTitle("Number of Reconstructed Neutrinos");
        eventHistogramCollection.m_hNRecoNeutrinos->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hTrueVtxShwDistance)
    {
        eventHistogramCollection.m_hTrueVtxShwDistance = new TH1F((histPrefix + "TrueVtxShwDistance").c_str(), "", 200, 0., 1000.);
        eventHistogramCollection.m_hTrueVtxShwDistance->GetXaxis()->SetRangeUser(0., +100.);
        eventHistogramCollection.m_hTrueVtxShwDistance->GetXaxis()->SetTitle("True Vertex to Shower Distance");
        eventHistogramCollection.m_hTrueVtxShwDistance->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hRecoVtxShwDistance)
    {
        eventHistogramCollection.m_hRecoVtxShwDistance = new TH1F((histPrefix + "RecoVtxShwDistance").c_str(), "", 200, 0., 1000.);
        eventHistogramCollection.m_hRecoVtxShwDistance->GetXaxis()->SetRangeUser(0., +100.);
        eventHistogramCollection.m_hRecoVtxShwDistance->GetXaxis()->SetTitle("Reco Vertex to Shower Distance");
        eventHistogramCollection.m_hRecoVtxShwDistance->GetYaxis()->SetTitle("Number of Events");
    }

    if (!eventHistogramCollection.m_hVtxShwResolution)
    {
        eventHistogramCollection.m_hVtxShwResolution = new TH1F((histPrefix + "VtxShwResolution").c_str(), "", 3200, -200., 200.);
        eventHistogramCollection.m_hVtxShwResolution->GetXaxis()->SetTitle("Vertex to Shower Distance Resolution");
        eventHistogramCollection.m_hVtxShwResolution->GetYaxis()->SetTitle("Number of Events");
    }

    eventHistogramCollection.m_hNEvents->Fill(1.0);//LORENA   
    eventHistogramCollection.m_hVtxDeltaX->Fill(eventResult.m_vertexOffset.m_x);
    eventHistogramCollection.m_hVtxDeltaY->Fill(eventResult.m_vertexOffset.m_y);
    eventHistogramCollection.m_hVtxDeltaZ->Fill(eventResult.m_vertexOffset.m_z);
    eventHistogramCollection.m_hVtxDeltaR->Fill(std::sqrt(eventResult.m_vertexOffset.m_x * eventResult.m_vertexOffset.m_x + eventResult.m_vertexOffset.m_y * eventResult.m_vertexOffset.m_y + eventResult.m_vertexOffset.m_z * eventResult.m_vertexOffset.m_z));
    eventHistogramCollection.m_hNRecoNeutrinos->Fill(eventResult.m_nRecoNeutrinos);

    eventHistogramCollection.m_hTrueVtxShwDistance->Fill(eventResult.m_trueVtxShwDistance);
    eventHistogramCollection.m_hRecoVtxShwDistance->Fill(eventResult.m_recoVtxShwDistance);
    eventHistogramCollection.m_hVtxShwResolution->Fill(eventResult.m_recoVtxShwDistance - eventResult.m_trueVtxShwDistance);

    eventHistogramCollection.m_hNeutrinoPurity->Fill(eventResult.m_neutrinoPurity);
    eventHistogramCollection.m_hCosmicFraction->Fill(1.f - eventResult.m_neutrinoPurity);
    eventHistogramCollection.m_hNeutrinoCompleteness->Fill(eventResult.m_neutrinoCompleteness);

    for(int i=1; i<=nTracks; i++)
      eventHistogramCollection.m_hNTracks->Fill(1.0);
    for(int i=1; i<=nShowers; i++)
      eventHistogramCollection.m_hNShowers->Fill(1.0);
    for(int i=1; i<=nTracksMisID; i++)
      eventHistogramCollection.m_hNTracksMisID->Fill(1.0);
    for(int i=1; i<=nShowersMisID; i++)
      eventHistogramCollection.m_hNShowersMisID->Fill(1.0);


    if (isCorrect)
    {
        eventHistogramCollection.m_hNCorrectEvents->Fill(1.0);
        eventHistogramCollection.m_hNuPurityCorrect->Fill(eventResult.m_neutrinoPurity);
        eventHistogramCollection.m_hNuCompletenessCorrect->Fill(eventResult.m_neutrinoCompleteness);
    }

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

    if (!primaryHistogramCollection.m_hHitsMisID)
    {
        primaryHistogramCollection.m_hHitsMisID = new TH1F((histPrefix + "HitsMisID").c_str(), "", nHitBins, hitsBinning);
        primaryHistogramCollection.m_hHitsMisID->GetXaxis()->SetRangeUser(1., +6000);
        primaryHistogramCollection.m_hHitsMisID->GetXaxis()->SetTitle("Number of Hits");
        primaryHistogramCollection.m_hHitsMisID->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hHitsMisID->GetYaxis()->SetTitle("Reconstruction MisID");
    }

    const int nMomentumBins(20); const int nMomentumBinEdges(nMomentumBins + 1);
    float momentumBinning[nMomentumBinEdges] = {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.4, 1.6, 2.0, 2.4, 2.8, 3.4, 4., 5.};

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

    if (!primaryHistogramCollection.m_hAngleAll)
    {
        primaryHistogramCollection.m_hAngleAll = new TH1F((histPrefix + "AngleAll").c_str(), "", 64, -M_PI, M_PI);
        primaryHistogramCollection.m_hAngleAll->GetXaxis()->SetRangeUser(0., +3.141);
        primaryHistogramCollection.m_hAngleAll->GetXaxis()->SetTitle("#theta_{z}");
        primaryHistogramCollection.m_hAngleAll->GetYaxis()->SetTitle("Number of Events");
    }

    if (!primaryHistogramCollection.m_hAngleEfficiency)
    {
        primaryHistogramCollection.m_hAngleEfficiency = new TH1F((histPrefix + "AngleEfficiency").c_str(), "", 64, -M_PI, M_PI);
        primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->SetRangeUser(0., +3.141);
        primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->SetTitle("#theta_{z}");
        primaryHistogramCollection.m_hAngleEfficiency->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hAngleEfficiency->GetYaxis()->SetTitle("Reconstruction Efficiency");
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

    if (!primaryHistogramCollection.m_hNPfosVsPTot)
    {
        primaryHistogramCollection.m_hNPfosVsPTot = new TH2F((histPrefix + "NMatchedPfosVsPTrue").c_str(), "", 21, -0.05, 2.05, 51, -0.5, 50.5);
        primaryHistogramCollection.m_hNPfosVsPTot->GetXaxis()->SetTitle("P_{True} [GeV]");
        primaryHistogramCollection.m_hNPfosVsPTot->GetXaxis()->SetRangeUser(0., +2.01);
        primaryHistogramCollection.m_hNPfosVsPTot->GetYaxis()->SetRangeUser(0., +10.);
        primaryHistogramCollection.m_hNPfosVsPTot->GetYaxis()->SetTitle("Number of Matched Particles");
    }

    if (!primaryHistogramCollection.m_hBestCompVsPTot)
    {
        primaryHistogramCollection.m_hBestCompVsPTot = new TH2F((histPrefix + "BestCompletenessVsPTrue").c_str(), "", 21, -0.05, 2.05, 26, -0.02, 1.02);
        primaryHistogramCollection.m_hBestCompVsPTot->GetXaxis()->SetTitle("P_{True} [GeV]");
        primaryHistogramCollection.m_hBestCompVsPTot->GetXaxis()->SetRangeUser(0., +2.01);
        primaryHistogramCollection.m_hBestCompVsPTot->GetYaxis()->SetRangeUser(0., +1.01);
        primaryHistogramCollection.m_hBestCompVsPTot->GetYaxis()->SetTitle("Best Completeness");
    }

    primaryHistogramCollection.m_hNPfosVsPTot->Fill(primaryResult.m_trueMomentum, primaryResult.m_nPfoMatches);
    primaryHistogramCollection.m_hBestCompVsPTot->Fill(primaryResult.m_trueMomentum, primaryResult.m_bestCompleteness);

    primaryHistogramCollection.m_hHitsAll->Fill(primaryResult.m_nTrueHits);
    primaryHistogramCollection.m_hMomentumAll->Fill(primaryResult.m_trueMomentum);
    primaryHistogramCollection.m_hAngleAll->Fill(primaryResult.m_trueAngle);

    if (primaryResult.m_nPfoMatches > 0)
    {
        primaryHistogramCollection.m_hHitsEfficiency->Fill(primaryResult.m_nTrueHits);
	if(primaryResult.m_isMisID)
	  primaryHistogramCollection.m_hHitsMisID->Fill(primaryResult.m_nTrueHits);
	  
        primaryHistogramCollection.m_hMomentumEfficiency->Fill(primaryResult.m_trueMomentum);
        primaryHistogramCollection.m_hAngleEfficiency->Fill(primaryResult.m_trueAngle);
        primaryHistogramCollection.m_hCompleteness->Fill(primaryResult.m_bestCompleteness);
        primaryHistogramCollection.m_hPurity->Fill(primaryResult.m_bestMatchPurity);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------


void ProcessHistogramCollections(const EventHistogramCollection &histogramCollection)
{
	histogramCollection.m_hNEvents->Write();
	histogramCollection.m_hNCorrectEvents->Write();
	histogramCollection.m_hNTracks->Write();
	histogramCollection.m_hNTracksMisID->Write();
	histogramCollection.m_hNShowers->Write();
	histogramCollection.m_hNShowersMisID->Write();
	histogramCollection.m_hVtxDeltaX->Write();
	histogramCollection.m_hVtxDeltaY->Write();
	histogramCollection.m_hVtxDeltaZ->Write();
	histogramCollection.m_hNeutrinoPurity->Write();
	histogramCollection.m_hCosmicFraction->Write();
	histogramCollection.m_hNeutrinoCompleteness->Write();
	histogramCollection.m_hNRecoNeutrinos->Write();

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

            for (int n = -1; n <= primaryHistogramCollection.m_hAngleEfficiency->GetXaxis()->GetNbins(); ++n)
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

	    primaryHistogramCollection.m_hHitsEfficiency->Write();
	    primaryHistogramCollection.m_hHitsMisID->Write();
	    primaryHistogramCollection.m_hMomentumEfficiency->Write();
	    primaryHistogramCollection.m_hAngleEfficiency->Write();
	    primaryHistogramCollection.m_hCompleteness->Write();
	    primaryHistogramCollection.m_hPurity->Write();	

        }

    }


}
