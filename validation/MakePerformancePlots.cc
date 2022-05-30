#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TRegexp.h"
#include "TKey.h"
#include "TLegend.h"
#include "TString.h"
#include "TStyle.h"


void SetPlotTitle(TH1F* hist, const int particle);
void FormatPlot(TH1F* hist, const int conf);
void FormatPlotWithRange(TH1F* hist, const int conf, const float min, const float max);
void FormatPlotWithRangeXY(TH1F* hist, const int conf, const float xMin, const float xMax, const float yMin, const float yMax);
void SaveToFile(TCanvas* canvas, const int particle, const std::string &name);

void MakePerformancePlots(const std::string &filename1, const std::string &filename2, const std::string &legend1, const std::string &legend2)
{
    TFile *file_v0 = new TFile(filename1.c_str());
    TFile *file_v1 = new TFile(filename2.c_str());

    vector<TFile*> filesVector;
    filesVector.emplace_back(file_v0);
    filesVector.emplace_back(file_v1);

    int nConfigToCompare = filesVector.size(); //Number of configurations to compare!
    std::vector<TString> legendName;
    legendName.emplace_back(legend1);
    legendName.emplace_back(legend2);

    auto legend = new TLegend(0.1,0.9,0.9,0.95);
    legend->SetNColumns(nConfigToCompare);

    // TString regDis = "*DIS*";
    TString regMu = "*MUON*";
    TString regPiplus = "*PIPLUS*";
    TString regPiminus = "*PIMINUS*";
    TString regProton = "*PROTON1*";
    TString regElectron = "*ELECTRON*";
    TString regPhoton = "*PHOTON1*";
    TString regAll = "*ALL*";
    TString regCompleteness = "*Completeness";
    TString regPurity = "*Purity";
    TString regHits = "*HitsEfficiency";
    TString regMomentum = "*MomentumEfficiency";
    TString regAngWithYZ = "*AngleWithYZEfficiency";
    TString regAngInYZ = "*AngleInYZEfficiency";
    TString regXVertex = "*XVertexEfficiency";
    TString regYVertex = "*YVertexEfficiency";
    TString regZVertex = "*ZVertexEfficiency";
    //TString regVertexDeltaR = "ALL_INTERACTIONS_VtxDeltaR";
    
    TRegexp reAll(regAll,kTRUE);
    TRegexp reComp(regCompleteness,kTRUE);
    TRegexp rePur(regPurity,kTRUE);
    TRegexp reHits(regHits,kTRUE);
    TRegexp reMom(regMomentum,kTRUE);
    TRegexp reAngWithYZ(regAngWithYZ,kTRUE);
    TRegexp reAngInYZ(regAngInYZ,kTRUE);
    TRegexp reXVtex(regXVertex,kTRUE);
    TRegexp reYVtex(regYVertex,kTRUE);
    TRegexp reZVtex(regZVertex,kTRUE);
    //TRegexp reVtxDeltaR(regVertexDeltaR,kTRUE);
      
    vector<TString> particleNames;
    particleNames.emplace_back(regMu);
    particleNames.emplace_back(regPiplus);
    particleNames.emplace_back(regPiminus);
    particleNames.emplace_back(regProton);
    particleNames.emplace_back(regElectron);
    particleNames.emplace_back(regPhoton);

    TH1F* hPurity[6][nConfigToCompare];
    TH1F* hCompleteness[6][nConfigToCompare];
    TH1F* hHitsEfficiency[6][nConfigToCompare];
    TH1F* hMomentumEfficiency[6][nConfigToCompare];      
    TH1F* hAngleWithYZEfficiency[6][nConfigToCompare];      
    TH1F* hAngleInYZEfficiency[6][nConfigToCompare];      
    TH1F* hXVertexEfficiency[6][nConfigToCompare];      
    TH1F* hYVertexEfficiency[6][nConfigToCompare];      
    TH1F* hZVertexEfficiency[6][nConfigToCompare];      

    TIter next(filesVector.at(0)->GetListOfKeys());
    TKey *key;
    Int_t i(1);
    while ((key= (TKey*)next()))
    { 
        TString st = key->GetName();
        vector<TH1F*> hTemp;
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
            hTemp.emplace_back((TH1F*)filesVector.at(iConf)->Get(st));
        
        if (st.Index(reAll) == kNPOS)
            continue;
        int iPart(0);

        for(auto partName: particleNames)
        {
            TRegexp rePart(partName,kTRUE);
            //Completeness plots
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reComp) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                {
                    hCompleteness[iPart][iConf]=hTemp[iConf];
                    std::cout << "partName = " << partName << " iConf = " << iConf << " iPart = " << iPart << " entries = " <<
                        hCompleteness[iPart][iConf]->GetEntries() << std::endl;
                }
            }
            
            //Purity plots
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(rePur) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hPurity[iPart][iConf]=hTemp[iConf];
            }
           
            //HitsEfficiency plots
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reHits) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hHitsEfficiency[iPart][iConf]=hTemp[iConf];
            }
           
            //MomentumEfficiency plots            
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reMom) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hMomentumEfficiency[iPart][iConf]=hTemp[iConf];
            }

            //AngleWithYZEfficiency plots            
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reAngWithYZ) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hAngleWithYZEfficiency[iPart][iConf]=hTemp[iConf];
            }

            //AngleInXEfficiency plots            
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reAngInYZ) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hAngleInYZEfficiency[iPart][iConf]=hTemp[iConf];
            }

            //XVertexEfficiency plots            
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reXVtex) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hXVertexEfficiency[iPart][iConf]=hTemp[iConf];
            }

            //XVertexEfficiency plots            
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reYVtex) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hYVertexEfficiency[iPart][iConf]=hTemp[iConf];
            }

            //XVertexEfficiency plots            
            if (!(st.Index(rePart) == kNPOS) && !(st.Index(reZVtex) == kNPOS))
            {
                for(int iConf=0; iConf<nConfigToCompare; iConf++)
                    hZVertexEfficiency[iPart][iConf]=hTemp[iConf];
            }

            iPart++;
        }
    }

    TCanvas *c = new TCanvas();
    //Draw a plot for each particle
    for(int iPart = 0; iPart < 6; iPart++)
    {
        gStyle->SetOptStat(0);
        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();

        //Completeness
        SetPlotTitle(hCompleteness[iPart][0], iPart);
        for(int iConf = 0; iConf < nConfigToCompare; iConf++)
        {
            FormatPlot(hCompleteness[iPart][iConf], iConf);
            hCompleteness[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
            if (iPart == 0)
                legend->AddEntry(hCompleteness[iPart][iConf],legendName[iConf],"l");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "Completeness");
        
        //Purity
        SetPlotTitle(hPurity[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlot(hPurity[iPart][iConf], iConf);
            hPurity[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "Purity");

        //HitsEfficiency
        gPad-> SetLogy(0);
        gPad-> SetLogx(1);
        SetPlotTitle(hHitsEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hHitsEfficiency[iPart][iConf], iConf, 10, 1e4);
            hHitsEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "HitsEfficiency");

        //MomentumEfficiency
        gPad-> SetLogx(0);
        SetPlotTitle(hMomentumEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hMomentumEfficiency[iPart][iConf], iConf, 0, 5);
            hMomentumEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "MomentumEfficiency");

        //AngleWithYZEfficiency
        gPad-> SetLogx(0);
        SetPlotTitle(hAngleWithYZEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hAngleWithYZEfficiency[iPart][iConf], iConf, 0, 180);
            hAngleWithYZEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "AngleWithYZEfficiency");

        //AngleInYZEfficiency
        gPad-> SetLogx(0);
        SetPlotTitle(hAngleInYZEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hAngleInYZEfficiency[iPart][iConf], iConf, 0, 180);
            hAngleInYZEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "AngleInYZEfficiency");

        //XVertexEfficiency
        gPad-> SetLogx(0);
        SetPlotTitle(hXVertexEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hXVertexEfficiency[iPart][iConf], iConf, -400, 400);
            hXVertexEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "XVertexEfficiency");

        //YVertexEfficiency
        gPad-> SetLogx(0);
        SetPlotTitle(hYVertexEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hYVertexEfficiency[iPart][iConf], iConf, -700, 700);
            hYVertexEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "YVertexEfficiency");

        //ZVertexEfficiency
        gPad-> SetLogx(0);
        SetPlotTitle(hZVertexEfficiency[iPart][0], iPart);
        for(int iConf=0; iConf<nConfigToCompare; iConf++)
        {
            FormatPlotWithRange(hZVertexEfficiency[iPart][iConf], iConf, -100, 1500);
            hZVertexEfficiency[iPart][iConf]->Draw(iConf > 0 ? "p same" : "p");
        }

        legend->Draw("same");
        SaveToFile(c, iPart, "ZVertexEfficiency");
    }
}

void SetPlotTitle(TH1F* hist, const int particle)
{
    switch(particle)
    {
        case 0:
            hist->SetTitle("Leading muon");
            break;
        case 1:
            hist->SetTitle("Leading pi+");
            break;
        case 2:
            hist->SetTitle("Leading pi-");
            break;
        case 3:
            hist->SetTitle("Leading proton");
            break;
        case 4:
            hist->SetTitle("Leading electron");
            break;
        case 5:
            hist->SetTitle("Leading photon");
            break;
        default:
            break;
    }
}

void FormatPlot(TH1F* hist, const int conf)
{
    hist->SetLineColor(1 + conf);
    hist->SetMarkerStyle(8);
    hist->SetMarkerSize(0.6);
    hist->SetMarkerColor(1 + conf);
    hist->SetLineWidth(2);
}

void FormatPlotWithRange(TH1F* hist, const int conf, const float min, const float max)
{
    FormatPlot(hist, conf);
    hist->GetXaxis()->SetRangeUser(min, max);
}

void FormatPlotWithRangeXY(TH1F* hist, const int conf, const float xMin, const float xMax, const float yMin, const float yMax)
{
    FormatPlotWithRange(hist, conf, xMin, xMax);
    hist->GetYaxis()->SetRangeUser(yMin, yMax);
}

void SaveToFile(TCanvas* canvas, const int particle, const std::string &name)
{
    for (std::string type : { "pdf", "eps" })
    {
        mkdir(type.c_str(), 0755);
        switch (particle)
        {
            case 0:
                canvas->Print((type + "/LeadingMuon_" + name + "." + type).c_str());
                break;
            case 1:
                canvas->Print((type + "/LeadingPiplus_" + name + "." + type).c_str());
                break;
            case 2:
                canvas->Print((type + "/LeadingPiminus_" + name + "." + type).c_str());
                break;
            case 3:
                canvas->Print((type + "/LeadingProton_" + name + "." + type).c_str());
                break;
            case 4:
                canvas->Print((type + "/LeadingElectron_" + name + "." + type).c_str());
                break;
            case 5:
                canvas->Print((type + "/LeadingPhoton_" + name + "." + type).c_str());
                break;
            default:
                break;
        }
    }
}

