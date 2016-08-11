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
    ELECTRON,
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
    OTHER_INTERACTION,
    ALL_INTERACTIONS // ATTN use carefully!
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
    unsigned int        m_nTrueHits;            ///< The number of true hits
    float               m_trueMomentum;         ///< The true momentum
    float               m_trueAngle;            ///< The true angle wrt the z axis
    unsigned int        m_nBestMatchedHits;     ///< The best number of matched hits
    unsigned int        m_nBestRecoHits;        ///< The number of hits in the best matched pfo
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
    int                 m_fileIdentifier;       ///< The file identifier
    int                 m_eventNumber;          ///< The event number
    int                 m_mcNeutrinoNuance;     ///< The mc neutrino nuance code (interaction type details)
    int                 m_nRecoNeutrinos;       ///< The number of reconstructed neutrinos
    PrimaryResultMap    m_primaryResultMap;     ///< The primary result map
    SimpleThreeVector   m_vertexOffset;         ///< The vertex offset
};

typedef std::vector<EventResult> EventResultList; // ATTN Not terribly efficient, but that's not the main aim here
typedef std::map<InteractionType, EventResultList> InteractionEventResultMap;

//------------------------------------------------------------------------------------------------------------------------------------------

class TH1F;

/**
 *  @brief  EventHistogramCollection class
 */
class EventHistogramCollection
{
public:
    /**
     *  @brief  Default constructor
     */
    EventHistogramCollection();

    TH1F *m_hVtxDeltaX;         ///< 
    TH1F *m_hVtxDeltaY;         ///< 
    TH1F *m_hVtxDeltaZ;         ///< 
    TH1F *m_hVtxDeltaR;         ///< 
    TH1F *m_nRecoNeutrinos;     ///<
};

typedef std::map<InteractionType, EventHistogramCollection> InteractionEventHistogramMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  PrimaryHistogramCollection class
 */
class PrimaryHistogramCollection
{
public:
    /**
     *  @brief  Default constructor
     */
    PrimaryHistogramCollection();

    TH1F *m_hHitsAll;           ///<
    TH1F *m_hHitsEfficiency;    ///<
    TH1F *m_hMomentumAll;       ///<
    TH1F *m_hMomentumEfficiency;///<
    TH1F *m_hAngleAll;          ///<
    TH1F *m_hAngleEfficiency;   ///<
    TH1F *m_hCompleteness;      ///<
    TH1F *m_hPurity;            ///<
};

typedef std::map<ExpectedPrimary, PrimaryHistogramCollection> PrimaryHistogramMap;
typedef std::map<InteractionType, PrimaryHistogramMap> InteractionPrimaryHistogramMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Validation - Main entry point for analysis
 * 
 *  @param  inputFiles the regex identifying the input root files
 *  @param  shouldDisplayEvents whether to display the reconstruction outcomes for individual events
 *  @param  shouldDisplayMatchedEvents whether to display matching results for individual events
 *  @param  skipEvents the number of events to skip
 *  @param  nEventsToProcess the number of events to process
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  histogramOutput whether to produce output histograms
 *  @param  correctId whether to demand that pfos are correctly flagged as tracks or showers
 *  @param  applyFiducialCut whether to apply fiducial volume cut to true neutrino vertex position
 *  @param  histPrefix histogram name prefix
 *  @param  outputFileName file name to which to write output ascii tables, etc.
 *  @param  correctEventFileName file name to which to write list of correct events
 */
void Validation(const std::string &inputFiles, const bool shouldDisplayEvents = true, const bool shouldDisplayMatchedEvents = true,
    const int skipEvents = 0, const int nEventsToProcess = std::numeric_limits<int>::max(), const int primaryMinHits = 0, const int minMatchedHits = 0,
    const bool histogramOutput = false, const bool correctId = false, const bool applyFiducialCut = false, const std::string histPrefix = "",
    const std::string outputFileName = "", const std::string correctEventFileName = "");

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
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  pfoMatchingMap the pfo matching map, to be populated
 */
void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Get the strongest pfo match (most matched hits) between an available mc primary and an available pfo
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  usedMCIds the list of mc primary ids with an existing match
 *  @param  usedPfoIds the list of pfo ids with an existing match
 *  @param  pfoMatchingMap the pfo matching map, to be populated
 */
bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, IntSet &usedMCIds,
    IntSet &usedPfoIds, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Get the best matches for any pfos left-over after the strong matching procedure
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  usedPfoIds the list of pfo ids with an existing match
 *  @param  pfoMatchingMap the pfo matching map, to be populated
 */
void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  CountPfoMatches Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  interactionType the interaction type
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  correctId whether to demand that pfos are correctly flagged as tracks or showers
 *  @param  applyFiducialCut whether to apply fiducial volume cut to true neutrino vertex position
 *  @param  interactionCountingMap the interaction counting map, to be populated
 *  @param  interactionEventOutcomeMap the interaction event outcome map, to be populated
 */
void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const int primaryMinHits, const bool correctId, const bool applyFiducialCut, InteractionCountingMap &interactionCountingMap,
    InteractionEventResultMap &interactionEventResultMap);

/**
 *  @brief  Whether a simple mc event passes a fiducial cut, applied to the mc neutrino vertex
 * 
 *  @param  simpleMCEvent the simple mc event
 * 
 *  @return boolean
 */
bool PassFiducialCut(const SimpleMCEvent &simpleMCEvent);

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
 *  @brief  Print matching details to screen for a simple mc event
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  pfoMatchingMap the pfo matching map
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 */
void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const PfoMatchingMap &pfoMatchingMap, const int primaryMinHits);

/**
 *  @brief  Print details to screen for a provided interaction type to counting map
 * 
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 *  @param  interactionCountingMap the interaction counting map
 *  @param  file name to which to write output ascii tables, etc.
 */
void DisplayInteractionCountingMap(const int primaryMinHits, const int minMatchedHits, const InteractionCountingMap &interactionCountingMap,
    const std::string &outputFileName);

/**
 *  @brief  Opportunity to fill histograms, perform post-processing of information collected in main loop over ntuple, etc.
 * 
 *  @param  interactionEventResultMap the interaction event result map
 *  @param  outputFileName file name to which to write output ascii tables, etc.
 *  @param  correctEventFileName file name to which to write list of correct events
 *  @param  histogramOutput whether to produce output histograms
 *  @param  prefix histogram name prefix
 */
void AnalyseInteractionEventResultMap(const InteractionEventResultMap &interactionEventResultMap, const std::string &outputFileName,
    const std::string &correctEventFileName, const bool histogramOutput, const std::string &prefix);

/**
 *  @brief  Fill histograms in the provided event histogram collection, using information in the provided event offset
 * 
 *  @param  histPrefix the histogram name prefix
 *  @param  eventResult the event result
 *  @param  eventHistogramCollection the event histogram collection
 */
void FillEventHistogramCollection(const std::string &histPrefix, const EventResult &eventResult, EventHistogramCollection &eventHistogramCollection);

/**
 *  @brief  Fill histograms in the provided histogram collection, using information in the provided primary result
 * 
 *  @param  histPrefix the histogram name prefix
 *  @param  primaryResult the primary result
 *  @param  primaryHistogramCollection the primary histogram collection
 */
void FillPrimaryHistogramCollection(const std::string &histPrefix, const PrimaryResult &primaryResult, PrimaryHistogramCollection &primaryHistogramCollection);

/**
 *  @brief  Process histograms stored in the provided map e.g. calculating final efficiencies, normalising, etc.
 * 
 *  @param  interactionPrimaryHistogramMap the interaction primary histogram map
 */
void ProcessHistogramCollections(const InteractionPrimaryHistogramMap &interactionPrimaryHistogramMap);

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
    m_fileIdentifier(-1),
    m_eventNumber(-1),
    m_mcNeutrinoNuance(-1),
    m_nRecoNeutrinos(0),
    m_vertexOffset(-999.f, -999.f, -999.f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

PrimaryHistogramCollection::PrimaryHistogramCollection() :
    m_hHitsAll(NULL),
    m_hHitsEfficiency(NULL),
    m_hMomentumAll(NULL),
    m_hMomentumEfficiency(NULL),
    m_hAngleAll(NULL),
    m_hAngleEfficiency(NULL),
    m_hCompleteness(NULL),
    m_hPurity(NULL)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

EventHistogramCollection::EventHistogramCollection() :
    m_hVtxDeltaX(NULL),
    m_hVtxDeltaY(NULL),
    m_hVtxDeltaZ(NULL),
    m_hVtxDeltaR(NULL),
    m_nRecoNeutrinos(NULL)
{
}

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
    case ALL_INTERACTIONS: return "ALL_INTERACTIONS";
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
    case ELECTRON: return "ELECTRON";
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
