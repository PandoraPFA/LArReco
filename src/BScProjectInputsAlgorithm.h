/**
 *  @file   larpandoracontent/LArPersistency/BScProjectInputsAlgorithm.h
 *
 *  @brief  Header file for the BSc project inputs algorithm class.
 *
 *  $Log: $
 */
#ifndef LAR_BSC_PROJECT_INPUTS_ALGORITHM_H
#define LAR_BSC_PROJECT_INPUTS_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

#include <fstream>

namespace pandora {class FileWriter;}

//------------------------------------------------------------------------------------------------------------------------------------------

namespace bsc_project
{

/**
 *  @brief  BScProjectInputsAlgorithm class
 */
class BScProjectInputsAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Default constructor
     */
    BScProjectInputsAlgorithm();

    /**
     *  @brief  Destructor
     */
    virtual ~BScProjectInputsAlgorithm();

    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

private:
    pandora::StatusCode Run();

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);
    
    std::string m_inputCaloHitListName;
    std::string m_inputCaloHitListNameW;
    std::string m_outputFilename;
    std::ofstream m_file;

    const std::string m_interactions[31] = {
        "CCQEL_MU_P",
        "CCRES_MU_P_PIPLUS",
        "CCRES_MU_PIPLUS",
        "CCRES_MU_P_PIZERO",
        "CCRES_MU_P",
        "CCRES_MU",
        "CCRES_MU_P_P",
        "CCRES_MU_PIZERO",
        "CCRES_MU_P_P_P",
        "CCRES_MU_P_P_PIPLUS",
        "CCQEL_MU_P_P",
        "CCCOH",
        "CCRES_MU_P_P_P_P",
        "CCRES_MU_P_P_PIZERO",
        "CCQEL_MU_P_P_P",
        "CCQEL_E_P",
        "CCRES_MU_P_PHOTON",
        "CCRES_E_PIPLUS",
        "CCRES_MU_P_P_P_PIZERO",
        "CCRES_MU_P_P_P_P_P",
        "CCRES_E_P_PIPLUS",
        "CCRES_E_P",
        "CCQEL_MU_P_P_P_P",
        "CCRES_MU_PHOTON",
        "CCRES_MU_P_P_P_PIPLUS",
        "CCRES_MU_P_P_P_PHOTON",
        "CCRES_E_P_PHOTON",
        "CCRES_MU_P_P_P_P_P_PIZERO",
        "CCRES_E",
        "CCRES_MU_P_P_P_P_PIZERO",
        "CCRES_E_P_PIZERO"
    };
};

inline pandora::Algorithm *BScProjectInputsAlgorithm::Factory::CreateAlgorithm() const
{
    return new BScProjectInputsAlgorithm();
}

} // namespace lar_content

#endif // #ifndef LAR_BSC_PROJECT_INPUTS_ALGORITHM_H
