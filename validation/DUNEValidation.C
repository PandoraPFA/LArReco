#include "Validation.h"

#include "TFile.h"
#include "TObjArray.h"
#include "TDirectory.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

void MakePerformancePlots(const std::string &filename1, const std::string &filename2, const std::string &legend1, const std::string &legend2);

void DUNEValidation(const std::string &inputFile1, const std::string &inputFile2, const std::string &legend1, const std::string &legend2);

void DUNEValidation(const std::string &inputFile1, const std::string &inputFile2, const std::string &legend1, const std::string &legend2)
{
    Parameters parameters;
    parameters.m_histogramOutput = true;

    Validation(inputFile1, parameters);
    TObjArray hList1(0);
    for (auto *obj : *gDirectory->GetList())
        hList1.Add(obj);
    TFile f1("output1.root", "RECREATE");
    hList1.Write();
    f1.Close();

    Validation(inputFile2, parameters);
    TObjArray hList2(0);
    for (auto *obj : *gDirectory->GetList())
        hList2.Add(obj);
    TFile f2("output2.root", "RECREATE");
    hList2.Write();
    f2.Close();

    MakePerformancePlots("output1.root", "output2.root", legend1, legend2);
}

