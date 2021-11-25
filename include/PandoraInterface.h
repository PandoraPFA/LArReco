/**
 *  @file   LArReco/include/PandoraInterface.h
 *
 *  @brief  Header file for PandoraInterface.
 *
 *  $Log: $
 */
#ifndef PANDORA_ND_INTERFACE_H
#define PANDORA_ND_INTERFACE_H 1

#include "Pandora/PandoraInputTypes.h"
#include "TG4Event.h"

namespace pandora
{
class Pandora;
}

//------------------------------------------------------------------------------------------------------------------------------------------

namespace lar_nd_reco
{

typedef std::map<int, float> MCParticleEnergyMap;

/**
 *  @brief  Parameters class
 */
class Parameters
{
public:
    /**
     *  @brief  Default constructor
     */
    Parameters();

    std::string m_settingsFile;  ///< The path to the pandora settings file
                                 ///< (mandatory parameter)
    std::string m_inputFileName; ///< The path to the input file containing events
                                 ///< and/or geometry information

    int m_nEventsToProcess;          ///< The number of events to process (default all
                                     ///< events in file)
    bool m_shouldDisplayEventNumber; ///< Whether event numbers should be
                                     ///< displayed (default false)

    bool m_shouldRunAllHitsCosmicReco;  ///< Whether to run all hits cosmic-ray reconstruction
    bool m_shouldRunStitching;          ///< Whether to stitch cosmic-ray muons crossing between volumes
    bool m_shouldRunCosmicHitRemoval;   ///< Whether to remove hits from tagged cosmic-rays
    bool m_shouldRunSlicing;            ///< Whether to slice events into separate regions for processing
    bool m_shouldRunNeutrinoRecoOption; ///< Whether to run neutrino reconstruction for each slice
    bool m_shouldRunCosmicRecoOption;   ///< Whether to run cosmic-ray reconstruction for each slice
    bool m_shouldPerformSliceId;        ///< Whether to identify slices and select most appropriate pfos
    bool m_printOverallRecoStatus;      ///< Whether to print current operation status messages

    int m_nEventsToSkip;   ///< The number of events to skip
    int m_maxMergedVoxels; ///< The max number of merged voxels to process (default all)

    bool m_use3D;     ///< Create 3D LArCaloHits
    bool m_useLArTPC; ///< Create LArTPC LArCaloHits with u,v,w views

    float m_voxelWidth; ///< Voxel box width (cm)

    const float m_mm2cm{0.1};           ///< Geant4 mm to cm conversion
    const float m_MeV2GeV{1e-3};        ///< Geant4 MeV to GeV conversion
    const float m_voxelPathShift{1e-3}; ///< Small path shift to find next voxel
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline Parameters::Parameters() :
    m_settingsFile(""),
    m_inputFileName(""),
    m_nEventsToProcess(-1),
    m_shouldDisplayEventNumber(false),
    m_shouldRunAllHitsCosmicReco(true),
    m_shouldRunStitching(true),
    m_shouldRunCosmicHitRemoval(true),
    m_shouldRunSlicing(true),
    m_shouldRunNeutrinoRecoOption(true),
    m_shouldRunCosmicRecoOption(true),
    m_shouldPerformSliceId(true),
    m_printOverallRecoStatus(false),
    m_nEventsToSkip(0),
    m_use3D(true),
    m_useLArTPC(false),
    m_voxelWidth(0.4f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  LArVoxel class
 */
class LArVoxel
{
public:
    /**
     *  @brief  Constructor
     *
     *  @param  voxelID Total bin number for the voxel (long integer, since it can be > 2^31)
     *  @param  energyInVoxel The total deposited energy in the voxel (GeV)
     *  @param  voxelPosVect Voxel position, set as the first corner of the voxel bin
     *  @param  trackID The Geant4 ID of the (first) contributing track to this voxel
     */
    LArVoxel(long voxelID, float energyInVoxel, const pandora::CartesianVector &voxelPosVect, int trackID);

    /**
     *  @brief  Set voxel energy (GeV)
     *
     *  @param  E voxel energy
     */
    void SetEnergy(float E);

    long m_voxelID;                          ///< The long integer ID of the voxel (can be larger than 2^31)
    float m_energyInVoxel;                   ///< The energy in the voxel (GeV)
    pandora::CartesianVector m_voxelPosVect; ///< Position vector (x,y,z) of the first voxel corner
    int m_trackID;                           ///< The Geant4 ID of the (first) contributing track to this voxel
};

typedef std::vector<LArVoxel> LArVoxelList;

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArVoxel::LArVoxel(long voxelID, float energyInVoxel, const pandora::CartesianVector &voxelPosVect, int trackID) :
    m_voxelID(voxelID),
    m_energyInVoxel(energyInVoxel),
    m_voxelPosVect(voxelPosVect),
    m_trackID(trackID)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline void LArVoxel::SetEnergy(float E)
{
    m_energyInVoxel = E;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

typedef std::array<int, 3> SignComponentArray;

/**
 * @brief  Ray class
 */
class LArRay
{
public:
    /**
     *  @brief  Constructor
     *
     *  @param  origin The starting point of the ray
     *  @param  dir The direction vector of the ray
     */
    LArRay(const pandora::CartesianVector &origin, const pandora::CartesianVector &dir);

    /**
     *  @brief  Update ray starting position
     *
     *  @param  newOrigin The new starting position (CartesianVector)
     */
    void UpdateOrigin(const pandora::CartesianVector &newOrigin);

    /**
     *  @brief  Get position along ray from its start point
     *
     *  @param  length Distance (cm) along the ray from its starting point
     *
     *  @return The position as a CartesianVector
     */
    pandora::CartesianVector GetPoint(double length) const;

    pandora::CartesianVector m_origin; ///< Starting point of the ray
    pandora::CartesianVector m_dir;    ///< Ray direction
    pandora::CartesianVector m_invDir; ///< Reciprocal direction vector
    SignComponentArray m_sign;         ///< Sign of reciprocal direction vector components
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArRay::LArRay(const pandora::CartesianVector &origin, const pandora::CartesianVector &dir) :
    m_origin(origin),
    m_dir(dir),
    m_invDir({0.f, 0.f, 0.f}),
    m_sign({0, 0, 0})
{
    const float dx(m_dir.GetX());
    const float dy(m_dir.GetY());
    const float dz(m_dir.GetZ());

    // maxVal is needed to set the inverse direction components for parallel lines
    const float maxVal(std::numeric_limits<float>::max());
    const float invdx(std::fabs(dx) > 0.0 ? 1.0 / dx : maxVal);
    const float invdy(std::fabs(dy) > 0.0 ? 1.0 / dy : maxVal);
    const float invdz(std::fabs(dz) > 0.0 ? 1.0 / dz : maxVal);

    m_invDir.SetValues(invdx, invdy, invdz);

    m_sign[0] = (m_invDir.GetX() < 0);
    m_sign[1] = (m_invDir.GetY() < 0);
    m_sign[2] = (m_invDir.GetZ() < 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline void LArRay::UpdateOrigin(const pandora::CartesianVector &newOrigin)
{
    m_origin = newOrigin;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::CartesianVector LArRay::GetPoint(double length) const
{
    return (m_origin + m_dir * length);
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief  Box class
 */
class LArBox
{
public:
    /**
     *  @brief  Constructor
     *
     *  @param  bottom The bottom box corner
     *  @param  top The top box corner
     */
    LArBox(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top);

    /**
     *  @brief  Find box intersection lengths for given ray
     *
     *  @param  ray The ray (starting point and direction)
     *  @param  t0 The first intersection length along ray from its starting point (double precision)
     *  @param  t1 The second intersection length along ray from its starting point (double precision)
     * 
     *  @return Success/failure of finding both intersection lengths
     */
    bool Intersect(const LArRay &ray, double &t0, double &t1) const;

    /**
     *  @brief  Check if the given point is inside the box
     *
     *  @param  point The point (CartesianVector)
     *
     *  @return Success/failure
     */
    bool Inside(const pandora::CartesianVector &point) const;

    pandora::CartesianVector m_bottom; ///< The bottom corner of the box
    pandora::CartesianVector m_top;    ///< The top corner of the box
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArBox::LArBox(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top) : m_bottom(bottom), m_top(top)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

typedef std::array<long, 3> LongBin3Array;
typedef std::array<long, 4> LongBin4Array;

/**
 *  @brief  Grid class
 */
class LArGrid : public LArBox
{
public:
    /**
     *  @brief  Constructor
     *
     *  @param  bottom The bottom starting corner of the grid (CartesianVector)
     *  @param  top The top end corner of the grid (CartesianVector)
     *  @param  binWidths The regular bin widths stored as a CartesianVector(dx, dy, dz)
     */
    LArGrid(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top, const pandora::CartesianVector &binWidths);

    /**
     *  @brief  Get the (x,y,z,total) bin indices for the given point. Need long integers, since total bin can be > 2^31
     *
     *  @param  point The CartesianVector position
     *
     *  @return The array of (x,y,z,total) bin long integer indices
     */
    LongBin4Array GetBinIndices(const pandora::CartesianVector &point) const;

    /**
     *  @brief  Get the position for the given x, y and z bins
     *
     *  @param  xBin The x bin index (long integer)
     *  @param  yBin The y bin index (long integer)
     *  @param  zBin The z bin index (long integer)
     *
     *  @return The CartesianVector position
     */
    pandora::CartesianVector GetPoint(long xBin, long yBin, long zBin) const;

    /**
     *  @brief  Get the position for the given array of bin indices (x,y,z,total)
     *
     *  @param  bins The (x,y,z,total) bins as an array of long integers (since total can be > 2^31)
     *
     *  @return The CartesianVector position
     */
    pandora::CartesianVector GetPoint(const LongBin4Array &bins) const;

    pandora::CartesianVector m_bottom;    ///< The bottom corner of the box
    pandora::CartesianVector m_top;       ///< The top corner of the box
    pandora::CartesianVector m_binWidths; ///< The bin widths (dx, dy, dz)
    LongBin3Array m_nBins;                ///< The (x,y,z) bin indices (long integers)
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArGrid::LArGrid(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top, const pandora::CartesianVector &binWidths) :
    LArBox(bottom, top),
    m_bottom(bottom),
    m_top(top),
    m_binWidths(binWidths),
    m_nBins({0, 0, 0})
{
    if (binWidths.GetX() <= 0 || binWidths.GetY() <= 0 || binWidths.GetZ() <= 0)
        throw pandora::StatusCodeException(pandora::STATUS_CODE_OUT_OF_RANGE);

    const long NxBins = static_cast<long>((top.GetX() - bottom.GetX()) / binWidths.GetX());
    const long NyBins = static_cast<long>((top.GetY() - bottom.GetY()) / binWidths.GetY());
    const long NzBins = static_cast<long>((top.GetZ() - bottom.GetZ()) / binWidths.GetZ());
    m_nBins = {NxBins, NyBins, NzBins};
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline long GetBinIndex(double x, double botX, double binWidth, long maxBin)
{
    long index(0);
    if (binWidth > 0.0)
        index = static_cast<long>(((x - botX) / binWidth));

    if (index < 0)
        index = 0;
    else if (index >= maxBin)
        index = maxBin - 1;

    return index;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline LongBin4Array LArGrid::GetBinIndices(const pandora::CartesianVector &point) const
{
    // Bin widths should always be non-zero. Need to use long integers to avoid
    // integer overflow for the total bin, which can be larger than 2^31
    const long xBin = GetBinIndex(point.GetX(), m_bottom.GetX(), m_binWidths.GetX(), m_nBins[0]);
    const long yBin = GetBinIndex(point.GetY(), m_bottom.GetY(), m_binWidths.GetY(), m_nBins[1]);
    const long zBin = GetBinIndex(point.GetZ(), m_bottom.GetZ(), m_binWidths.GetZ(), m_nBins[2]);

    const long totBin = (zBin * m_nBins[1] + yBin) * m_nBins[0] + xBin;

    LongBin4Array binIndices = {xBin, yBin, zBin, totBin};
    return binIndices;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::CartesianVector LArGrid::GetPoint(long xBin, long yBin, long zBin) const
{
    const float x = m_bottom.GetX() + xBin * m_binWidths.GetX();
    const float y = m_bottom.GetY() + yBin * m_binWidths.GetY();
    const float z = m_bottom.GetZ() + zBin * m_binWidths.GetZ();

    return pandora::CartesianVector(x, y, z);
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::CartesianVector LArGrid::GetPoint(const LongBin4Array &bins) const
{
    return GetPoint(bins[0], bins[1], bins[2]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Create the detector geometry based on the C++ root file
 *
 *  @param  parameters The application parameters
 *  @param  pPrimaryPandora The address of the primary pandora instance
 */
void CreateGeometry(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Process events using the supplied pandora instance
 *
 *  @param  parameters The application parameters
 *  @param  pPrimaryPandora The address of the primary pandora instance
 */
void ProcessEvents(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Create MC particles from the Geant4 trajectories
 *
 *  @param  event The Geant4 event
 *  @param  pPrimaryPandora The address of the primary pandora instance
 *  @param  parameters The application parameters
 *
 *  @return Map of <trackID, energy> for the MC particles
 */
MCParticleEnergyMap CreateMCParticles(const TG4Event &event, const pandora::Pandora *const pPrimaryPandora, const Parameters &parameters);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Convert the GENIE neutrino reaction string to a Nuance-like integer code
 *
 *  @param  reaction The neutrino reaction string
 *
 *  @return The reaction integer code
 */
int GetNuanceCode(const std::string &reaction);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Make voxels from a given TG4HitSegment (a Geant4 energy deposition step)
 *
 *  @param  g4Hit The TG4HitSegment
 *  @param  grid Voxelisation grid
 *  @param  parameters The application parameters
 *
 *  @return vector of LArVoxels
 */
LArVoxelList MakeVoxels(const TG4HitSegment &g4Hit, const LArGrid &grid, const Parameters &parameters);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Combine energies for voxels with the same ID
 *
 *  @param  voxelList The unmerged list (vector) of voxels
 *
 *  @return vector of merged LArVoxels
 */
LArVoxelList MergeSameVoxels(const LArVoxelList &voxelList);

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  Parse the command line arguments, setting the application parameters
 *
 *  @param  argc argument count
 *  @param  argv argument vector
 *  @param  parameters to receive the application parameters
 *
 *  @return success
 */
bool ParseCommandLine(int argc, char *argv[], Parameters &parameters);

/**
 *  @brief  Print the list of configurable options
 *
 *  @return false, to force abort
 */
bool PrintOptions();

/**
 *  @brief  Process view option so that 3x2D and/or 3D hit lists are created
 *
 *  @param  viewOption the view option string
 *  @param  parameters to receive the application parameters
 *
 */
void ProcessViewOption(const std::string &viewOption, Parameters &parameters);

/**
 *  @brief  Process the provided reco option string to perform high-level steering
 *
 *  @param  recoOption the reco option string
 *  @param  parameters to receive the application parameters
 *
 *  @return success
 */
bool ProcessRecoOption(const std::string &recoOption, Parameters &parameters);

/**
 *  @brief  Process list of external, commandline parameters to be passed to specific algorithms
 *
 *  @param  parameters the parameters
 *  @param  pPandora the address of the pandora instance
 */
void ProcessExternalParameters(const Parameters &parameters, const pandora::Pandora *const pPandora);

} // namespace lar_nd_reco

#endif // #ifndef PANDORA_ND_INTERFACE_H
