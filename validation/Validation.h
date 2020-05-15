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
    bool                    m_applySBNDFiducialCut;     ///< Whether to apply sbnd fiducial volume cut to true neutrino vertex position
    bool                    m_correctTrackShowerId;     ///< Whether to demand that pfos are correctly flagged as tracks or showers
    float                   m_vertexXCorrection;        ///< The vertex x correction, added to reported mc neutrino endpoint x value, in cm
    bool                    m_histogramOutput;          ///< Whether to produce output histograms
    bool                    m_testBeamMode;             ///< Whether running in test beam mode
    bool                    m_triggeredBeamOnly;        ///< Whether to only consider triggered beam particles
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

    int                 m_primaryId;                    ///< The identifier
    int                 m_pdgCode;                      ///< The pdg code
    float               m_energy;                       ///< The energy
    SimpleThreeVector   m_momentum;                     ///< The momentum
    SimpleThreeVector   m_vertex;                       ///< The vertex
    SimpleThreeVector   m_endpoint;                     ///< The endpoint
    int                 m_nMCHitsTotal;                 ///< The total number of mc hits
    int                 m_nMCHitsU;                     ///< The number of u mc hits
    int                 m_nMCHitsV;                     ///< The number of v mc hits
    int                 m_nMCHitsW;                     ///< The number of w mc hits

    int                 m_nPrimaryMatchedPfos;          ///< The number of matched pfos
    int                 m_nPrimaryMatchedNuPfos;        ///< The number of matched nu pfos
    int                 m_nPrimaryMatchedCRPfos;        ///< The number of matched cr pfos
    int                 m_bestMatchPfoId;               ///< The best match pfo identifier
    int                 m_bestMatchPfoPdgCode;          ///< The best match pfo pdg code
    int                 m_bestMatchPfoIsRecoNu;         ///< Whether best match pfo is reconstructed as part of a neutrino hierarchy
    int                 m_bestMatchPfoRecoNuId;         ///< The identifier of the associated reco neutrino (if part of a neutrino hierarchy)
    int                 m_bestMatchPfoIsTestBeam;       ///< Whether best match pfo is reconstructed as a test beam particle
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

    int                 m_interactionType;              ///< The target interaction type
    int                 m_mcNuanceCode;                 ///< The target nuance code
    int                 m_isNeutrino;                   ///< Whether the target is a neutrino
    int                 m_isBeamParticle;               ///< Whether the target is a beam particle
    int                 m_isCosmicRay;                  ///< Whether the target is a cosmic ray

    SimpleThreeVector   m_targetVertex;                 ///< The target vertex position
    SimpleThreeVector   m_recoVertex;                   ///< The reco vertex position, if available

    int                 m_isCorrectNu;                  ///< Whether the target was correctly reconstructed as a neutrino
    int                 m_isCorrectTB;                  ///< Whether the target was correctly reconstructed as a beam particle
    int                 m_isCorrectCR;                  ///< Whether the target was correctly reconstructed as a cosmic ray
    int                 m_isFakeNu;                     ///< Whether the target was reconstructed as a fake neutrino
    int                 m_isFakeCR;                     ///< Whether the target was reconstructed as a fake cosmic ray
    int                 m_isSplitNu;                    ///< Whether the target was reconstructed as a split neutrino
    int                 m_isSplitCR;                    ///< Whether the target was reconstructed as a split cosmic ray
    int                 m_isLost;                       ///< Whether the target was lost (not reconstructed)

    int                 m_nTargetMatches;               ///< The number of pfo matches to the target
    int                 m_nTargetNuMatches;             ///< The number of neutrino pfo matches to the target
    int                 m_nTargetCRMatches;             ///< The number of cosmic ray pfo matches to the target
    int                 m_nTargetGoodNuMatches;         ///< The number of good neutrino pfo matches to the target (all from same parent neutrino)
    int                 m_nTargetNuSplits;              ///< The number of split neutrino pfo matches to the target (from different parent neutrinos)
    int                 m_nTargetNuLosses;              ///< The number of neutrino primaries with no matches

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
    CCDIS_MU,
    CCDIS_MU_P,
    CCDIS_MU_P_P,
    CCDIS_MU_P_P_P,
    CCDIS_MU_P_P_P_P,
    CCDIS_MU_P_P_P_P_P,
    CCDIS_MU_PIPLUS,
    CCDIS_MU_P_PIPLUS,
    CCDIS_MU_P_P_PIPLUS,
    CCDIS_MU_P_P_P_PIPLUS,
    CCDIS_MU_P_P_P_P_PIPLUS,
    CCDIS_MU_P_P_P_P_P_PIPLUS,
    CCDIS_MU_PHOTON,
    CCDIS_MU_P_PHOTON,
    CCDIS_MU_P_P_PHOTON,
    CCDIS_MU_P_P_P_PHOTON,
    CCDIS_MU_P_P_P_P_PHOTON,
    CCDIS_MU_P_P_P_P_P_PHOTON,
    CCDIS_MU_PIZERO,
    CCDIS_MU_P_PIZERO,
    CCDIS_MU_P_P_PIZERO,
    CCDIS_MU_P_P_P_PIZERO,
    CCDIS_MU_P_P_P_P_PIZERO,
    CCDIS_MU_P_P_P_P_P_PIZERO,
    NCDIS_P,
    NCDIS_P_P,
    NCDIS_P_P_P,
    NCDIS_P_P_P_P,
    NCDIS_P_P_P_P_P,
    NCDIS_PIPLUS,
    NCDIS_P_PIPLUS,
    NCDIS_P_P_PIPLUS,
    NCDIS_P_P_P_PIPLUS,
    NCDIS_P_P_P_P_PIPLUS,
    NCDIS_P_P_P_P_P_PIPLUS,
    NCDIS_PIMINUS,
    NCDIS_P_PIMINUS,
    NCDIS_P_P_PIMINUS,
    NCDIS_P_P_P_PIMINUS,
    NCDIS_P_P_P_P_PIMINUS,
    NCDIS_P_P_P_P_P_PIMINUS,
    NCDIS_PHOTON,
    NCDIS_P_PHOTON,
    NCDIS_P_P_PHOTON,
    NCDIS_P_P_P_PHOTON,
    NCDIS_P_P_P_P_PHOTON,
    NCDIS_P_P_P_P_P_PHOTON,
    NCDIS_PIZERO,
    NCDIS_P_PIZERO,
    NCDIS_P_P_PIZERO,
    NCDIS_P_P_P_PIZERO,
    NCDIS_P_P_P_P_PIZERO,
    NCDIS_P_P_P_P_P_PIZERO,
    CCCOH,
    NCCOH,
    COSMIC_RAY_MU,
    COSMIC_RAY_P,
    COSMIC_RAY_E,
    COSMIC_RAY_PHOTON,
    COSMIC_RAY_OTHER,
    BEAM_PARTICLE_MU,
    BEAM_PARTICLE_P,
    BEAM_PARTICLE_E,
    BEAM_PARTICLE_PHOTON,
    BEAM_PARTICLE_PI_PLUS,
    BEAM_PARTICLE_PI_MINUS,
    BEAM_PARTICLE_KAON_PLUS,
    BEAM_PARTICLE_KAON_MINUS,
    BEAM_PARTICLE_OTHER,
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
    unsigned int            m_nMatch0;                  ///< The number of times the mc primary has 0 pfo matches
    unsigned int            m_nMatch1;                  ///< The number of times the mc primary has 1 pfo matches
    unsigned int            m_nMatch2;                  ///< The number of times the mc primary has 2 pfo matches
    unsigned int            m_nMatch3Plus;              ///< The number of times the mc primary has 3 or more pfo matches
    unsigned int            m_correctId;                ///< The number of times the mc primary particle id was correct
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
    unsigned int            m_nMCHitsTotal;             ///< The number of hits in the mc primary
    unsigned int            m_nBestMatchSharedHitsTotal;///< The number of hits shared by the mc primary and the best matched pfo
    unsigned int            m_nBestMatchRecoHitsTotal;  ///< The number of hits in the best matched pfo
    float                   m_bestMatchCompleteness;    ///< The completeness of the best matched pfo
    float                   m_bestMatchPurity;          ///< The purity of the best matched pfo
    bool                    m_isCorrectParticleId;      ///< Whether the best matched pfo has the correct particle id
    float                   m_trueMomentum;             ///< The true momentum of the mc primary
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

    int                     m_fileIdentifier;           ///< The file identifier
    int                     m_eventNumber;              ///< The event number
    bool                    m_isCorrect;                ///< Whether the target is reconstructed correctly
    bool                    m_hasRecoVertex;            ///< Whether a reco vertex is matched to the target
    SimpleThreeVector       m_vertexOffset;             ///< The offset between the reco and true target vertices
    PrimaryResultMap        m_primaryResultMap;         ///< The primary result map
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

    TH1F                   *m_hVtxDeltaX;               ///< The vtx delta x histogram
    TH1F                   *m_hVtxDeltaY;               ///< The vtx delta y histogram
    TH1F                   *m_hVtxDeltaZ;               ///< The vtx delta z histogram
    TH1F                   *m_hVtxDeltaR;               ///< The vtx delta r histogram
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

    TH1F                   *m_hHitsAll;                 ///< The number of primaries vs number of hits histogram
    TH1F                   *m_hHitsEfficiency;          ///< The primary efficiency vs number of hits histogram
    TH1F                   *m_hMomentumAll;             ///< The number of primaries vs momentum histogram
    TH1F                   *m_hMomentumEfficiency;      ///< The primary efficiency vs momentum histogram
    TH1F                   *m_hCompleteness;            ///< The primary (best match) completeness histogram
    TH1F                   *m_hPurity;                  ///< The primary (best match) purity histogram
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
 *  @param  parameters the parameters
 *
 *  @return the number of chain entries read
 */
int ReadNextEvent(TChain *const pTChain, const int iEntry, SimpleMCEvent &simpleMCEvent, const Parameters &parameters);

/**
 *  @brief  Print matching details to screen for a simple mc event
 *
 *  @param  simpleMCEvent the simple mc event
 *  @param  parameters the parameters
 */
void DisplaySimpleMCEventMatches(const SimpleMCEvent &simpleMCEvent, const Parameters &parameters);

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
 *  @brief  Whether a simple mc event passes the relevant fiducial cut, applied to target vertices
 *
 *  @param  simpleMCTarget the simple mc target
 *  @param  parameters the parameters
 *
 *  @return boolean
 */
bool PassFiducialCut(const SimpleMCTarget &simpleMCTarget, const Parameters &parameters);

/**
 *  @brief  Whether a simple mc event passes uboone fiducial cut, applied to target vertices
 *
 *  @param  simpleMCTarget the simple mc target
 *
 *  @return boolean
 */
bool PassUbooneFiducialCut(const SimpleMCTarget &simpleMCTarget);

/**
 *  @brief  Whether a simple mc event passes sbnd fiducial cut, applied to target vertices
 *
 *  @param  simpleMCTarget the simple mc target
 *
 *  @return boolean
 */
bool PassSBNDFiducialCut(const SimpleMCTarget &simpleMCTarget);

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

/**
 *  @brief  Opportunity to fill histograms, perform post-processing of information collected in main loop over ntuple, etc.
 *
 *  @param  interactionTargetResultMap the interaction target result map
 *  @param  parameters the parameters
 */
void AnalyseInteractionTargetResultMap(const InteractionTargetResultMap &interactionTargetResultMap, const Parameters &parameters);

/**
 *  @brief  Fill histograms in the provided target histogram collection, using information in the provided target result
 *
 *  @param  histPrefix the histogram prefix
 *  @param  targetResult the target result
 *  @param  targetHistogramCollection the target histogram collection
 */
void FillTargetHistogramCollection(const std::string &histPrefix, const TargetResult &targetResult, TargetHistogramCollection &targetHistogramCollection);

/**
 *  @brief  Fill histograms in the provided histogram collection, using information in the provided primary result
 *
 *  @param  histPrefix the histogram prefix
 *  @param  parameters the parameters
 *  @param  primaryResult the primary result
 *  @param  primaryHistogramCollection the primary histogram collection
 */
void FillPrimaryHistogramCollection(const std::string &histPrefix, const Parameters &parameters, const PrimaryResult &primaryResult, PrimaryHistogramCollection &primaryHistogramCollection);

/**
 *  @brief  Process histograms stored in the provided map e.g. calculating final efficiencies, normalising, etc.
 *
 *  @param  interactionPrimaryHistogramMap the interaction primary histogram map
 */
void ProcessHistogramCollections(const InteractionPrimaryHistogramMap &interactionPrimaryHistogramMap);

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

Parameters::Parameters() :
    m_displayMatchedEvents(true),
    m_skipEvents(0),
    m_nEventsToProcess(std::numeric_limits<int>::max()),
    m_applyUbooneFiducialCut(false),
    m_applySBNDFiducialCut(false),
    m_correctTrackShowerId(false),
    m_vertexXCorrection(0.495694f),
    m_histogramOutput(false),
    m_testBeamMode(false),
    m_triggeredBeamOnly(true)
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
    m_primaryId(-1),
    m_pdgCode(0),
    m_energy(0.f),
    m_momentum(0.f, 0.f, 0.f),
    m_vertex(-1.f, -1.f, -1.f),
    m_endpoint(-1.f, -1.f, -1.f),
    m_nMCHitsTotal(0),
    m_nMCHitsU(0),
    m_nMCHitsV(0),
    m_nMCHitsW(0),
    m_nPrimaryMatchedPfos(0),
    m_nPrimaryMatchedNuPfos(0),
    m_nPrimaryMatchedCRPfos(0),
    m_bestMatchPfoId(-1),
    m_bestMatchPfoPdgCode(0),
    m_bestMatchPfoIsRecoNu(0),
    m_bestMatchPfoRecoNuId(-1),
    m_bestMatchPfoIsTestBeam(0),
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
    m_targetVertex(0.f, 0.f, 0.f),
    m_recoVertex(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    m_isCorrectNu(false),
    m_isCorrectTB(false),
    m_isCorrectCR(false),
    m_isFakeNu(false),
    m_isFakeCR(false),
    m_isSplitNu(false),
    m_isSplitCR(false),
    m_isLost(false),
    m_nTargetMatches(0),
    m_nTargetNuMatches(0),
    m_nTargetCRMatches(0),
    m_nTargetGoodNuMatches(0),
    m_nTargetNuSplits(0),
    m_nTargetNuLosses(0),
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
    m_trueMomentum(-1.f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

TargetResult::TargetResult() :
    m_fileIdentifier(-1),
    m_eventNumber(-1),
    m_isCorrect(false),
    m_hasRecoVertex(false),
    m_vertexOffset(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
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

TargetHistogramCollection::TargetHistogramCollection() :
    m_hVtxDeltaX(nullptr),
    m_hVtxDeltaY(nullptr),
    m_hVtxDeltaZ(nullptr),
    m_hVtxDeltaR(nullptr)
{
}

#endif // #ifndef NEW_LAR_VALIDATION_H
