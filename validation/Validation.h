/**
 *  @file   LArReco/validation/Validation.h
 *
 *  @brief  Header file for validation functionality
 *
 *  $Log: $
 */
#ifndef LAR_VALIDATION_H
#define LAR_VALIDATION_H 1

#include "ValidationIO.h"

/**
 * @brief   MatchingDetails class
 */
class MatchingDetails
{
public:
    /**
     *  @brief  Default constructor  
     */
    MatchingDetails();

    int                     m_matchedPrimaryId;         ///< The total number of occurences
    int                     m_nMatchedHits;             ///< The number of times the primary has 0 pfo matches
    float                   m_completeness;             ///< The completeness of the match
};

typedef std::map<int, MatchingDetails> PfoMatchingMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   CountingDetails class
 */
class CountingDetails
{
public:
    /**
     *  @brief  Default constructor  
     */
    CountingDetails();

    unsigned int        m_nTotal;               ///< The total number of occurences
    unsigned int        m_nMatch0;              ///< The number of times the primary has 0 pfo matches
    unsigned int        m_nMatch1;              ///< The number of times the primary has 1 pfo matches
    unsigned int        m_nMatch2;              ///< The number of times the primary has 2 pfo matches
    unsigned int        m_nMatch3Plus;          ///< The number of times the primary has 3 or more pfo matches
};

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   ExpectedPrimary enum
 */
enum ExpectedPrimary
{
    MUON,
    PROTON1,
    PROTON2,
    PIPLUS,
    PIMINUS,
    NEUTRON,
    PHOTON1,
    PHOTON2,
    OTHER_PRIMARY
};

typedef std::map<ExpectedPrimary, CountingDetails> CountingMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief   InteractionType enum
 */
enum InteractionType
{
    CCQEL_MU,
    CCQEL_MU_P,
    CCQEL_MU_P_P,
    NCQEL_P,
    NCQEL_P_P,
    CCRES_MU_PIPLUS,
    CCRES_MU_P_PIPLUS,
    CCRES_MU_P_P_PIPLUS,
    CCRES_MU_PIZERO,
    CCRES_MU_P_PIZERO,
    CCRES_MU_P_P_PIZERO,
    NCRES_P_PIPLUS,
    NCRES_P_P_PIPLUS,
    NCRES_P_PIZERO,
    NCRES_P_P_PIZERO,
    OTHER_INTERACTION
};

typedef std::map<InteractionType, CountingMap> InteractionCountingMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   PrimaryResult class
 */
class PrimaryResult
{
public:
    /**
     *  @brief  Default constructor  
     */
    PrimaryResult();

    unsigned int        m_nPfoMatches;          ///< The total number of pfo matches for a given primary
    float               m_bestCompleteness;     ///< The best match pfo is determined by the best completeness (most matched hits)
    float               m_bestMatchPurity;      ///< The purity of the best matched pfo
};

typedef std::map<ExpectedPrimary, PrimaryResult> PrimaryResultMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   EventResult class
 */
class EventResult
{
public:
    /**
     *  @brief  Default constructor  
     */
    EventResult();

    // ATTN Put items to count on a per-event basis here
float            m_openingAngle;
    PrimaryResultMap    m_primaryResultMap;     ///< The primary result map
};

typedef std::vector<EventResult> EventResultList; // ATTN Not terribly efficient, but that's not the main aim here
typedef std::map<InteractionType, EventResultList> InteractionEventResultMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Validation - Main entry point for analysis
 * 
 *  @param  inputFiles the regex identifying the input root files
 *  @param  shouldDisplay whether to display the reconstruction outcomes for individual events
 *  @param  maxEvents the maximum number of events to process
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 */
void Validation(const std::string &inputFiles, const bool shouldDisplay = true, const int skipEvents = 0,
    const int nEventsToProcess = std::numeric_limits<int>::max(), const int primaryMinHits = 0, const int minMatchedHits = 0);

/**
 *  @brief  Get the event interaction type
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 * 
 *  @return the interaction type
 */
InteractionType GetInteractionType(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits);

/**
 *  @brief  Finalise the mc primary to pfo matching, using a pfo matching map to store the results
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  pfoMatchingMap the pfo matching map, to be populated
 */
void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const int minMatchedHits, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Get the strongest pfo match (most matched hits) between an available mc primary and an available pfo
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  usedMCIds the list of mc primary ids with an existing match
 *  @param  usedPfoIds the list of pfo ids with an existing match
 *  @param  pfoMatchingMap the pfo matching map, to be populated
 */
bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const int minMatchedHits, IntSet &usedMCIds,
    IntSet &usedPfoIds, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Get the best matches for any pfos left-over after the strong matching procedure
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  usedPfoIds the list of pfo ids with an existing match
 *  @param  pfoMatchingMap the pfo matching map, to be populated
 */
void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const int minMatchedHits, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  CountPfoMatches Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  interactionType the interaction type
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  interactionCountingMap the interaction counting map, to be populated
 *  @param  interactionEventOutcomeMap the interaction event outcome map, to be populated
 */
void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const int primaryMinHits, InteractionCountingMap &interactionCountingMap, InteractionEventResultMap &interactionEventResultMap);

/**
 *  @brief  Work out which of the primary particles (expected for a given interaction types) corresponds to the provided priamry id
 *          ATTN: Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  primaryId the primary id
 *  @param  simpleMCPrimaryList the simple mc primary list
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 * 
 *  @return the expected primary
 */
ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList, const int primaryMinHits);

/**
 *  @brief  Print details to screen for a provided interaction type to counting map
 * 
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  interactionCountingMap the interaction counting map
 */
void DisplayInteractionCountingMap(const int primaryMinHits, const int minMatchedHits, const InteractionCountingMap &interactionCountingMap);

/**
 *  @brief  Opportunity to fill histograms, perform post-processing of information collected in main loop over ntuple, etc.
 * 
 *  @param  interactionEventResultMap the interaction event result map
 */
void AnalyseInteractionEventResultMap(const InteractionEventResultMap &interactionEventResultMap);

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

MatchingDetails::MatchingDetails() :
    m_matchedPrimaryId(-1),
    m_nMatchedHits(0),
    m_completeness(0.f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

CountingDetails::CountingDetails() :
    m_nTotal(0),
    m_nMatch0(0),
    m_nMatch1(0),
    m_nMatch2(0),
    m_nMatch3Plus(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

PrimaryResult::PrimaryResult() :
    m_nPfoMatches(0),
    m_bestCompleteness(0.f),
    m_bestMatchPurity(0.f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

EventResult::EventResult() :
    m_openingAngle(-0.5f)
{
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Get string representing interaction type
 * 
 *  @param  interactionType
 * 
 *  @return the interaction type string
 */
std::string ToString(const InteractionType interactionType)
{
    switch (interactionType)
    {
    case CCQEL_MU: return "CCQEL_MU";
    case CCQEL_MU_P: return "CCQEL_MU_P";
    case CCQEL_MU_P_P: return "CCQEL_MU_P_P";
    case NCQEL_P: return "NCQEL_P";
    case NCQEL_P_P: return "NCQEL_P_P";
    case CCRES_MU_PIPLUS: return "CCRES_MU_PIPLUS";
    case CCRES_MU_P_PIPLUS: return "CCRES_MU_P_PIPLUS";
    case CCRES_MU_P_P_PIPLUS: return "CCRES_MU_P_P_PIPLUS";
    case CCRES_MU_PIZERO: return "CCRES_MU_PIZERO";
    case CCRES_MU_P_PIZERO: return "CCRES_MU_P_PIZERO";
    case CCRES_MU_P_P_PIZERO: return "CCRES_MU_P_P_PIZERO";
    case NCRES_P_PIPLUS: return "NCRES_P_PIPLUS";
    case NCRES_P_P_PIPLUS: return "NCRES_P_P_PIPLUS";
    case NCRES_P_PIZERO: return "NCRES_P_PIZERO";
    case NCRES_P_P_PIZERO: return "NCRES_P_P_PIZERO";
    case OTHER_INTERACTION: return "OTHER_INTERACTION";
    default: return "UNKNOWN";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Get string representing expected primary
 * 
 *  @param  expectedPrimary
 * 
 *  @return the expected primary string
 */
std::string ToString(const ExpectedPrimary expectedPrimary)
{
    switch (expectedPrimary)
    {
    case MUON: return "MUON";
    case PROTON1: return "PROTON1";
    case PROTON2: return "PROTON2";
    case PIPLUS: return "PIPLUS";
    case PIMINUS: return "PIMINUS";
    case NEUTRON: return "NEUTRON";
    case PHOTON1: return "PHOTON1";
    case PHOTON2: return "PHOTON2";
    case OTHER_PRIMARY: return "OTHER_PRIMARY";
    default: return "UNKNOWN";
    }
}

#endif // #ifndef LAR_VALIDATION_H
