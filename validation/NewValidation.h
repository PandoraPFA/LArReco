/**
 *  @file   LArReco/validation/NewValidation.h
 *
 *  @brief  Header file for validation functionality
 *
 *  $Log: $
 */
#ifndef NEW_LAR_VALIDATION_H
#define NEW_LAR_VALIDATION_H 1

#include <limits>

typedef std::vector<int> IntVector;
typedef std::vector<float> FloatVector;

/**
 * @brief   Parameters class
 */
class Parameters
{
public:
    /**
     *  @brief  Default constructor
     */
    Parameters();

    bool                    m_displayMatchedEvents;     ///< Whether to display matching results for individual events
    int                     m_skipEvents;               ///< The number of events to skip
    int                     m_nEventsToProcess;         ///< The number of events to process
    bool                    m_applyUbooneFiducialCut;   ///< Whether to apply uboone fiducial volume cut to true neutrino vertex position
    bool                    m_correctTrackShowerId;     ///< Whether to demand that pfos are correctly flagged as tracks or showers
    float                   m_vertexXCorrection;        ///< The vertex x correction, added to reported mc neutrino endpoint x value, in cm
    bool                    m_histogramOutput;          ///< Whether to produce output histograms
    std::string             m_histPrefix;               ///< Histogram name prefix
    std::string             m_mapFileName;              ///< File name to which to write output ascii tables, etc.
    std::string             m_eventFileName;            ///< File name to which to write list of correct events
};

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  SimpleThreeVector class
 */
class SimpleThreeVector
{
public:
    /**
     *  @brief  Default constructor
     */
    SimpleThreeVector();

    /**
     *  @brief  Constructor
     * 
     *  @param  x the x value
     *  @param  y the y value
     *  @param  z the z value
     */
    SimpleThreeVector(const float x, const float y, const float z);

    float               m_x;                            ///< The x value
    float               m_y;                            ///< The y value
    float               m_z;                            ///< The z value
};

typedef std::vector<SimpleThreeVector> SimpleThreeVectorList;

/**
 *  @brief  Simple three vector subtraction operator
 * 
 *  @param  lhs first vector, from which the second is subtracted
 *  @param  rhs second vector, which is subtracted from the first
 */
SimpleThreeVector operator-(const SimpleThreeVector &lhs, const SimpleThreeVector &rhs);

/**
 *  @brief  Simple three vector addition operator
 * 
 *  @param  lhs first vector, from which the second is added
 *  @param  rhs second vector, which is added to the first
 */
SimpleThreeVector operator+(const SimpleThreeVector &lhs, const SimpleThreeVector &rhs);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief SimpleMCPrimary class
 */
class SimpleMCPrimary
{
public:
    /**
     *  @brief  Constructor
     */
    SimpleMCPrimary();

    int                 m_pdgCode;                      ///< The pdg code
    int                 m_nMCHitsTotal;                 ///< The total number of mc hits
    int                 m_nMCHitsU;                     ///< The number of u mc hits
    int                 m_nMCHitsV;                     ///< The number of v mc hits
    int                 m_nMCHitsW;                     ///< The number of w mc hits
    float               m_energy;                       ///< The energy
    SimpleThreeVector   m_momentum;                     ///< The momentum
    SimpleThreeVector   m_vertex;                       ///< The vertex
    SimpleThreeVector   m_endpoint;                     ///< The endpoint

    int                 m_nMatchedPfos;                 ///< The number of matched pfos
    int                 m_bestMatchPfoPdgCode;          ///< The best match pfo pdg code
    int                 m_bestMatchPfoIsRecoNu;         ///< Whether best match pfo is reconstructed as part of a neutrino hierarchy
    int                 m_bestMatchPfoNHitsTotal;       ///< The best match pfo total number of pfo hits
    int                 m_bestMatchPfoNHitsU;           ///< The best match pfo number of u pfo hits
    int                 m_bestMatchPfoNHitsV;           ///< The best match pfo number of v pfo hits
    int                 m_bestMatchPfoNHitsW;           ///< The best match pfo number of w pfo hits
    int                 m_bestMatchPfoNSharedHitsTotal; ///< The best match pfo total number of matched hits
    int                 m_bestMatchPfoNSharedHitsU;     ///< The best match pfo number of u matched hits
    int                 m_bestMatchPfoNSharedHitsV;     ///< The best match pfo number of v matched hits
    int                 m_bestMatchPfoNSharedHitsW;     ///< The best match pfo number of w matched hits
};

typedef std::vector<SimpleMCPrimary> SimpleMCPrimaryList;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief SimpleMCTarget class
 */
class SimpleMCTarget
{
public:
    /**
     *  @brief  Constructor
     */
    SimpleMCTarget();

    int                 m_interactionType;              ///< 
    int                 m_mcNuanceCode;                 ///< 
    int                 m_isNeutrino;                   ///< 
    int                 m_isBeamParticle;               ///< 
    int                 m_isCosmicRay;                  ///< 

    int                 m_isCorrectNu;                  ///< 
    int                 m_isCorrectTB;                  ///< 
    int                 m_isCorrectCR;                  ///< 
    int                 m_isFakeNu;                     ///< 
    int                 m_isSplitCR;                    ///< 
    int                 m_isLostCR;                     ///< 
    int                 m_isFakeCR;                     ///< 
    int                 m_nNuMatches;                   ///< 
    int                 m_nNuSplits;                    ///< 
    int                 m_nCRMatches;                   ///< 

    int                 m_nTargetPrimaries;             ///< The number of target mc primaries
    SimpleMCPrimaryList m_mcPrimaryList;                ///< The list of mc primaries
};

typedef std::vector<SimpleMCTarget> SimpleMCTargetList;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief SimpleMCEvent class
 */
class SimpleMCEvent
{
public:
    /**
     *  @brief  Constructor
     */
    SimpleMCEvent();

    int                 m_fileIdentifier;               ///< The file identifier
    int                 m_eventNumber;                  ///< The event number

    int                 m_nMCTargets;                   ///< The number of mc targets
    SimpleMCTargetList  m_mcTargetList;                 ///< The list of mc targets
};

typedef std::vector<SimpleMCEvent> SimpleMCEventList;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   ExpectedPrimary enum
 */
enum ExpectedPrimary : int
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

/**
 *  @brief  Get a string representation of an interaction type
 *
 *  @param  interactionType the interaction type
 *
 *  @return string
 */
std::string ToString(const ExpectedPrimary expectedPrimary);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief   InteractionType enum
 */
enum InteractionType : int
{
    CCQEL_MU,
    CCQEL_MU_P,
    CCQEL_MU_P_P,
    CCQEL_MU_P_P_P,
    CCQEL_MU_P_P_P_P,
    CCQEL_MU_P_P_P_P_P,
    CCQEL_E,
    CCQEL_E_P,
    CCQEL_E_P_P,
    CCQEL_E_P_P_P,
    CCQEL_E_P_P_P_P,
    CCQEL_E_P_P_P_P_P,
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
    CCRES_E,
    CCRES_E_P,
    CCRES_E_P_P,
    CCRES_E_P_P_P,
    CCRES_E_P_P_P_P,
    CCRES_E_P_P_P_P_P,
    CCRES_E_PIPLUS,
    CCRES_E_P_PIPLUS,
    CCRES_E_P_P_PIPLUS,
    CCRES_E_P_P_P_PIPLUS,
    CCRES_E_P_P_P_P_PIPLUS,
    CCRES_E_P_P_P_P_P_PIPLUS,
    CCRES_E_PHOTON,
    CCRES_E_P_PHOTON,
    CCRES_E_P_P_PHOTON,
    CCRES_E_P_P_P_PHOTON,
    CCRES_E_P_P_P_P_PHOTON,
    CCRES_E_P_P_P_P_P_PHOTON,
    CCRES_E_PIZERO,
    CCRES_E_P_PIZERO,
    CCRES_E_P_P_PIZERO,
    CCRES_E_P_P_P_PIZERO,
    CCRES_E_P_P_P_P_PIZERO,
    CCRES_E_P_P_P_P_P_PIZERO,
    NCRES_P,
    NCRES_P_P,
    NCRES_P_P_P,
    NCRES_P_P_P_P,
    NCRES_P_P_P_P_P,
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
    COSMIC_RAY,
    BEAM_PARTICLE,
    OTHER_INTERACTION,
    ALL_INTERACTIONS
};

/**
 *  @brief  Get a string representation of an interaction type
 *
 *  @param  interactionType the interaction type
 *
 *  @return string
 */
std::string ToString(const InteractionType interactionType);

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

    unsigned int            m_nTotal;                   ///< The total number of occurences
    unsigned int            m_nMatch0;                  ///< The number of times the primary has 0 pfo matches
    unsigned int            m_nMatch1;                  ///< The number of times the primary has 1 pfo matches
    unsigned int            m_nMatch2;                  ///< The number of times the primary has 2 pfo matches
    unsigned int            m_nMatch3Plus;              ///< The number of times the primary has 3 or more pfo matches
    unsigned int            m_correctId;                ///< The number of times the primary particle identifcation was correct
};

typedef std::map<ExpectedPrimary, CountingDetails> CountingMap;
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

    unsigned int            m_nPfoMatches;              ///< The total number of pfo matches for a given primary
    unsigned int            m_nMCHitsTotal;             ///< The number of mc hits
    unsigned int            m_nBestMatchSharedHitsTotal;///< 
    unsigned int            m_nBestMatchRecoHitsTotal;  ///< 
    float                   m_bestMatchCompleteness;    ///< The best match pfo is determined by the best completeness (most matched hits)
    float                   m_bestMatchPurity;          ///< The purity of the best matched pfo
    bool                    m_isCorrectParticleId;      ///< 
    float                   m_trueMomentum;             ///< The true momentum
};

typedef std::map<ExpectedPrimary, PrimaryResult> PrimaryResultMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief   TargetResult class
 */
class TargetResult
{
public:
    /**
     *  @brief  Default constructor
     */
    TargetResult();

    bool                    m_isCorrect;                ///< 
    PrimaryResultMap        m_primaryResultMap;         ///< The primary result map
    // TODO add vertex resolution again
};

typedef std::vector<TargetResult> TargetResultList; // ATTN Not terribly efficient, but that's not the main aim here
typedef std::map<InteractionType, TargetResultList> InteractionTargetResultMap;

//------------------------------------------------------------------------------------------------------------------------------------------

class TH1F;

/**
 *  @brief  TargetHistogramCollection class
 */
class TargetHistogramCollection
{
public:
    /**
     *  @brief  Default constructor
     */
    TargetHistogramCollection();

    TH1F                   *m_hVtxDeltaX;               ///< 
    TH1F                   *m_hVtxDeltaY;               ///< 
    TH1F                   *m_hVtxDeltaZ;               ///< 
    TH1F                   *m_hVtxDeltaR;               ///< 
};

typedef std::map<InteractionType, TargetHistogramCollection> InteractionTargetHistogramMap;

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

    TH1F                   *m_hHitsAll;                 ///<
    TH1F                   *m_hHitsEfficiency;          ///<
    TH1F                   *m_hMomentumAll;             ///<
    TH1F                   *m_hMomentumEfficiency;      ///<
    TH1F                   *m_hCompleteness;            ///<
    TH1F                   *m_hPurity;                  ///<
};

typedef std::map<ExpectedPrimary, PrimaryHistogramCollection> PrimaryHistogramMap;
typedef std::map<InteractionType, PrimaryHistogramMap> InteractionPrimaryHistogramMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Validation - Main entry point for analysis
 * 
 *  @param  inputFiles the regex identifying the input root files
 *  @param  parameters the parameters
 */
void Validation(const std::string &inputFiles, const Parameters &parameters = Parameters());

/**
 *  @brief  Read the next event from the chain
 * 
 *  @param  pTChain the address of the chain
 *  @param  iEntry the first chain entry to read
 *  @param  simpleMCEvent the event to be populated
 *
 *  @return the number of chain entries read
 */
int ReadNextEvent(TChain *const pTChain, const int iEntry, SimpleMCEvent &simpleMCEvent);

/**
 *  @brief  Print matching details to screen for a simple mc event
 * 
 *  @param  simpleMCEvent the simple mc event
 */
void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent);

/**
 *  @brief  CountPfoMatches Relies on fact that primary list is sorted by number of true good hits
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  parameters the parameters
 *  @param  interactionCountingMap the interaction counting map, to be populated
 *  @param  interactionTargetResultMap the interaction target outcome map, to be populated
 */
void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters, InteractionCountingMap &interactionCountingMap,
    InteractionTargetResultMap &interactionTargetResultMap);

/**
 *  @brief  Whether a simple mc event passes uboone fiducial cut, applied to target vertices
 * 
 *  @param  simpleMCTarget the simple mc target
 * 
 *  @return boolean
 */
bool PassUbooneFiducialCut(const SimpleMCTarget &simpleMCTarget);

/**
 *  @brief  Work out which of the primary particles (expected for a given interaction types) corresponds to the provided primary id
 *          ATTN: Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  simpleMCPrimary the simple mc primary
 *  @param  simpleMCPrimaryList the simple mc primary list
 * 
 *  @return the expected primary
 */
ExpectedPrimary GetExpectedPrimary(const SimpleMCPrimary &simpleMCPrimary, const SimpleMCPrimaryList &simpleMCPrimaryList);

/**
 *  @brief  Whether a provided mc primary and best matched pfo are deemed to have a good particle id match
 * 
 *  @param  simpleMCPrimary the simple mc primary
 *  @param  bestMatchPfoPdgCode the best matched pfo pdg code
 * 
 *  @return boolean
 */
bool IsGoodParticleIdMatch(const SimpleMCPrimary &simpleMCPrimary, const int bestMatchPfoPdgCode);

/**
 *  @brief  Print details to screen for a provided interaction type to counting map
 * 
 *  @param  interactionCountingMap the interaction counting map
 *  @param  parameters the parameters
 */
void DisplayInteractionCountingMap(const InteractionCountingMap &interactionCountingMap, const Parameters &parameters);

///**
// *  @brief  Opportunity to fill histograms, perform post-processing of information collected in main loop over ntuple, etc.
// * 
// *  @param  interactionTargetResultMap the interaction target result map
// *  @param  parameters the parameters
// */
//void AnalyseInteractionTargetResultMap(const InteractionTargetResultMap &interactionTargetResultMap, const Parameters &parameters);
//
///**
// *  @brief  Fill histograms in the provided histogram collection, using information in the provided primary result
// * 
// *  @param  histPrefix the histogram prefix
// *  @param  primaryResult the primary result
// *  @param  primaryHistogramCollection the primary histogram collection
// */
//void FillPrimaryHistogramCollection(const std::string &histPrefix, const PrimaryResult &primaryResult, PrimaryHistogramCollection &primaryHistogramCollection);
//
///**
// *  @brief  Fill histograms in the provided target histogram collection, using information in the provided target result
// * 
// *  @param  histPrefix the histogram prefix
// *  @param  isCorrect whether the event is deemed correct
// *  @param  targetResult the target result
// *  @param  targetHistogramCollection the target histogram collection
// */
//void FillTargetHistogramCollection(const std::string &histPrefix, const bool isCorrect, const TargetResult &targetResult, TargetHistogramCollection &targetHistogramCollection);
//
///**
// *  @brief  Process histograms stored in the provided map e.g. calculating final efficiencies, normalising, etc.
// * 
// *  @param  interactionPrimaryHistogramMap the interaction primary histogram map
// */
//void ProcessHistogramCollections(const InteractionPrimaryHistogramMap &interactionPrimaryHistogramMap);

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

Parameters::Parameters() :
    m_displayMatchedEvents(true),
    m_skipEvents(0),
    m_nEventsToProcess(std::numeric_limits<int>::max()),
    m_applyUbooneFiducialCut(false),
    m_correctTrackShowerId(false),
    m_vertexXCorrection(0.495694f),
    m_histogramOutput(false)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleThreeVector::SimpleThreeVector() :
    m_x(-std::numeric_limits<float>::max()),
    m_y(-std::numeric_limits<float>::max()),
    m_z(-std::numeric_limits<float>::max())
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

SimpleThreeVector::SimpleThreeVector(const float x, const float y, const float z) :
    m_x(x),
    m_y(y),
    m_z(z)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

SimpleThreeVector operator-(const SimpleThreeVector &lhs, const SimpleThreeVector &rhs)
{
    return SimpleThreeVector(lhs.m_x - rhs.m_x, lhs.m_y - rhs.m_y, lhs.m_z - rhs.m_z);
}

//------------------------------------------------------------------------------------------------------------------------------------------

SimpleThreeVector operator+(const SimpleThreeVector &lhs, const SimpleThreeVector &rhs)
{
    return SimpleThreeVector(lhs.m_x + rhs.m_x, lhs.m_y + rhs.m_y, lhs.m_z + rhs.m_z);
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleMCPrimary::SimpleMCPrimary() :
    m_pdgCode(0),
    m_nMCHitsTotal(0),
    m_nMCHitsU(0),
    m_nMCHitsV(0),
    m_nMCHitsW(0),
    m_energy(0),
    m_momentum(0.f, 0.f, 0.f),
    m_vertex(-1.f, -1.f, -1.f),
    m_endpoint(-1.f, -1.f, -1.f),
    m_nMatchedPfos(0),
    m_bestMatchPfoPdgCode(0),
    m_bestMatchPfoIsRecoNu(0),
    m_bestMatchPfoNHitsTotal(0),
    m_bestMatchPfoNHitsU(0),
    m_bestMatchPfoNHitsV(0),
    m_bestMatchPfoNHitsW(0),
    m_bestMatchPfoNSharedHitsTotal(0),
    m_bestMatchPfoNSharedHitsU(0),
    m_bestMatchPfoNSharedHitsV(0),
    m_bestMatchPfoNSharedHitsW(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleMCTarget::SimpleMCTarget() :
    m_interactionType(OTHER_INTERACTION),
    m_mcNuanceCode(0),
    m_isNeutrino(false),
    m_isBeamParticle(false),
    m_isCosmicRay(false),
    m_isCorrectNu(false),
    m_isCorrectTB(false),
    m_isCorrectCR(false),
    m_isFakeNu(false),
    m_isSplitCR(false),
    m_isLostCR(false),
    m_isFakeCR(false),
    m_nNuMatches(0),
    m_nNuSplits(0),
    m_nCRMatches(0),
    m_nTargetPrimaries(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleMCEvent::SimpleMCEvent() :
    m_fileIdentifier(-1),
    m_eventNumber(0),
    m_nMCTargets(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

CountingDetails::CountingDetails() :
    m_nTotal(0),
    m_nMatch0(0),
    m_nMatch1(0),
    m_nMatch2(0),
    m_nMatch3Plus(0),
    m_correctId(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

PrimaryResult::PrimaryResult() :
    m_nPfoMatches(0),
    m_nMCHitsTotal(0),
    m_nBestMatchSharedHitsTotal(0),
    m_nBestMatchRecoHitsTotal(0),
    m_bestMatchCompleteness(0.f),
    m_bestMatchPurity(0.f),
    m_isCorrectParticleId(false),
    m_trueMomentum(std::numeric_limits<float>::max())
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

TargetResult::TargetResult() :
    m_isCorrect(false)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

PrimaryHistogramCollection::PrimaryHistogramCollection() :
    m_hHitsAll(nullptr),
    m_hHitsEfficiency(nullptr),
    m_hMomentumAll(nullptr),
    m_hMomentumEfficiency(nullptr),
    m_hCompleteness(nullptr),
    m_hPurity(nullptr)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

//EventHistogramCollection::EventHistogramCollection() :
//    m_hVtxDeltaX(nullptr),
//    m_hVtxDeltaY(nullptr),
//    m_hVtxDeltaZ(nullptr),
//    m_hVtxDeltaR(nullptr)
//{
//}

#endif // #ifndef NEW_LAR_VALIDATION_H
