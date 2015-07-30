#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

typedef std::vector<int> IntVector;
typedef std::vector<float> FloatVector;

typedef std::set<int> IntSet;

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

    float                   m_x;                        ///< The x value
    float                   m_y;                        ///< The y value
    float                   m_z;                        ///< The z value
};

typedef std::vector<SimpleThreeVector> SimpleThreeVectorList;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief SimpleMatchedPfo class
 */
class SimpleMatchedPfo
{
public:
    /**
     *  @brief  Constructor
     */
    SimpleMatchedPfo();

    int                     m_id;                       ///< The unique identifier
    int                     m_pdgCode;                  ///< The pdg code
    int                     m_nPfoHitsTotal;            ///< The total number of pfo hits
    int                     m_nPfoHitsU;                ///< The number of u pfo hits
    int                     m_nPfoHitsV;                ///< The number of v pfo hits
    int                     m_nPfoHitsW;                ///< The number of w pfo hits
    int                     m_nMatchedHitsTotal;        ///< The total number of matched hits
    int                     m_nMatchedHitsU;            ///< The number of u matched hits
    int                     m_nMatchedHitsV;            ///< The number of v matched hits
    int                     m_nMatchedHitsW;            ///< The number of w matched hits
    SimpleThreeVector       m_vertex;                   ///< The vertex (currently only filled for track pfos)
    SimpleThreeVector       m_endpoint;                 ///< The endpoint (currently only filled for track pfos)
    SimpleThreeVector       m_vertexDirection;          ///< The vertex direction (currently only filled for track pfos)
    SimpleThreeVector       m_endDirection;             ///< The endpoint direction (currently only filled for track pfos)
};

typedef std::vector<SimpleMatchedPfo> SimpleMatchedPfoList;

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

    int                     m_id;                       ///< The unique identifier
    int                     m_pdgCode;                  ///< The pdg code
    int                     m_nMCHitsTotal;             ///< The total number of mc hits
    int                     m_nMCHitsU;                 ///< The number of u mc hits
    int                     m_nMCHitsV;                 ///< The number of v mc hits
    int                     m_nMCHitsW;                 ///< The number of w mc hits
    float                   m_energy;                   ///< The energy
    SimpleThreeVector       m_momentum;                 ///< The momentum (presumably at the vertex)
    SimpleThreeVector       m_vertex;                   ///< The vertex
    SimpleThreeVector       m_endpoint;                 ///< The endpoint
    int                     m_nMatchedPfos;             ///< The number of matched pfos
    SimpleMatchedPfoList    m_matchedPfoList;           ///< The list of matched pfos
};

typedef std::vector<SimpleMCPrimary> SimpleMCPrimaryList;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief SimpleMCPrimary class
 */
class SimpleMCEvent
{
public:
    /**
     *  @brief  Constructor
     */
    SimpleMCEvent();

    int                     m_fileIdentifier;           ///< The file identifier
    int                     m_eventNumber;              ///< The event number
    int                     m_nMCNeutrinos;             ///< The number of mc neutrinos
    int                     m_mcNeutrinoPdg;            ///< The mc neutrino pdg code
    int                     m_mcNeutrinoNuance;         ///< The mc neutrino nuance code (interaction type details)
    SimpleThreeVector       m_mcNeutrinoVtx;            ///< The mc neutrino vertex
    int                     m_nRecoNeutrinos;           ///< The number of reconstructed neutrinos
    int                     m_recoNeutrinoPdg;          ///< The reconstructed neutrino pdg code
    SimpleThreeVector       m_recoNeutrinoVtx;          ///< The reconstructed neutrino vertex
    int                     m_nMCPrimaries;             ///< The number of mc primaries
    SimpleMCPrimaryList     m_mcPrimaryList;            ///< The list of mc primaries
};

typedef std::vector<SimpleMCEvent> SimpleMCEventList;

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
    CCQEL_MU_N,
    CCQEL_MU_P,
    CCQEL_MU_P_N,
    CCQEL_MU_P_P,
    CCQEL_MU_P_P_N,
    NCQEL_P_P,
    NCQEL_P_P_N,
    CCRES_MU_P_PIPLUS,
    CCRES_MU_P_PIPLUS_N,
    CCRES_MU_N_PIPLUS,
    OTHER_INTERACTION
};

typedef std::map<InteractionType, CountingMap> InteractionTypeToCountingMap;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Get string representing interaction type
 * 
 *  @param  interactionType
 * 
 *  @return the interaction type string
 */
std::string ToString(const InteractionType interactionType);

/**
 *  @brief  Get string representing expected primary
 * 
 *  @param  expectedPrimary
 * 
 *  @return the expected primary string
 */
std::string ToString(const ExpectedPrimary expectedPrimary);

/**
 *  @brief  Read the next event from the chain
 * 
 *  @param  pTChain the address of the chain
 *  @param  iEntry the first chain entry to read
 *  @param  simpleMCEvent the event to be populated
 *
 *  @return the number of chain entries read
 */
unsigned int ReadNextEvent(TChain *const pTChain, const unsigned int iEntry, SimpleMCEvent &simpleMCEvent);

/**
 *  @brief  Print details to screen for a simple mc event
 * 
 *  @param  simpleMCEvent the simple mc event
 *  @param  primaryMinHits the min number of hits in order to consider a primary
 *  @param  minMatchedHits the min number of matched hits in order to consider a matched pfo
 */
void DisplaySimpleMCEvent(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits);

/**
 *  @brief  Print details to screen for a provided interaction type to counting map
 * 
 *  @param  interactionTypeToCountingMap the interaction type to counting map
 */
void DisplayInteractionTypeToCountingMap(const InteractionTypeToCountingMap &interactionTypeToCountingMap);

/**
 *  @brief  Get the event interaction type
 * 
 *  @param  simpleMCEvent
 *  @param  primaryMinHits
 * 
 *  @return the interaction type
 */
InteractionType GetInteractionType(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits);

/**
 *  @brief  GetStrongestPfoMatch
 * 
 *  @param  simpleMCEvent
 *  @param  usedMCIds
 *  @param  usedPfoIds
 *  @param  pfoMatchingMap
 */
bool GetStrongestPfoMatch(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, IntSet &usedMCIds,
    IntSet &usedPfoIds, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  GetRemainingPfoMatches
 * 
 *  @param  simpleMCEvent
 *  @param  primaryMinHits
 *  @param  minMatchedHits
 *  @param  usedPfoIds
 *  @param  pfoMatchingMap
 */
void GetRemainingPfoMatches(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, const IntSet &usedPfoIds,
    PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  FinalisePfoMatching
 * 
 *  @param  simpleMCPrimary
 *  @param  primaryMinHits
 *  @param  minMatchedHits
 *  @param  pfoMatchingMap
 */
void FinalisePfoMatching(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits, PfoMatchingMap &pfoMatchingMap);

/**
 *  @brief  GetExpectedPrimary Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  primaryId
 *  @param  simpleMCPrimaryList
 *  @param  primaryMinHits
 * 
 *  @return the expected primary
 */
ExpectedPrimary GetExpectedPrimary(const int primaryId, const SimpleMCPrimaryList &simpleMCPrimaryList, const int primaryMinHits);

/**
 *  @brief  CountPfoMatches Relies on fact that primary list is sorted by number of true hits
 * 
 *  @param  simpleMCEvent
 *  @param  interactionType
 *  @param  pfoMatchingMap
 *  @param  primaryMinHits
 *  @param  interactionTypeToCountingMap
 */
void CountPfoMatches(const SimpleMCEvent &simpleMCEvent, const InteractionType interactionType, const PfoMatchingMap &pfoMatchingMap,
    const int primaryMinHits, InteractionTypeToCountingMap &interactionTypeToCountingMap);

/**
 *  @brief  Validation
 * 
 *  @param  inputFiles
 *  @param  shouldDisplay
 *  @param  maxEvents
 *  @param  primaryMinHits
 *  @param  nuance
 */
void Validation(const std::string &inputFiles, const bool shouldDisplay = true, const int skipEvents = 0, const int nEventsToProcess = std::numeric_limits<int>::max(),
    const int primaryMinHits = 0, const int minMatchedHits = 0);

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
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleMatchedPfo::SimpleMatchedPfo() :
    m_id(-1),
    m_pdgCode(0), 
    m_nPfoHitsTotal(0),
    m_nPfoHitsU(0),
    m_nPfoHitsV(0),
    m_nPfoHitsW(0),
    m_nMatchedHitsTotal(0),
    m_nMatchedHitsU(0),
    m_nMatchedHitsV(0),
    m_nMatchedHitsW(0),
    m_vertex(0.f, 0.f, 0.f),
    m_endpoint(0.f, 0.f, 0.f),
    m_vertexDirection(0.f, 0.f, 0.f),
    m_endDirection(0.f, 0.f, 0.f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleMCPrimary::SimpleMCPrimary() :
    m_id(-1),
    m_pdgCode(0),
    m_nMCHitsTotal(0),  
    m_nMCHitsU(0),
    m_nMCHitsV(0),
    m_nMCHitsW(0),
    m_energy(0.f),
    m_momentum(0.f, 0.f, 0.f),
    m_vertex(-1.f, -1.f, -1.f),
    m_endpoint(-1.f, -1.f, -1.f),
    m_nMatchedPfos(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

SimpleMCEvent::SimpleMCEvent() :
    m_fileIdentifier(-1),
    m_eventNumber(0),
    m_nMCNeutrinos(0),
    m_mcNeutrinoPdg(0),
    m_mcNeutrinoNuance(-1),
    m_mcNeutrinoVtx(-1.f, -1.f, -1.f),
    m_nRecoNeutrinos(0),
    m_recoNeutrinoPdg(0),
    m_recoNeutrinoVtx(-1.f, -1.f, -1.f),
    m_nMCPrimaries(0)
{
}

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

//------------------------------------------------------------------------------------------------------------------------------------------

std::string ToString(const InteractionType interactionType)
{
    switch (interactionType)
    {
    case CCQEL_MU: return "CCQEL_MU";
    case CCQEL_MU_N: return "CCQEL_MU_N";
    case CCQEL_MU_P: return "CCQEL_MU_P";
    case CCQEL_MU_P_N: return "CCQEL_MU_P_N";
    case CCQEL_MU_P_P: return "CCQEL_MU_P_P";
    case CCQEL_MU_P_P_N: return "CCQEL_MU_P_P_N";
    case NCQEL_P_P: return "NCQEL_P_P";
    case NCQEL_P_P_N: return "NCQEL_P_P_N";
    case CCRES_MU_P_PIPLUS: return "CCRES_MU_P_PIPLUS";
    case CCRES_MU_P_PIPLUS_N: return "CCRES_MU_P_PIPLUS_N";
    case CCRES_MU_N_PIPLUS: return "CCRES_MU_N_PIPLUS";
    case OTHER_INTERACTION: return "OTHER_INTERACTION";
    default: return "UNKNOWN";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

unsigned int ReadNextEvent(TChain *const pTChain, const unsigned int iEntry, SimpleMCEvent &simpleMCEvent)
{
    pTChain->SetBranchAddress("fileIdentifier", &simpleMCEvent.m_fileIdentifier);
    pTChain->SetBranchAddress("eventNumber", &simpleMCEvent.m_eventNumber);
    pTChain->SetBranchAddress("nMCNeutrinos", &simpleMCEvent.m_nMCNeutrinos);
    pTChain->SetBranchAddress("mcNeutrinoPdg", &simpleMCEvent.m_mcNeutrinoPdg);
    pTChain->SetBranchAddress("mcNeutrinoNuance", &simpleMCEvent.m_mcNeutrinoNuance);
    pTChain->SetBranchAddress("nRecoNeutrinos", &simpleMCEvent.m_nRecoNeutrinos);
    pTChain->SetBranchAddress("mcNeutrinoVtxX", &simpleMCEvent.m_mcNeutrinoVtx.m_x);
    pTChain->SetBranchAddress("mcNeutrinoVtxY", &simpleMCEvent.m_mcNeutrinoVtx.m_y);
    pTChain->SetBranchAddress("mcNeutrinoVtxZ", &simpleMCEvent.m_mcNeutrinoVtx.m_z);
    pTChain->SetBranchAddress("recoNeutrinoPdg", &simpleMCEvent.m_recoNeutrinoPdg);
    pTChain->SetBranchAddress("recoNeutrinoVtxX", &simpleMCEvent.m_recoNeutrinoVtx.m_x);
    pTChain->SetBranchAddress("recoNeutrinoVtxY", &simpleMCEvent.m_recoNeutrinoVtx.m_y);
    pTChain->SetBranchAddress("recoNeutrinoVtxZ", &simpleMCEvent.m_recoNeutrinoVtx.m_z);
    pTChain->SetBranchAddress("nMCPrimaries", &simpleMCEvent.m_nMCPrimaries);

    pTChain->GetEntry(iEntry);

    for (int iPrimary = 0; iPrimary < simpleMCEvent.m_nMCPrimaries; ++iPrimary)
    {
        SimpleMCPrimary simpleMCPrimary;

        pTChain->SetBranchAddress("mcPrimaryId", &simpleMCPrimary.m_id);
        pTChain->SetBranchAddress("mcPrimaryPdg", &simpleMCPrimary.m_pdgCode);
        pTChain->SetBranchAddress("mcPrimaryNHitsTotal", &simpleMCPrimary.m_nMCHitsTotal);
        pTChain->SetBranchAddress("mcPrimaryNHitsU", &simpleMCPrimary.m_nMCHitsU);
        pTChain->SetBranchAddress("mcPrimaryNHitsV", &simpleMCPrimary.m_nMCHitsV);
        pTChain->SetBranchAddress("mcPrimaryNHitsW", &simpleMCPrimary.m_nMCHitsW);
        pTChain->SetBranchAddress("mcPrimaryE", &simpleMCPrimary.m_energy);
        pTChain->SetBranchAddress("mcPrimaryPX", &simpleMCPrimary.m_momentum.m_x);
        pTChain->SetBranchAddress("mcPrimaryPY", &simpleMCPrimary.m_momentum.m_y);
        pTChain->SetBranchAddress("mcPrimaryPZ", &simpleMCPrimary.m_momentum.m_z);
        pTChain->SetBranchAddress("mcPrimaryVtxX", &simpleMCPrimary.m_vertex.m_x);
        pTChain->SetBranchAddress("mcPrimaryVtxY", &simpleMCPrimary.m_vertex.m_y);
        pTChain->SetBranchAddress("mcPrimaryVtxZ", &simpleMCPrimary.m_vertex.m_z);
        pTChain->SetBranchAddress("mcPrimaryEndX", &simpleMCPrimary.m_endpoint.m_x);
        pTChain->SetBranchAddress("mcPrimaryEndY", &simpleMCPrimary.m_endpoint.m_y);
        pTChain->SetBranchAddress("mcPrimaryEndZ", &simpleMCPrimary.m_endpoint.m_z);
        pTChain->SetBranchAddress("mcPrimaryNMatchedPfos", &simpleMCPrimary.m_nMatchedPfos);

        IntVector *pPfoIdVector(NULL), *pPfoPdgVector(NULL), *pPfoNHitsTotalVector(NULL), *pPfoNHitsUVector(NULL),
            *pPfoNHitsVVector(NULL), *pPfoNHitsWVector(NULL), *pPfoNMatchedHitsTotalVector(NULL), *pPfoNMatchedHitsUVector(NULL),
            *pPfoNMatchedHitsVVector(NULL), *pPfoNMatchedHitsWVector(NULL);
        FloatVector *pPfoVtxXVector(NULL), *pPfoVtxYVector(NULL), *pPfoVtxZVector(NULL), *pPfoEndXVector(NULL), *pPfoEndYVector(NULL),
            *pPfoEndZVector(NULL), *pPfoVtxDirXVector(NULL), *pPfoVtxDirYVector(NULL), *pPfoVtxDirZVector(NULL), *pPfoEndDirXVector(NULL),
            *pPfoEndDirYVector(NULL), *pPfoEndDirZVector(NULL);

        pTChain->SetBranchAddress("matchedPfoId", &pPfoIdVector);
        pTChain->SetBranchAddress("matchedPfoPdg", &pPfoPdgVector);
        pTChain->SetBranchAddress("matchedPfoNHitsTotal", &pPfoNHitsTotalVector);
        pTChain->SetBranchAddress("matchedPfoNHitsU", &pPfoNHitsUVector);
        pTChain->SetBranchAddress("matchedPfoNHitsV", &pPfoNHitsVVector);
        pTChain->SetBranchAddress("matchedPfoNHitsW", &pPfoNHitsWVector);
        pTChain->SetBranchAddress("matchedPfoNMatchedHitsTotal", &pPfoNMatchedHitsTotalVector);
        pTChain->SetBranchAddress("matchedPfoNMatchedHitsU", &pPfoNMatchedHitsUVector);
        pTChain->SetBranchAddress("matchedPfoNMatchedHitsV", &pPfoNMatchedHitsVVector);
        pTChain->SetBranchAddress("matchedPfoNMatchedHitsW", &pPfoNMatchedHitsWVector);

        pTChain->SetBranchAddress("matchedPfoVtxX", &pPfoVtxXVector);
        pTChain->SetBranchAddress("matchedPfoVtxY", &pPfoVtxYVector);
        pTChain->SetBranchAddress("matchedPfoVtxZ", &pPfoVtxZVector);
        pTChain->SetBranchAddress("matchedPfoEndX", &pPfoEndXVector);
        pTChain->SetBranchAddress("matchedPfoEndY", &pPfoEndYVector);
        pTChain->SetBranchAddress("matchedPfoEndZ", &pPfoEndZVector);
        pTChain->SetBranchAddress("matchedPfoVtxDirX", &pPfoVtxDirXVector);
        pTChain->SetBranchAddress("matchedPfoVtxDirY", &pPfoVtxDirYVector);
        pTChain->SetBranchAddress("matchedPfoVtxDirZ", &pPfoVtxDirZVector);
        pTChain->SetBranchAddress("matchedPfoEndDirX", &pPfoEndDirXVector);
        pTChain->SetBranchAddress("matchedPfoEndDirY", &pPfoEndDirYVector);
        pTChain->SetBranchAddress("matchedPfoEndDirZ", &pPfoEndDirZVector);

        pTChain->GetEntry(iEntry + iPrimary);

        for (int iMatchedPfo = 0; iMatchedPfo < simpleMCPrimary.m_nMatchedPfos; ++iMatchedPfo)
        {
            SimpleMatchedPfo simpleMatchedPfo;
            simpleMatchedPfo.m_id = pPfoIdVector->at(iMatchedPfo);
            simpleMatchedPfo.m_pdgCode = pPfoPdgVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nPfoHitsTotal = pPfoNHitsTotalVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nPfoHitsU = pPfoNHitsUVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nPfoHitsV = pPfoNHitsVVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nPfoHitsW = pPfoNHitsWVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nMatchedHitsTotal = pPfoNMatchedHitsTotalVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nMatchedHitsU = pPfoNMatchedHitsUVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nMatchedHitsV = pPfoNMatchedHitsVVector->at(iMatchedPfo);
            simpleMatchedPfo.m_nMatchedHitsW = pPfoNMatchedHitsWVector->at(iMatchedPfo);
            simpleMatchedPfo.m_vertex.m_x = pPfoVtxXVector->at(iMatchedPfo);
            simpleMatchedPfo.m_vertex.m_y = pPfoVtxYVector->at(iMatchedPfo);
            simpleMatchedPfo.m_vertex.m_z = pPfoVtxZVector->at(iMatchedPfo);
            simpleMatchedPfo.m_endpoint.m_x = pPfoEndXVector->at(iMatchedPfo);
            simpleMatchedPfo.m_endpoint.m_y = pPfoEndYVector->at(iMatchedPfo);
            simpleMatchedPfo.m_endpoint.m_z = pPfoEndZVector->at(iMatchedPfo);
            simpleMatchedPfo.m_vertexDirection.m_x = pPfoVtxDirXVector->at(iMatchedPfo);
            simpleMatchedPfo.m_vertexDirection.m_y = pPfoVtxDirYVector->at(iMatchedPfo);
            simpleMatchedPfo.m_vertexDirection.m_z = pPfoVtxDirZVector->at(iMatchedPfo);
            simpleMatchedPfo.m_endDirection.m_x = pPfoEndDirXVector->at(iMatchedPfo);
            simpleMatchedPfo.m_endDirection.m_y = pPfoEndDirYVector->at(iMatchedPfo);
            simpleMatchedPfo.m_endDirection.m_z = pPfoEndDirZVector->at(iMatchedPfo);

            simpleMCPrimary.m_matchedPfoList.push_back(simpleMatchedPfo);
        }

        simpleMCEvent.m_mcPrimaryList.push_back(simpleMCPrimary);
    }

    pTChain->ResetBranchAddresses();
    return simpleMCEvent.m_nMCPrimaries;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplaySimpleMCEvent(const SimpleMCEvent &simpleMCEvent, const int primaryMinHits, const int minMatchedHits)
{
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "File " << simpleMCEvent.m_fileIdentifier << ", event " << simpleMCEvent.m_eventNumber << std::endl
              << "nuPdg " << simpleMCEvent.m_mcNeutrinoPdg << ", nuance " << simpleMCEvent.m_mcNeutrinoNuance
              << " (" << simpleMCEvent.m_nMCNeutrinos << ") " << std::endl << "recoNuPdg " << simpleMCEvent.m_recoNeutrinoPdg
              << " (" << simpleMCEvent.m_nRecoNeutrinos << ") " << std::endl;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        if (simpleMCPrimary.m_nMCHitsTotal >= primaryMinHits)
        {
            std::cout << std::endl << "MCPrimary " << simpleMCPrimary.m_id << ", PDG " << simpleMCPrimary.m_pdgCode
                      << ", nMCHits " << simpleMCPrimary.m_nMCHitsTotal << " (" << simpleMCPrimary.m_nMCHitsU
                      << ", " << simpleMCPrimary.m_nMCHitsV << ", " << simpleMCPrimary.m_nMCHitsW << ")" << std::endl;
        }

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);

            if ((simpleMCPrimary.m_nMCHitsTotal >= primaryMinHits) && (simpleMatchedPfo.m_nMatchedHitsTotal >= minMatchedHits))
            {
                std::cout << "-MatchedPfo " << simpleMatchedPfo.m_id << ", PDG " << simpleMatchedPfo.m_pdgCode
                          << ", nMatchedHits " << simpleMatchedPfo.m_nMatchedHitsTotal << " (" << simpleMatchedPfo.m_nMatchedHitsU
                          << ", " << simpleMatchedPfo.m_nMatchedHitsV << ", " << simpleMatchedPfo.m_nMatchedHitsW << ")"
                          << ", nPfoHits " << simpleMatchedPfo.m_nPfoHitsTotal << " (" << simpleMatchedPfo.m_nPfoHitsU
                          << ", " << simpleMatchedPfo.m_nPfoHitsV << ", " << simpleMatchedPfo.m_nPfoHitsW << ")" << std::endl;
            }
        }
    }
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void DisplayInteractionTypeToCountingMap(const InteractionTypeToCountingMap &interactionTypeToCountingMap)
{
    for (InteractionTypeToCountingMap::const_iterator iter = interactionTypeToCountingMap.begin(); iter != interactionTypeToCountingMap.end(); ++iter)
    {
        const InteractionType interactionType(iter->first);
        const CountingMap &countingMap(iter->second);
        std::cout << std::endl << ToString(interactionType) << std::endl;

        for (CountingMap::const_iterator cIter = countingMap.begin(); cIter != countingMap.end(); ++cIter)
        {
            const ExpectedPrimary expectedPrimary(cIter->first);
            const CountingDetails &countingDetails(cIter->second);

            std::cout << "-" << ToString(expectedPrimary) << ": nEvents: " << countingDetails.m_nTotal << ", nPfos |0: " << countingDetails.m_nMatch0
                      << "|, |1: " << countingDetails.m_nMatch1 << "|, |2: " << countingDetails.m_nMatch2 << "|, |3+: " << countingDetails.m_nMatch3Plus << "|" << std::endl;
        }
    }
}
