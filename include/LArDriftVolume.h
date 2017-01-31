/**
 *  @file   LArReco/include/LArDriftVolume.h
 * 
 *  @brief  Header file for LArDriftVolume.
 * 
 *  $Log: $
 */
#ifndef LAR_DRIFT_VOLUME_H
#define LAR_DRIFT_VOLUME_H 1

#include <vector>

namespace lar_reco
{

/**
 *  @brief  drift volume class to hold properties of drift volume
 */
class LArDriftVolume
{
public:
    /**
     *  @brief  Constructor
     *
     *  @param  volumeID         unique ID number
     *  @param  isPositiveDrift  direction of drift 
     *  @param  wirePitchU       wire pitch (U view)
     *  @param  wirePitchV       wire pitch (V view)
     *  @param  wirePitchW       wire pitch (W view)
     *  @param  wireAngleU       wire angle (U view)
     *  @param  wireAngleV       wire angle (V view)
     *  @param  centerX          centre of volume (X)
     *  @param  centerY          centre of volume (Y)
     *  @param  centerZ          centre of volume (Z)
     *  @param  widthX           width of volume (X)
     *  @param  widthY           width of volume (Y)
     *  @param  widthZ           width of volume (Z)
     *  @param  sigmaUVZ         matching between views
     *  @param  tpcVolumeList    input list of TPC volumes
     */
    LArDriftVolume(const unsigned int volumeID, const bool isPositiveDrift,
        const double wirePitchU, const double wirePitchV, const double wirePitchW, const double wireAngleU, const double wireAngleV, 
        const double centerX, const double centerY, const double centerZ, const double widthX, const double widthY, const double widthZ,
        const double sigmaUVZ);

    /**
     *  @brief Return unqiue ID
     */
    unsigned int GetVolumeID() const;  

    /**
     *  @brief Return drift direction (true if positive)
     */
    bool IsPositiveDrift() const;

    /**
     *  @brief Return wire pitch in U view
     */
    double GetWirePitchU() const;

    /**
     *  @brief Return wire pictch in V view
     */
    double GetWirePitchV() const;

    /**
     *  @brief Return wire pitch in W view
     */
    double GetWirePitchW() const;

    /**
     *  @brief Return wire angle in U view (Pandora convention)
     */
    double GetWireAngleU() const;

    /**
     *  @brief Return wire angle in V view (Pandora convention)
     */
    double GetWireAngleV() const;

    /**
     *  @brief Return X position at centre of drift volume
     */
    double GetCenterX() const;

    /**
     *  @brief Return Y position at centre of drift volume
     */
    double GetCenterY() const;

    /**
     *  @brief Return Z position at centre of drift volume
     */
    double GetCenterZ() const;

    /**
     *  @brief Return X span of drift volume
     */
    double GetWidthX() const;

    /**
     *  @brief Return Y span of drift volume
     */
    double GetWidthY() const;

    /**
     *  @brief Return Z span of drift volume
     */
    double GetWidthZ() const;

    /**
     *  @brief Return sigmaUVZ parameter (used for matching views)
     */
    double GetSigmaUVZ() const;

private:
    unsigned int  m_volumeID;
    bool          m_isPositiveDrift;
    double        m_wirePitchU;
    double        m_wirePitchV;
    double        m_wirePitchW;
    double        m_wireAngleU;
    double        m_wireAngleV;
    double        m_centerX;
    double        m_centerY;
    double        m_centerZ;
    double        m_widthX;
    double        m_widthY;
    double        m_widthZ;
    double        m_sigmaUVZ;
};

typedef std::vector<LArDriftVolume> LArDriftVolumeList;

//------------------------------------------------------------------------------------------------------------------------------------------

inline LArDriftVolume::LArDriftVolume(const unsigned int volumeID, const bool isPositiveDrift, const double wirePitchU, const double wirePitchV,
        const double wirePitchW, const double wireAngleU, const double wireAngleV, const double centerX, const double centerY, const double centerZ,
        const double widthX, const double widthY, const double widthZ, const double sigmaUVZ) :
    m_volumeID(volumeID),
    m_isPositiveDrift(isPositiveDrift),
    m_wirePitchU(wirePitchU),
    m_wirePitchV(wirePitchV),
    m_wirePitchW(wirePitchW),
    m_wireAngleU(wireAngleU),
    m_wireAngleV(wireAngleV),
    m_centerX(centerX),
    m_centerY(centerY),
    m_centerZ(centerZ),
    m_widthX(widthX),
    m_widthY(widthY),
    m_widthZ(widthZ),
    m_sigmaUVZ(sigmaUVZ)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline unsigned int LArDriftVolume::GetVolumeID() const
{ 
    return m_volumeID; 
}
   
//------------------------------------------------------------------------------------------------------------------------------------------

inline bool LArDriftVolume::IsPositiveDrift() const
{
    return m_isPositiveDrift;
}
  
//------------------------------------------------------------------------------------------------------------------------------------------      

inline double LArDriftVolume::GetWirePitchU() const
{ 
    return m_wirePitchU; 
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWirePitchV() const
{ 
    return m_wirePitchV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWirePitchW() const
{
    return m_wirePitchW;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWireAngleU() const
{
    return m_wireAngleU;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWireAngleV() const
{
    return m_wireAngleV;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetCenterX() const
{
    return m_centerX;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetCenterY() const
{ 
    return m_centerY;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetCenterZ() const
{
    return m_centerZ;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWidthX() const
{
    return m_widthX;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWidthY() const
{ 
    return m_widthY;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetWidthZ() const
{
    return m_widthZ;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline double LArDriftVolume::GetSigmaUVZ() const
{
    return m_sigmaUVZ;
}

} // namespace lar_reco

#endif // #ifndef LAR_DRIFT_VOLUME_H
