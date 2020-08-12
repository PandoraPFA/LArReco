#include "Pandora/Algorithm.h"
#include "larpandoracontent/LArHelpers/LArClusterHelper.h"
#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"

#include "larpandoracontent/LArHelpers/LArPointingClusterHelper.h"
#include "larpandoracontent/LArObjects/LArPointingCluster.h"

#include "larpandoracontent/LArMonitoring/NeutrinoEventValidationAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArObjects/LArTwoDSlidingFitResult.h"

#include "larpandoracontent/LArHelpers/LArInteractionTypeHelper.h"
#include "larpandoracontent/LArHelpers/LArMonitoringHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"

#include "larpandoracontent/LArMonitoring/EventValidationBaseAlgorithm.h"
#include "larpandoracontent/LArMonitoring/TestBeamEventValidationAlgorithm.h"

using namespace lar_content;

namespace development_area
{
    class ParticleHierarchyValidationAlgorithm : public pandora::Algorithm
    {
    public:
        /**
         *  @brief  Factory class for instantiating algorithm
         */
        class Factory : public pandora::AlgorithmFactory
        {
        public:
            pandora::Algorithm *CreateAlgorithm() const;
        };
        
        static LArMCParticleHelper::PrimaryParameters CreatePrimaryParameters(int i);
        
        std::vector<float> purityAndCompleteness(const pandora::ParticleFlowObject *const pPfo, const pandora::MCParticleList *const pMCParts, const pandora::CaloHitList *const CaloHits, LArMCParticleHelper::PrimaryParameters primaryParameters = ParticleHierarchyValidationAlgorithm::CreatePrimaryParameters(1));
        
            //Constructor and Destructor
        ParticleHierarchyValidationAlgorithm();
        ~ParticleHierarchyValidationAlgorithm();
        
    private:
        pandora::StatusCode Run();
        pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);
        
        std::string m_inputPfoListName;
        std::string m_inputMCParticleListName;
        
            //Tree Member variables
        bool        m_writeToTree;
        std::string m_treeName;
        std::string m_fileName;
              
        int eventNo;
        bool addAll;
        bool m_showAllPfoData;
        
        LArMCParticleHelper::PrimaryParameters m_primaryParameters;
        
        
    }; //PHVA class
    
    inline pandora::Algorithm *ParticleHierarchyValidationAlgorithm::Factory::CreateAlgorithm() const
    {
        return new ParticleHierarchyValidationAlgorithm();
    }
} //namespace development_area
