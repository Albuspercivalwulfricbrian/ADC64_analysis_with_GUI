#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TString.h>
#include "ChannelEntry.h"
#include <string.h>
#include "muon_struct.h"
#include "Map.h"
#include <PSDmuonTracker.h>
#include <TPolyLine3D.h>
#include <Math/Vector3D.h>
#include <TGraph.h>
#define CreateReducedTree 1

using namespace std;
using namespace MAP_H;
using namespace ROOT::Math;

// function Object to be minimized
Double_t mypow(Double_t *x, Double_t *par)
{
    Double_t c = par[0]*pow((exp(1)*par[1]),par[2]);
    return c*pow(exp(-par[1]/x[0])/x[0],par[2]);
}

int main(int argc, char** argv)
{
    TString TrackCalculationMode = "EigenVector";

    if(argc == 3) {
        argv[1];
        argv[2];
        // argv[4];
    }

    TF1* mypow1 = new TF1("mypow1",mypow,.1, 10., 3);
    mypow1->SetParNames("Ymax","Xmax","Power");
    Float_t  Ymax,  Xmax,   R1, R2, FitMin, FitMax, Xadd;

    CreateMap();

	TString source_path = (TString)argv[1];
    TString run_name = (TString)argv[2];

    const Int_t total_channels = 128;

    TFile *source_file = TFile::Open(source_path+"tr_tracks.root");
    TTree *source_tree = (TTree*)source_file->Get("Tracks");
    TFile *result_root = new TFile (source_path+"track_pics.root", "RECREATE");
    TrackInfo *trackinfo = new TrackInfo;
    source_tree->SetBranchAddress("TrackInfo", &trackinfo);    

    TrackInfo trackreduced;
    Float_t AvAngle, ZenithAngle, AzimuthAngle;
    TTree *tracktree = new TTree ("Tracks","Tracks");
    tracktree->Branch("TrackInfo", "TrackInfo",&trackreduced);
    tracktree->Branch("AvAngle", &AvAngle, "AvAngle/F");
    tracktree->Branch("ZenithAngle", &ZenithAngle, "ZenithAngle/F");
    tracktree->Branch("AzimuthAngle", &AzimuthAngle, "AzimuthAngle/F");

    TCanvas *canvas[9];
    for (int i = 0; i < 9; i++)
    {
        canvas[i] = new TCanvas(Form("c_%i",i),Form("c_%i",i));
        canvas[i]->Divide(4,4);
    }

    TH1F *histall[128];
    TH1F *histall1[128];

    for (int i = 1; i <= 128; i++)
    {
        histall[i-1] = new TH1F(Form("ch_%i_x=%i_y=%i_z=%i",i,Map[i][0],Map[i][1],Map[i][2]),Form("ch_%i_x=%i_y=%i_z=%i",i,Map[i][0],Map[i][1],Map[i][2]),70,100,40000);
        histall1[i-1] = new TH1F(Form("before_ch_%i_x=%i_y=%i_z=%i",i,Map[i][0],Map[i][1],Map[i][2]),Form("before_ch_%i_x=%i_y=%i_z=%i",i,Map[i][0],Map[i][1],Map[i][2]),70,100,40000);
        histall1[i-1]->SetLineColor(1);
    }   

    TH1F *histall13d[3][6][7];
    TH1F *histall3d[3][6][7];

    for (int i = 1; i <= 128; i++)
    {
        if (i!=64 && i!=128)
        {
            histall3d[Map[i][0]-1][Map[i][1]-1][Map[i][2]-1] = histall[i-1];
            histall13d[Map[i][0]-1][Map[i][1]-1][Map[i][2]-1] = histall1[i-1];
        }
    }   

    Int_t total_entries = source_tree->GetEntries()/1;
    time_t start_time = time(NULL);

    for (int EvNum = 0; EvNum < total_entries; EvNum++)
    {
        trackinfo->Reset();
        trackreduced.Reset();
        source_tree->GetEntry(EvNum);
        int track_size = trackinfo->GetCurrentTrackSize();
        trackinfo->SearchTrueTrack(1500);
        int reduced_size = trackinfo->GetReduced().size();
        if (reduced_size!=0)
        {
            bool isVertical = trackinfo->isVertical();
            if (reduced_size >=4)
            {
                MuonTracker MTracker(trackinfo);
                // MTracker.SetDebugMode(1);
                MTracker.SetZenithVector(0,1,0);
                MTracker.SetAzimuthVector(1,0,0);
                MTracker.SetTrackCalcMode(TrackCalculationMode);
                MTracker.CalculateTrack();
                if (MTracker.GetTrackZenithAngle() > 0.1 && MTracker.GetTrackZenithAngle() < 1.57)
                {
                    for (int i = 0; i< reduced_size; i++)
                    {
                        histall[trackinfo->GetReduced()[i].channel-1]->Fill(trackinfo->GetReduced()[i].charge);
                    }
                }

                    MTracker.CalculateChargeStraightened();
                    auto kkk = MTracker.GetChargeStraightened();
                    MTracker.GetCalibratedCharge(trackinfo);
                if (MTracker.GetTrackZenithAngle() > 0.1 && MTracker.GetTrackZenithAngle() < 1.57
                 )
                {                
                    for (int i = 0; i< reduced_size; i++)
                    {
                        histall1[trackinfo->GetReduced()[i].channel-1]->Fill(trackinfo->GetReduced()[i].charge);       
                    }                    
                }
                    trackreduced.hits = trackinfo->GetReduced();
                    AvAngle = trackinfo->GetAvAngle();
                    ZenithAngle = MTracker.GetTrackZenithAngle();
                    AzimuthAngle = MTracker.GetTrackAzimuthAngle();
                    tracktree->Fill();
            }

        }
        else {};
    }

    for (int x = 0; x < 3; x++)
    {
        for (int y = 0; y < 6; y++)
        {   
            Float_t peaks[7] = {0.};
            Float_t zpos[7] = {0.};
            for (int z = 0; z < 7; z++)
            {
                zpos[z] = z+1;
                canvas[3*x+y/2]->cd(z+1+8*(y%2));
                Ymax =  (Float_t)histall13d[x][y][z]->GetMaximum();
                Xmax =  (Float_t)histall13d[x][y][z]->GetBinCenter(histall13d[x][y][z]->GetMaximumBin());
                FitMin = Xmax-1500;
                FitMax = 2.*Xmax;
                R2= 1.35*Xmax; 
                R1= 0.65*Xmax;
                mypow1->SetParameters(Ymax,Xmax,10.);
                mypow1->SetParLimits(1,R1,R2);
                histall3d[x][y][z]->Draw();
                histall13d[x][y][z]->Draw("same");
                histall13d[x][y][z]->Fit(mypow1,"Q","",FitMin,FitMax);

                peaks[z] = histall13d[x][y][z] -> GetFunction("mypow1")->GetParameter(1);
            }
            canvas[3*x+y/2]->cd(7+8*(y%2)+1);
            TGraph *gr = new TGraph(7,zpos,peaks);
            gr->Draw("AP*");
            gr->GetYaxis()->SetRangeUser(0,1.1*TMath::MaxElement(7,gr->GetY()));
        }
    }

    for (int i = 0; i < 9; i++) canvas[i]->Write();
    tracktree->Write();
    result_root->Close();
}