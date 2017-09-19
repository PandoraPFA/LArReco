/**
 *  @file   LArReco/include/PandoraInterface.h
 * 
 *  @brief  Header file for PandoraInterface.
 * 
 *  $Log: $
 */
#ifndef PANDORA_INTERFACE_H
#define PANDORA_INTERFACE_H 1

namespace pandora {class Pandora;}

//------------------------------------------------------------------------------------------------------------------------------------------

namespace lar_reco
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

    std::string     m_settingsFile;                 ///< The path to the pandora settings file (mandatory parameter)
    std::string     m_eventFileNameList;            ///< Colon-separated list of file names to be processed
    std::string     m_geometryFileName;             ///< Name of the file containing geometry information
    std::string     m_stitchingSettingsFile;        ///< The path to the stitching settings file (required only if multiple drift volumes)

    int             m_nEventsToProcess;             ///< The number of events to process (default all events in file)
    int             m_nEventsToSkip;                ///< The number of events to skip
    int             m_nDriftVolumes;                ///< The number of drift volumes
    bool            m_uniqueInstanceSettings;       ///< Whether to enable unique configuration of each Pandora instance
    bool            m_shouldDisplayEventNumber;     ///< Whether event numbers should be displayed (default false)

    bool            m_shouldRunAllHitsCosmicReco;   ///< Whether to run all hits cosmic-ray reconstruction
    bool            m_shouldRunCosmicHitRemoval;    ///< Whether to remove hits from tagged cosmic-rays
    bool            m_shouldRunSlicing;             ///< Whether to slice events into separate regions for processing
    bool            m_shouldRunNeutrinoRecoOption;  ///< Whether to run neutrino reconstruction for each slice
    bool            m_shouldRunCosmicRecoOption;    ///< Whether to run cosmic-ray reconstruction for each slice
    bool            m_shouldIdentifyNeutrinoSlice;  ///< Whether to identify most appropriate neutrino slice
    bool            m_printOverallRecoStatus;       ///< Whether to print current operation status messages
};

/**
 *  @brief  Create pandora instances
 * 
 *  @param  parameters the parameters
 *  @param  pPrimaryPandora to receive the address of the primary pandora instance
 */
void CreatePandoraInstances(const Parameters &parameters, const pandora::Pandora *&pPrimaryPandora);

/**
 *  @brief  Process events using the supplied pandora instances
 *
 *  @param  parameters the application parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void ProcessEvents(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Create primary pandora instance
 *
 *  @param  parameters the parameters
 *  @param  pPrimaryPandora to receive the address of the primary pandora instance
 */
void CreatePrimaryPandoraInstance(const Parameters &parameters, const pandora::Pandora *&pPrimaryPandora);

/**
 *  @brief  Create daughter pandora instances
 *
 *  @param  parameters the parameters
 *  @param  pPrimaryPandora the address of the primary pandora instance
 */
void CreateDaughterPandoraInstances(const Parameters &parameters, const pandora::Pandora *const pPrimaryPandora);

/**
 *  @brief  Create a new Pandora instance and register lar content algs and plugins
 *
 *  @return the address of the new Pandora instance
 */
const pandora::Pandora *CreateNewPandora();

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
 *  @param  volumeIdString the volume id string, if any
 */
void ProcessExternalParameters(const Parameters &parameters, const pandora::Pandora *const pPandora, const std::string volumeIdString = "");

/**
 *  @brief  Replace all instances of a specified "search" with a "replacement", in a given "subject" string
 *
 *  @param  subject the subject string
 *  @param  search the search string
 *  @param  replacement the replacement string
 *
 *  @return the modified subject
 */
std::string ReplaceString(std::string subject, const std::string &search, const std::string &replacement);

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline Parameters::Parameters() :
    m_settingsFile(""),
    m_eventFileNameList(""),
    m_geometryFileName(""),
    m_stitchingSettingsFile(""),
    m_nEventsToProcess(-1),
    m_nEventsToSkip(0),
    m_nDriftVolumes(1),
    m_uniqueInstanceSettings(false),
    m_shouldDisplayEventNumber(false),
    m_shouldRunAllHitsCosmicReco(true),
    m_shouldRunCosmicHitRemoval(true),
    m_shouldRunSlicing(true),
    m_shouldRunNeutrinoRecoOption(true),
    m_shouldRunCosmicRecoOption(true),
    m_shouldIdentifyNeutrinoSlice(true),
    m_printOverallRecoStatus(false)
{
}

} // namespace lar_reco

#endif // #ifndef PANDORA_INTERFACE_H
