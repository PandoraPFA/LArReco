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
 * @brief   MatchingParameters class
 */
class MatchingParameters
{
public:
    /**
     *  @brief  Constructor
     * 
     *  @param  minPrimaryGoodHits the minimum number of good mc primary hits
     *  @param  minHitsForGoodView minimum number of good mc primary hits in given view to declare view to be good
     *  @param  minPrimaryGoodViews minimum number of good views for a mc primary
     *  @param  useSmallPrimaries whether to consider matches to mc primaries with fewer than minPrimaryHits
     *  @param  minSharedHits the minimum number of shared hit
     *  @param  minCompleteness the minimum particle completeness
     *  @param  minPurity the minimum particle purity
     *  @param  applyFiducialCut whether to apply fiducial volume cut to true neutrino vertex position
     *  @param  correctTrackShowerId whether to demand that pfos are correctly flagged as tracks or showers
     */
    MatchingParameters(const int minPrimaryGoodHits, const int minHitsForGoodView, const int minPrimaryGoodViews, const bool useSmallPrimaries,
        const int minSharedHits, const float minCompleteness, const float minPurity, const bool applyFiducialCut, const bool correctTrackShowerId);

    int                     m_minPrimaryGoodHits;       ///< The minimum number of good mc primary hits
    bool                    m_useSmallPrimaries;        ///< Whether to consider matches to mc primaries with fewer than m_minPrimaryGoodHits
    int                     m_minHitsForGoodView;       ///< The minimum number of good mc primary hits in given view to declare view to be good
    int                     m_minPrimaryGoodViews;      ///< The minimum number of good views for a mc primary
    int                     m_minSharedHits;            ///< The minimum number of shared hits used in matching scheme
    float                   m_minCompleteness;          ///< The minimum particle completeness to declare a match
    float                   m_minPurity;                ///< The minimum particle purity to declare a match
    bool                    m_applyFiducialCut;         ///< Whether to apply fiducial volume cut to true neutrino vertex position
    bool                    m_correctTrackShowerId;     ///< Whether to demand that pfos are correctly flagged as tracks or showers
};

//------------------------------------------------------------------------------------------------------------------------------------------


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
    PROTON3,
    PROTON4,
    PROTON5,
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
    // TODO Move to dynamic interaction type identification and labelling
    CCQEL_MU,
    CCQEL_MU_P,
    CCQEL_MU_P_P,
    CCQEL_MU_P_P_P,
    CCQEL_MU_P_P_P_P,
    CCQEL_MU_P_P_P_P_P,
    NCQEL_P,
    NCQEL_P_P,
    NCQEL_P_P_P,
    NCQEL_P_P_P_P,
    NCQEL_P_P_P_P_P,
    CCRES_MU,
    CCRES_MU_P,
    CCRES_MU_P_P,
    CCRES_MU_P_P_P,
    CCRES_MU_P_P_P_P,
    CCRES_MU_P_P_P_P_P,
    CCRES_MU_PIPLUS,
    CCRES_MU_P_PIPLUS,
    CCRES_MU_P_P_PIPLUS,
    CCRES_MU_P_P_P_PIPLUS,
    CCRES_MU_P_P_P_P_PIPLUS,
    CCRES_MU_P_P_P_P_P_PIPLUS,
    CCRES_MU_PHOTON,
    CCRES_MU_P_PHOTON,
    CCRES_MU_P_P_PHOTON,
    CCRES_MU_P_P_P_PHOTON,
    CCRES_MU_P_P_P_P_PHOTON,
    CCRES_MU_P_P_P_P_P_PHOTON,
    CCRES_MU_PIZERO,
    CCRES_MU_P_PIZERO,
    CCRES_MU_P_P_PIZERO,
    CCRES_MU_P_P_P_PIZERO,
    CCRES_MU_P_P_P_P_PIZERO,
    CCRES_MU_P_P_P_P_P_PIZERO,
    NCRES_PIPLUS,
    NCRES_P_PIPLUS,
    NCRES_P_P_PIPLUS,
    NCRES_P_P_P_PIPLUS,
    NCRES_P_P_P_P_PIPLUS,
    NCRES_P_P_P_P_P_PIPLUS,
    NCRES_PIMINUS,
    NCRES_P_PIMINUS,
    NCRES_P_P_PIMINUS,
    NCRES_P_P_P_PIMINUS,
    NCRES_P_P_P_P_PIMINUS,
    NCRES_P_P_P_P_P_PIMINUS,
    NCRES_PHOTON,
    NCRES_P_PHOTON,
    NCRES_P_P_PHOTON,
    NCRES_P_P_P_PHOTON,
    NCRES_P_P_P_P_PHOTON,
    NCRES_P_P_P_P_P_PHOTON,
    NCRES_PIZERO,
    NCRES_P_PIZERO,
    NCRES_P_P_PIZERO,
    NCRES_P_P_P_PIZERO,
    NCRES_P_P_P_P_PIZERO,
    NCRES_P_P_P_P_P_PIZERO,
    CCDIS,
    NCDIS,
    CCCOH,
    NCCOH,
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

    FloatVector         m_allCompletenesses;    ///< 
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
class TH2F;

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

    TH2F *m_hNPfosVsPTot;       ///<
    TH2F *m_hBestCompVsPTot;    ///<
    TH2F *m_hAllCompVsPTot;     ///<
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
 *  @param  minPrimaryGoodHits the minimum number of good mc primary hits
 *  @param  minHitsForGoodView minimum number of good mc primary hits in given view to declare view to be good
 *  @param  minPrimaryGoodViews minimum number of good views for a mc primary
 *  @param  minCompleteness minimum particle completeness to declare a match
 *  @param  minPurity minimum particle purity to declare a match
 *  @param  useSmallPrimaries whether to consider matches to mc primaries with fewer than minPrimaryHits
 *  @param  minSharedHits the min number of shared hits in order to consider a matched pfo
 *  @param  applyFiducialCut whether to apply fiducial volume cut to true neutrino vertex position
 *  @param  correctTrackShowerId whether to demand that pfos are correctly flagged as tracks or showers
 *  @param  histogramOutput whether to produce output histograms
 *  @param  histPrefix histogram name prefix
 *  @param  mapFileName file name to which to write output ascii tables, etc.
 *  @param  eventFileName file name to which to write list of correct events
 */
void Validation(const std::string inputFiles,
    const bool shouldDisplayEvents = true,
    const bool shouldDisplayMatchedEvents = true,
    const int skipEvents = 0,
    const int nEventsToProcess = std::numeric_limits<int>::max(),
    const int minPrimaryGoodHits = 15,
    const int minHitsForGoodView = 5,
    const int minPrimaryGoodViews = 2,
    const bool useSmallPrimaries = true,
    const int minSharedHits = 5,
    const float minCompleteness = 0.1f,
    const float minPurity = 0.5f,
    const bool applyFiducialCut = true,
    const bool correctTrackShowerId = false,
    const bool histogramOutput = false,
    const std::string histPrefix = "",
    const std::string mapFileName = "",
    const std::string eventFileName = "");

/**
 *  @brief  Get the event interaction type
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  matchingParameters the matching parameters
 * 
 *  @return the interaction type
 */
InteractionType GetInteractionType(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters);

/**
 *  @brief  Finalise the mc primary to pfo matching, using a pfo matching map to store the results
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  matchingParameters the matching parameters
 *  @param  pfoMatchingMap to receive the populated pfo matching map
 */
void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Get the strongest pfo match (most matched hits) between an available mc primary and an available pfo
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  matchingParameters the matching parameters
 *  @param  usedMCIds the list of mc primary ids with an existing match
 *  @param  usedPfoIds the list of pfo ids with an existing match
 *  @param  pfoMatchingMap to receive the populated pfo matching map
 */
bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters, IntSet &usedMCIds, IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Get the best matches for any pfos left-over after the strong matching procedure
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  matchingParameters the matching parameters
 *  @param  usedPfoIds the list of pfo ids with an existing match
 *  @param  pfoMatchingMap to receive the populated pfo matching map
 */
void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const MatchingParameters &matchingParameters, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  Whether a provided mc primary passes selection, based on number of "good" hits
 * 
 *  @param  simpleMCPrimary the simple mc primary
 *  @param  matchingParameters the matching parameters
 * 
 *  @return boolean
 */
bool IsGoodMCPrimary(const SimpleMCPrimary &simpleMCPrimary, const MatchingParameters &matchingParameters);

/**
 *  @brief  Whether a provided mc primary has a match, of any quality (use simple matched pfo list and information in matching details map)
 * 
 *  @param  simpleMCPrimary the simple mc primary
 *  @param  pfoMatchingMap the pfo matching map
 *  @param  matchingParameters the matching parameters
 * 
 *  @return boolean
 */
bool HasMatch(const SimpleMCPrimary &simpleMCPrimary, const PfoMatchingMap &pfoMatchingMap, const MatchingParameters &matchingParameters);

/**
 *  @brief  Whether a provided mc primary and pfo are deemed to be a good match
 * 
 *  @param  simpleMCPrimary the simple mc primary
 *  @param  simpleMatchedPfo the simple matched pfo
 *  @param  matchingParameters the matching parameters
 * 
 *  @return boolean
 */
bool IsGoodMatch(const SimpleMCPrimary &simpleMCPrimary, const SimpleMatchedPfo &simpleMatchedPfo, const MatchingParameters &matchingParameters);

/**
 *  @brief  Work out which of the primary particles (expected for a given interaction types) corresponds to the provided priamry id
 *          ATTN: Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  primaryId the primary id
 *  @param  simpleMCPrimaryList the simple mc primary list
 * 
 *  @return the expected primary
 */
ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList);

/**
 *  @brief  Whether a simple mc event passes a fiducial cut, applied to the mc neutrino vertex
 * 
 *  @param  simpleMCEvent the simple mc event
 * 
 *  @return boolean
 */
bool PassFiducialCut(const SimpleMCEvent &simpleMCEvent);

/**
 *  @brief  CountPfoMatches Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  interactionType the interaction type
 *  @param  pfoMatchingMap the pfo matching map
 *  @param  matchingParameters, the matching parameters
 *  @param  interactionCountingMap the interaction counting map, to be populated
 *  @param  interactionEventOutcomeMap the interaction event outcome map, to be populated
 */
void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const MatchingParameters &matchingParameters, InteractionCountingMap &interactionCountingMap, InteractionEventResultMap &interactionEventResultMap);

/**
 *  @brief  Print matching details to screen for a simple mc event
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  pfoMatchingMap the pfo matching map
 *  @param  matchingParameters the matching parameters
 */
void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const PfoMatchingMap &pfoMatchingMap, const MatchingParameters &matchingParameters);

/**
 *  @brief  Print details to screen for a provided interaction type to counting map
 * 
 *  @param  interactionCountingMap the interaction counting map
 *  @param  matchingParameters the matching parameters
 *  @param  mapFileName file name to which to write output ascii tables, etc.
 */
void DisplayInteractionCountingMap(const InteractionCountingMap &interactionCountingMap, const MatchingParameters &matchingParameters,
    const std::string &mapFileName);

/**
 *  @brief  Opportunity to fill histograms, perform post-processing of information collected in main loop over ntuple, etc.
 * 
 *  @param  interactionEventResultMap the interaction event result map
 *  @param  mapFileName file name to which to write output ascii tables, etc.
 *  @param  eventFileName file name to which to write list of correct events
 *  @param  histogramOutput whether to produce output histograms
 *  @param  prefix histogram name prefix
 */
void AnalyseInteractionEventResultMap(const InteractionEventResultMap &interactionEventResultMap, const std::string &mapFileName,
    const std::string &eventFileName, const bool histogramOutput, const std::string &prefix);

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

MatchingParameters::MatchingParameters(const int minPrimaryGoodHits, const int minHitsForGoodView, const int minPrimaryGoodViews, const bool useSmallPrimaries,
        const int minSharedHits, const float minCompleteness, const float minPurity, const bool applyFiducialCut, const bool correctTrackShowerId) :
    m_minPrimaryGoodHits(minPrimaryGoodHits),
    m_minHitsForGoodView(minHitsForGoodView),
    m_minPrimaryGoodViews(minPrimaryGoodViews),
    m_useSmallPrimaries(useSmallPrimaries),
    m_minSharedHits(minSharedHits),
    m_minCompleteness(minCompleteness),
    m_minPurity(minPurity),
    m_applyFiducialCut(applyFiducialCut),
    m_correctTrackShowerId(correctTrackShowerId)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

MatchingDetails::MatchingDetails() :
    m_matchedPrimaryId(-1),
    m_nMatchedHits(0)
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
    m_hPurity(NULL),
    m_hNPfosVsPTot(NULL),
    m_hBestCompVsPTot(NULL),
    m_hAllCompVsPTot(NULL)
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
    case CCQEL_MU_P_P_P: return "CCQEL_MU_P_P_P";
    case CCQEL_MU_P_P_P_P: return "CCQEL_MU_P_P_P_P";
    case CCQEL_MU_P_P_P_P_P: return "CCQEL_MU_P_P_P_P_P";
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
    case CCDIS: return "CCDIS";
    case NCDIS: return "NCDIS";
    case CCCOH: return "CCCOH";
    case NCCOH: return "NCCOH";
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
    case PROTON3: return "PROTON3";
    case PROTON4: return "PROTON4";
    case PROTON5: return "PROTON5";
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
