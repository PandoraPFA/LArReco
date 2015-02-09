/**
 *  @file   TestPandora/src/PandoraInterface.cc
 *
 *  @brief  Implementation for the pandora interface binary
 *
 *  $Log: $
 */

// External Pandora includes
#include "Api/PandoraApi.h"

#include "LArContent.h"

#include "MicroBooNEPseudoLayerPlugin.h"
#include "MicroBooNETransformationPlugin.h"
#include "MicroBooNELegacyTransformationPlugin.h"

#include "LBNE35tPseudoLayerPlugin.h"
#include "LBNE35tTransformationPlugin.h"

#include "LBNE4APAPseudoLayerPlugin.h"
#include "LBNE4APATransformationPlugin.h"

#ifdef MONITORING
#include "TApplication.h"
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/time.h>

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

//------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        // Parse command line parameters
        Parameters parameters;

        if (!ParseCommandLine(argc, argv, parameters))
            return 1;
#ifdef MONITORING
        TApplication *pTApplication = new TApplication("MyTest", &argc, argv);
        pTApplication->SetReturnFromRun(kTRUE);
#endif
        // Construct pandora instance
        pandora::Pandora *pPandora = new pandora::Pandora();

        PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::RegisterAlgorithms(*pPandora));
        PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::RegisterBasicPlugins(*pPandora));

        if ("uboone" == parameters.m_whichDetector)
        {
            std::cout << " Loading plugins for MicroBooNE detector " << std::endl;
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandora,
                new lar_pandora::MicroBooNEPseudoLayerPlugin));
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandora,
              new lar_pandora::MicroBooNELegacyTransformationPlugin));
              //new lar_pandora::MicroBooNETransformationPlugin));
        }
        else if ("lbne35tShort" == parameters.m_whichDetector)
        {
            std::cout << " Loading plugins for LBNE35t detector (short drift volume)" << std::endl;
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandora,
                new lar_pandora::LBNE35tPseudoLayerPlugin));
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandora,
                new lar_pandora::LBNE35tTransformationPlugin(true)));
        }
        else if ("lbne35tLong" == parameters.m_whichDetector)
        {
            std::cout << " Loading plugins for LBNE35t detector (long drift volume)" << std::endl;
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandora,
                new lar_pandora::LBNE35tPseudoLayerPlugin));
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandora,
                new lar_pandora::LBNE35tTransformationPlugin(false)));
        }
        else if ("lbne4apaLeft" == parameters.m_whichDetector)
        {
            std::cout << " Loading plugins for LBNE4APA detector (left drift volume)" << std::endl;
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandora,
                new lar_pandora::LBNE4APAPseudoLayerPlugin));
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandora,
                new lar_pandora::LBNE4APATransformationPlugin(true)));
        }
        else if ("lbne4apaRight" == parameters.m_whichDetector)
        {
            std::cout << " Loading plugins for LBNE4APA detector (right drift volume)" << std::endl;
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArPseudoLayerPlugin(*pPandora,
                new lar_pandora::LBNE4APAPseudoLayerPlugin));
            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, LArContent::SetLArTransformationPlugin(*pPandora,
                new lar_pandora::LBNE4APATransformationPlugin(false)));
        }
        else
        {
            std::cout << " Not a valid detector (options: uboone, lbne35tLong, lbne35tShort, lbne4apaLeft, lbne4apaRight)" << std::endl << " Exiting" << std::endl;
            return 1;
        }

        // Read in pandora settings from config file
        PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::ReadSettings(*pPandora, parameters.m_pandoraSettingsFile));

        // Process the events
        int nEvents(0);

        while ((nEvents++ < parameters.m_nEventsToProcess) || (0 > parameters.m_nEventsToProcess))
        {
            struct timeval startTime, endTime;

            if (parameters.m_shouldDisplayEventNumber)
            {
                std::cout << std::endl << "   PROCESSING EVENT: " << (nEvents - 1) << std::endl << std::endl;
            }

            if (parameters.m_shouldDisplayEventTime)
            {
                (void) gettimeofday(&startTime, NULL);
            }

            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::ProcessEvent(*pPandora));

            if (parameters.m_shouldDisplayEventTime)
            {
                (void) gettimeofday(&endTime, NULL);
                std::cout << "Event time " << (endTime.tv_sec + (endTime.tv_usec / 1.e6) - startTime.tv_sec - (startTime.tv_usec / 1.e6)) << std::endl;
            }

            PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*pPandora));
        }

        delete pPandora;
    }
    catch (pandora::StatusCodeException &statusCodeException)
    {
        std::cerr << "Pandora Exception caught: " << statusCodeException.ToString() << std::endl;
        return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool ParseCommandLine(int argc, char *argv[], Parameters &parameters)
{
    int c(0);

    while ((c = getopt(argc, argv, "i:d:n:t::N::h")) != -1)
    {
        switch (c)
        {
        case 'i':
            parameters.m_pandoraSettingsFile = optarg;
            break;
        case 'd':
            parameters.m_whichDetector = optarg;
            break;
        case 'n':
            parameters.m_nEventsToProcess = atoi(optarg);
            break;
        case 't':
            parameters.m_shouldDisplayEventTime = true;
            break;
        case 'N':
            parameters.m_shouldDisplayEventNumber = true;
            break;
        case 'h':
        default:
            std::cout << std::endl << "./bin/PandoraInterface " << std::endl
                      << "    -i PandoraSettings.xml  (mandatory)" << std::endl
                      << "    -d WhichDetector        (optional)" << std::endl
                      << "    -n NEventsToProcess     (optional)" << std::endl
                      << "    -N                      (optional, display event numbers)" << std::endl
                      << "    -t                      (optional, display event times)" << std::endl << std::endl;
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

Parameters::Parameters() :
    m_whichDetector("uboone"),
    m_nEventsToProcess(-1),
    m_shouldDisplayEventTime(false),
    m_shouldDisplayEventNumber(false)
{
}
