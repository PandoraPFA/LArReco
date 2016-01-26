/**
 *  @file   LArReco/include/PandoraInterface.h
 * 
 *  @brief  Header file for PandoraInterface.
 * 
 *  $Log: $
 */
#ifndef PANDORA_INTERFACE_H
#define PANDORA_INTERFACE_H 1

#include "LArStitching/MultiPandora.h"

namespace lar_reco_mp
{

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

    std::string     m_pandoraSettingsFile;          ///< The path to the pandora settings file (mandatory parameter)
    std::string     m_whichDetector;                ///< The detector name (default is MicroBooNE)
    int             m_nEventsToProcess;             ///< The number of events to process (default all events in file)
    bool            m_shouldDisplayEventTime;       ///< Whether event times should be calculated and displayed (default false)
    bool            m_shouldDisplayEventNumber;     ///< Whether event numbers should be displayed (default false)
};

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
 *  @brief  Create a new Pandora instance and register lar content algs and plugins
 *
 *  @return the address of the new Pandora instance
 */
const pandora::Pandora *CreateNewPandora();

/**
 *  @brief  Create and configure the primary/stitching pandora instance
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ConfigurePrimaryPandoraInstance(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Create and configure the dune 35t pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ConfigureMicroBooNEPandoraInstance(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Create and configure the dune 35t pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ConfigureDune35tPandoraInstances(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Create and configure the dune 4apa pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ConfigureDune4APAPandoraInstances(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Create and configure the protodune pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ConfigureProtoDunePandoraInstances(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Process events using the supplied pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ProcessEvents(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Set x0 values for all pfos created by the specified pandora isntance
 *
 *  @param  pPandora the address of the relevant pandora instance
 */
pandora::StatusCode SetParticleX0Values(const pandora::Pandora *const pPandora);

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline Parameters::Parameters() :
    m_whichDetector(""),
    m_nEventsToProcess(-1),
    m_shouldDisplayEventTime(false),
    m_shouldDisplayEventNumber(false)
{
}

} // namespace lar_reco_mp

#endif // #ifndef PANDORA_INTERFACE_H
