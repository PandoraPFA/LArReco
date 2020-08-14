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
using namespace pandora;

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
        
        static LArMCParticleHelper::PrimaryParameters CreatePrimaryParameters();
        
        void getPurityAndCompleteness(const pandora::PfoList *const Pfos, const pandora::MCParticleList *const MCParts, const pandora::CaloHitList *const CaloHits, std::vector<std::vector<float>> &pfoPurityCompleteness, LArMCParticleHelper::PrimaryParameters primaryParameters = ParticleHierarchyValidationAlgorithm::CreatePrimaryParameters());
        
        std::vector<float> purityAndCompleteness(const pandora::ParticleFlowObject *const pPfo, const pandora::MCParticleList *const pMCParts, const pandora::CaloHitList *const CaloHits, LArMCParticleHelper::PrimaryParameters &primaryParameters);
        
            //Constructor and Destructor
        ParticleHierarchyValidationAlgorithm();
        ~ParticleHierarchyValidationAlgorithm();
        
    private:
        pandora::StatusCode Run();
        pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);
        void getRelevantPfos(const PfoList *const Pfos, PfoList &relevantPfos, const bool f_addAll);
        void getRelevantMCParts(const MCParticleList *const MCParts, MCParticleList &relevantMCParts, const bool f_addAll);
        void getViewHits(const ParticleFlowObject *const c_Pfo, int &NUHits, int &NVHits, int &NWHits, const bool f_showAllPfoData);
        
        
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
