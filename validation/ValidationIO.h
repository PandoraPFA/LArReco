/**
 *  @file   LArReco/validation/ValidationIO.h
 *
 *  @brief  Header file for validation io functionality
 *
 *  $Log: $
 */
#ifndef LAR_VALIDATION_IO_H
#define LAR_VALIDATION_IO_H 1

#include <cmath>
#include <iostream>
#include <iomanip>
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
    int                     m_parentId;                 ///< The unique identifier of the parent pfo (-1 if no parent set)
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
    int                     m_nGoodMCHitsTotal;         ///< The total number of good mc hits
    int                     m_nGoodMCHitsU;             ///< The number of good u mc hits
    int                     m_nGoodMCHitsV;             ///< The number of good v mc hits
    int                     m_nGoodMCHitsW;             ///< The number of good w mc hits
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
 *  @brief SimpleMCEvent class
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
    float                   m_mcNeutrinoE;              ///< The mc neutrino energy
    SimpleThreeVector       m_mcNeutrinoP;              ///< The mc neutrino momentum
    int                     m_nRecoNeutrinos;           ///< The number of reconstructed neutrinos
    int                     m_recoNeutrinoPdg;          ///< The reconstructed neutrino pdg code
    SimpleThreeVector       m_recoNeutrinoVtx;          ///< The reconstructed neutrino vertex
    int                     m_nEventNeutrinoHitsTotal;  ///< The total number of neutrino-induced hits in the event
    int                     m_nEventOtherHitsTotal;     ///< The total number of other (non-neutrino) hits in the event
    int                     m_nRecoNeutrinoHitsTotal;   ///< The total number of neutrino-induced hits in the input reco neutrino(s)
    int                     m_nRecoOtherHitsTotal;      ///< The total number of (non-neutrino) hits in the input reco neutrino(s)
    int                     m_recoNeutrinoNPrimaries;   ///< The number of reconstructed primary partices in the input reco neutrino(s)
    int                     m_nMCPrimaries;             ///< The number of mc primaries
    SimpleMCPrimaryList     m_mcPrimaryList;            ///< The list of mc primaries
};

typedef std::vector<SimpleMCEvent> SimpleMCEventList;

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  ValidationIO class
 */
class ValidationIO
{
public:
    /**
     *  @brief  Read the next event from the chain
     * 
     *  @param  pTChain the address of the chain
     *  @param  iEntry the first chain entry to read
     *  @param  simpleMCEvent the event to be populated
     *
     *  @return the number of chain entries read
     */
    static unsigned int ReadNextEvent(TChain *const pTChain, const unsigned int iEntry, SimpleMCEvent &simpleMCEvent);

    /**
     *  @brief  Print details to screen for a simple mc event
     * 
     *  @param  simpleMCEvent the simple mc event
     */
    static void DisplaySimpleMCEvent(const SimpleMCEvent &simpleMCEvent);
};

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
    m_nGoodMCHitsTotal(0),
    m_nGoodMCHitsU(0),
    m_nGoodMCHitsV(0),
    m_nGoodMCHitsW(0),
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

unsigned int ValidationIO::ReadNextEvent(TChain *const pTChain, const unsigned int iEntry, SimpleMCEvent &simpleMCEvent)
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
    pTChain->SetBranchAddress("mcNeutrinoE", &simpleMCEvent.m_mcNeutrinoE);
    pTChain->SetBranchAddress("mcNeutrinoPX", &simpleMCEvent.m_mcNeutrinoP.m_x);
    pTChain->SetBranchAddress("mcNeutrinoPY", &simpleMCEvent.m_mcNeutrinoP.m_y);
    pTChain->SetBranchAddress("mcNeutrinoPZ", &simpleMCEvent.m_mcNeutrinoP.m_z);
    pTChain->SetBranchAddress("recoNeutrinoPdg", &simpleMCEvent.m_recoNeutrinoPdg);
    pTChain->SetBranchAddress("recoNeutrinoVtxX", &simpleMCEvent.m_recoNeutrinoVtx.m_x);
    pTChain->SetBranchAddress("recoNeutrinoVtxY", &simpleMCEvent.m_recoNeutrinoVtx.m_y);
    pTChain->SetBranchAddress("recoNeutrinoVtxZ", &simpleMCEvent.m_recoNeutrinoVtx.m_z);
    pTChain->SetBranchAddress("nEventNeutrinoHitsTotal", &simpleMCEvent.m_nEventNeutrinoHitsTotal);
    pTChain->SetBranchAddress("nEventOtherHitsTotal", &simpleMCEvent.m_nEventOtherHitsTotal);
    pTChain->SetBranchAddress("nRecoNeutrinoHitsTotal", &simpleMCEvent.m_nRecoNeutrinoHitsTotal);
    pTChain->SetBranchAddress("nRecoOtherHitsTotal", &simpleMCEvent.m_nRecoOtherHitsTotal);
    pTChain->SetBranchAddress("nMCPrimaries", &simpleMCEvent.m_nMCPrimaries);
    pTChain->SetBranchAddress("recoNeutrinoNPrimaries", &simpleMCEvent.m_recoNeutrinoNPrimaries);

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
        pTChain->SetBranchAddress("mcPrimaryNGoodHitsTotal", &simpleMCPrimary.m_nGoodMCHitsTotal);
        pTChain->SetBranchAddress("mcPrimaryNGoodHitsU", &simpleMCPrimary.m_nGoodMCHitsU);
        pTChain->SetBranchAddress("mcPrimaryNGoodHitsV", &simpleMCPrimary.m_nGoodMCHitsV);
        pTChain->SetBranchAddress("mcPrimaryNGoodHitsW", &simpleMCPrimary.m_nGoodMCHitsW);
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

        IntVector *pPfoIdVector(NULL), *pPfoParentIdVector(NULL), *pPfoPdgVector(NULL), *pPfoNHitsTotalVector(NULL), *pPfoNHitsUVector(NULL),
            *pPfoNHitsVVector(NULL), *pPfoNHitsWVector(NULL), *pPfoNMatchedHitsTotalVector(NULL), *pPfoNMatchedHitsUVector(NULL),
            *pPfoNMatchedHitsVVector(NULL), *pPfoNMatchedHitsWVector(NULL);
        FloatVector *pPfoVtxXVector(NULL), *pPfoVtxYVector(NULL), *pPfoVtxZVector(NULL), *pPfoEndXVector(NULL), *pPfoEndYVector(NULL),
            *pPfoEndZVector(NULL), *pPfoVtxDirXVector(NULL), *pPfoVtxDirYVector(NULL), *pPfoVtxDirZVector(NULL), *pPfoEndDirXVector(NULL),
            *pPfoEndDirYVector(NULL), *pPfoEndDirZVector(NULL);

        pTChain->SetBranchAddress("matchedPfoId", &pPfoIdVector);
        pTChain->SetBranchAddress("matchedPfoParentId", &pPfoParentIdVector);
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
            simpleMatchedPfo.m_parentId = pPfoParentIdVector->at(iMatchedPfo);
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

void ValidationIO::DisplaySimpleMCEvent(const SimpleMCEvent &simpleMCEvent)
{
    const std::streamsize ss(std::cout.precision());

    std::cout << "---RAW-MATCHING-OUTPUT--------------------------------------------------------------------------" << std::endl;
    std::cout << "File " << simpleMCEvent.m_fileIdentifier << ", event " << simpleMCEvent.m_eventNumber << std::endl
              << "nuPdg " << simpleMCEvent.m_mcNeutrinoPdg << " (" << simpleMCEvent.m_nMCNeutrinos << "), nuance "
              << simpleMCEvent.m_mcNeutrinoNuance << ", nPrimaries " << simpleMCEvent.m_nMCPrimaries << std::endl
              << "recoNuPdg " << simpleMCEvent.m_recoNeutrinoPdg << " (" << simpleMCEvent.m_nRecoNeutrinos << ")";

    if (simpleMCEvent.m_nRecoNeutrinoHitsTotal + simpleMCEvent.m_nRecoOtherHitsTotal > 0)
        std::cout << ", Purity " << std::setprecision(2) << (static_cast<float>(simpleMCEvent.m_nRecoNeutrinoHitsTotal) / static_cast<float>(simpleMCEvent.m_nRecoNeutrinoHitsTotal + simpleMCEvent.m_nRecoOtherHitsTotal))
                  << std::setprecision(ss) << " (" << simpleMCEvent.m_nRecoNeutrinoHitsTotal << " / " << (simpleMCEvent.m_nRecoNeutrinoHitsTotal + simpleMCEvent.m_nRecoOtherHitsTotal) << ")";

    if (simpleMCEvent.m_nEventNeutrinoHitsTotal > 0)
        std::cout << ", Completeness " << std::setprecision(2) << (static_cast<float>(simpleMCEvent.m_nRecoNeutrinoHitsTotal)  / static_cast<float>(simpleMCEvent.m_nEventNeutrinoHitsTotal))
                  << std::setprecision(ss) << " (" << simpleMCEvent.m_nRecoNeutrinoHitsTotal << " / " << simpleMCEvent.m_nEventNeutrinoHitsTotal << ")";

    std::cout << ", nPrimaries " << simpleMCEvent.m_recoNeutrinoNPrimaries << std::endl;

    for (SimpleMCPrimaryList::const_iterator pIter = simpleMCEvent.m_mcPrimaryList.begin(); pIter != simpleMCEvent.m_mcPrimaryList.end(); ++pIter)
    {
        const SimpleMCPrimary &simpleMCPrimary(*pIter);

        std::cout << std::endl << "Primary " << simpleMCPrimary.m_id << ", PDG " << simpleMCPrimary.m_pdgCode
                  << ", nMCHits " << simpleMCPrimary.m_nMCHitsTotal << " (" << simpleMCPrimary.m_nMCHitsU
                  << ", " << simpleMCPrimary.m_nMCHitsV << ", " << simpleMCPrimary.m_nMCHitsW << "),"
                  << " [nGood " << simpleMCPrimary.m_nGoodMCHitsTotal << " (" << simpleMCPrimary.m_nGoodMCHitsU << ", " << simpleMCPrimary.m_nGoodMCHitsV
                  << ", " << simpleMCPrimary.m_nGoodMCHitsW << ")]" << std::endl;

        for (SimpleMatchedPfoList::const_iterator mIter = simpleMCPrimary.m_matchedPfoList.begin(); mIter != simpleMCPrimary.m_matchedPfoList.end(); ++mIter)
        {
            const SimpleMatchedPfo &simpleMatchedPfo(*mIter);
            std::cout << "-MatchedPfo " << simpleMatchedPfo.m_id;

            if (simpleMatchedPfo.m_parentId >= 0)
                std::cout << ", ParentPfo " << simpleMatchedPfo.m_parentId;

            std::cout << ", PDG " << simpleMatchedPfo.m_pdgCode
                      << ", nMatchedHits " << simpleMatchedPfo.m_nMatchedHitsTotal << " (" << simpleMatchedPfo.m_nMatchedHitsU
                      << ", " << simpleMatchedPfo.m_nMatchedHitsV << ", " << simpleMatchedPfo.m_nMatchedHitsW << ")"
                      << ", nPfoHits " << simpleMatchedPfo.m_nPfoHitsTotal << " (" << simpleMatchedPfo.m_nPfoHitsU
                      << ", " << simpleMatchedPfo.m_nPfoHitsV << ", " << simpleMatchedPfo.m_nPfoHitsW << ")" << std::endl;
        }
    }
    std::cout << "------------------------------------------------------------------------------------------------" << std::endl;
}

#endif // #ifndef LAR_VALIDATION_IO_H
