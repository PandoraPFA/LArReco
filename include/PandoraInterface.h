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

typedef std::map<int, double> MCParticleEnergyMap;

/**
 *  @brief  Parameters class
 */
class Parameters
{
public:
    /**
   *  @brief Default constructor
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

    pandora::InputInt m_nEventsToSkip; ///< The number of events to skip

    bool m_use3D;     ///< Create 3D LArCaloHits
    bool m_useLArTPC; ///< Create LArTPC LArCaloHits with u,v,w views

    double m_voxelWidth; ///< Voxel box width (cm)
};

/**
 *  @brief  LArVoxel class
 */
class LArVoxel
{
public:
    /**
   *  @brief Default constructor
   */
    LArVoxel(long voxelID, double energyInVoxel, const pandora::CartesianVector &voxelPosVect, int trackID);

    LArVoxel(const LArVoxel &rhs);
    void setEnergy(double E)
    {
        m_energyInVoxel = E;
    }

    long m_voxelID;                          ///< The ID of the voxel
    double m_energyInVoxel;                  ///< The energy in the voxel
    pandora::CartesianVector m_voxelPosVect; ///< position vector (x,y,z) of voxel
    int m_trackID;                           ///< The ID for contributing track to this hit segment, and thus voxel
};

/**
 * @brief Ray class
 */
class LArRay
{
public:
    /**
     * @brief Default constructor
     */
    LArRay(const pandora::CartesianVector &origin, const pandora::CartesianVector &dir);

    LArRay(const LArRay &ray);

    void updateOrigin(const pandora::CartesianVector &newOrigin)
    {
        m_origin = newOrigin;
    }

    pandora::CartesianVector getPoint(double length) const; ///< Pos along ray from start point

    pandora::CartesianVector m_origin; ///< Starting point of the ray
    pandora::CartesianVector m_dir;    ///< Ray direction
    pandora::CartesianVector m_invDir; ///< Reciprocal direction vector
    std::array<int, 3> m_sign;         ///< sign of reciprocal direction vector components
};

/**
 * @brief Box class
 */
class LArBox
{
public:
    /**
     * @brief Default constructor
     */
    LArBox(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top);

    LArBox(const LArBox &rhs);

    bool intersect(const LArRay &ray, double &t0, double &t1) const;
    bool inside(const pandora::CartesianVector &point) const;

    pandora::CartesianVector m_bottom; // The bottom corner of the box
    pandora::CartesianVector m_top;    // The top corner of the box
};

/**
 * @brief Grid class
 */
class LArGrid : public LArBox
{
public:
    /**
     * @brief Default constructor
     */
    LArGrid(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top, const pandora::CartesianVector &binWidths);

    LArGrid(const LArGrid &rhs);

    std::array<long, 4> getBinIndices(const pandora::CartesianVector &point) const;

    pandora::CartesianVector getPoint(long xBin, long yBin, long zBin) const;
    pandora::CartesianVector getPoint(const std::array<long, 4> &bins) const;

    pandora::CartesianVector m_bottom; // The bottom corner of the box
    pandora::CartesianVector m_top;    // The top corner of the box
    pandora::CartesianVector m_binWidths;
    float m_binExtent; // magnitude of m_binWidths vector
    std::array<long, 3> m_nBins;
};

/**
 *  @brief  Create the detector geometry based on the C++ root file
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void CreateGeometry(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Process events using the supplied pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ProcessEvents(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief Create MC particles from the Geant4 trajectories
 *
 *  @param event The Geant4 event
 *  @param pPrimaryPandora the address of the primary pandora instance
 *
 *  @return Map of <trackID, energy> for the MC particles
 */
MCParticleEnergyMap CreateMCParticles(const TG4Event &event, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Make voxels from TG4HitSegments (Geant4 hits)
 *
 *  @param  g4Hit the TG4HitSegment
 *  @param  voxelWidth the voxel box width (cm)
 *
 *  @return vector of LArVoxels
 */
std::vector<LArVoxel> MakeVoxels(const TG4HitSegment &g4Hit, const LArGrid &grid);

/**
 * @brief Combine energies for voxels with the same ID
 *
 *  @param voxelList The list of voxels (which can be updated)
 *
 *  @return vector of merged LArVoxels
 */
std::vector<LArVoxel> MergeSameVoxels(const std::vector<LArVoxel> &voxelList);

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

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline Parameters::Parameters() :
    m_settingsFile(""),
    m_inputFileName(""),
    m_nEventsToProcess(-1),
    m_shouldDisplayEventNumber(false),
    m_use3D(true),
    m_useLArTPC(false),
    m_voxelWidth(0.4)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArVoxel::LArVoxel(long voxelID, double energyInVoxel, const pandora::CartesianVector &voxelPosVect, int trackID) :
    m_voxelID(voxelID),
    m_energyInVoxel(energyInVoxel),
    m_voxelPosVect(voxelPosVect),
    m_trackID(trackID)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArVoxel::LArVoxel(const LArVoxel &rhs) :
    m_voxelID(rhs.m_voxelID),
    m_energyInVoxel(rhs.m_energyInVoxel),
    m_voxelPosVect(rhs.m_voxelPosVect),
    m_trackID(rhs.m_trackID)
{
}

inline LArRay::LArRay(const pandora::CartesianVector &origin, const pandora::CartesianVector &dir) :
    m_origin(origin),
    m_dir(dir),
    m_invDir({0, 0, 0}),
    m_sign({0, 0, 0})
{
    float dx = m_dir.GetX();
    float dy = m_dir.GetY();
    float dz = m_dir.GetZ();

    // Initalise inverse direction values to maximum numerical limit (i.e. parallel lines)
    const float maxVal = std::numeric_limits<float>::max();
    float invdx(maxVal), invdy(maxVal), invdz(maxVal);
    if (fabs(dx) > 0.0)
    {
        invdx = 1.0 / dx;
    }
    if (fabs(dy) > 0.0)
    {
        invdy = 1.0 / dy;
    }
    if (fabs(dz) > 0.0)
    {
        invdz = 1.0 / dz;
    }
    m_invDir.SetValues(invdx, invdy, invdz);

    m_sign[0] = (m_invDir.GetX() < 0);
    m_sign[1] = (m_invDir.GetY() < 0);
    m_sign[2] = (m_invDir.GetZ() < 0);
}

inline LArRay::LArRay(const LArRay &rhs) :
    m_origin(rhs.m_origin),
    m_dir(rhs.m_dir),
    m_invDir(rhs.m_invDir),
    m_sign({rhs.m_sign[0], rhs.m_sign[1], rhs.m_sign[2]})
{
}

inline pandora::CartesianVector LArRay::getPoint(double length) const
{
    pandora::CartesianVector point = m_origin + m_dir * length;
    return point;
}

inline LArBox::LArBox(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top) : m_bottom(bottom), m_top(top)
{
}

inline LArBox::LArBox(const LArBox &rhs) : m_bottom(rhs.m_bottom), m_top(rhs.m_top)
{
}

inline LArGrid::LArGrid(const pandora::CartesianVector &bottom, const pandora::CartesianVector &top, const pandora::CartesianVector &binWidths) :
    LArBox(bottom, top),
    m_bottom(bottom),
    m_top(top),
    m_binWidths(binWidths),
    m_binExtent(binWidths.GetMagnitude()),
    m_nBins({0, 0, 0})
{
    long NxBins = long((top.GetX() - bottom.GetX()) / binWidths.GetX());
    long NyBins = long((top.GetY() - bottom.GetY()) / binWidths.GetY());
    long NzBins = long((top.GetZ() - bottom.GetZ()) / binWidths.GetZ());
    std::cout << "Grid: Nx = " << NxBins << ", Ny = " << NyBins << ", Nz = " << NzBins << std::endl;
    m_nBins = {NxBins, NyBins, NzBins};
}

inline LArGrid::LArGrid(const LArGrid &rhs) :
    LArBox(rhs),
    m_bottom(rhs.m_bottom),
    m_top(rhs.m_top),
    m_binWidths(rhs.m_binWidths),
    m_binExtent(rhs.m_binExtent),
    m_nBins(rhs.m_nBins)
{
}

inline long getBinIndex(double x, double botX, double binWidth, long maxBin)
{
    long index(0);
    if (binWidth > 0.0)
        index = (x - botX) / binWidth;

    if (index < 0)
        index = 0;
    else if (index >= maxBin)
        index = maxBin - 1;

    return index;
}

inline std::array<long, 4> LArGrid::getBinIndices(const pandora::CartesianVector &point) const
{
    // Bin widths should always be non-zero. Need to use long integers to avoid
    // integer overflow for the total bin, which can be larger than 2^31
    long xBin = getBinIndex(point.GetX(), m_bottom.GetX(), m_binWidths.GetX(), m_nBins[0]);
    long yBin = getBinIndex(point.GetY(), m_bottom.GetY(), m_binWidths.GetY(), m_nBins[1]);
    long zBin = getBinIndex(point.GetZ(), m_bottom.GetZ(), m_binWidths.GetZ(), m_nBins[2]);

    long totBin = (zBin * m_nBins[1] + yBin) * m_nBins[0] + xBin;

    std::array<long, 4> binIndices = {xBin, yBin, zBin, totBin};
    return binIndices;
}

inline pandora::CartesianVector LArGrid::getPoint(long xBin, long yBin, long zBin) const
{
    float x = m_bottom.GetX() + xBin * m_binWidths.GetX();
    float y = m_bottom.GetY() + yBin * m_binWidths.GetY();
    float z = m_bottom.GetZ() + zBin * m_binWidths.GetZ();

    return pandora::CartesianVector(x, y, z);
}

inline pandora::CartesianVector LArGrid::getPoint(const std::array<long, 4> &bins) const
{
    return getPoint(bins[0], bins[1], bins[2]);
}

} // namespace lar_nd_reco

#endif // #ifndef PANDORA_ND_INTERFACE_H
