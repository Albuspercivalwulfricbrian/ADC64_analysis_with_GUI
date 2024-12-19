#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TString.h>
#include "ChannelEntry.h"
#include <string.h>
#include "muon_struct.h"
#include "ChannelMap/ChannelMap/include/ChannelMap.h"
#include "Map.h"
#include "ctime"

using namespace std;
using namespace MAP_H;
int main(int argc, char** argv)
{
    if(argc == 3) {
        argv[1];
        argv[2];
        // argv[4];
    }
	TString source_path = (TString)argv[1];
    TString run_name = (TString)argv[2];
    const Int_t total_channels = 128;
    CreateMap();
    TFile *source_file = TFile::Open(source_path+run_name);
    TTree *source_tree = (TTree*)source_file->Get("adc64_data");
    std::array<short_energy_ChannelEntry*, total_channels> short_channel_info;
    for(Int_t ch = 0; ch < total_channels; ch++)
    {

        short_channel_info[ch] = new short_energy_ChannelEntry();
        short_channel_info[ch]->Initialize();
        source_tree->SetBranchAddress((TString::Format("channel_%i", ch+1)).Data(), &short_channel_info[ch]);
    }

    TrackInfo trackinfo;
    // TFile *combined_root = new TFile (source_path+run_name+"_tracks.root", "RECREATE");
    TFile *combined_root = new TFile (source_path+"tr_tracks.root", "RECREATE");
    TTree *tracktree = new TTree ("Tracks","Tracks");
    Int_t total_entries = source_tree->GetEntries();
    tracktree->Branch("TrackInfo", "TrackInfo",&trackinfo);
    time_t start_time = time(NULL);

    for (int i = 0; i < source_tree->GetEntries(); i++)
    {
        trackinfo.Reset();
        source_tree->GetEntry(i);

        for (int ch = 0; ch < total_channels; ch++)
        {
            if (short_channel_info[ch]->amp > 100 && short_channel_info[ch]->amp < 60000
                 && short_channel_info[ch]->charge > 1000 
                && ch !=63 && ch!=127
                )
            {
                HitInfo hitinfo;
                hitinfo.II = short_channel_info[ch]->II;
                hitinfo.charge = short_channel_info[ch]->charge;
                hitinfo.amp = short_channel_info[ch]->amp;
                
                hitinfo.time = short_channel_info[ch]->time;
                hitinfo.channel = ch+1;
                hitinfo.X = Map[ch+1][0];
                hitinfo.Y = Map[ch+1][1];
                hitinfo.Z = Map[ch+1][2];
                
                hitinfo.zl_rms = short_channel_info[ch]->zl_rms;
                trackinfo.AddHit(hitinfo);                
            }
        }
        if (trackinfo.GetCurrentTrackSize()!=0) tracktree->Fill();
        if (i%1000 == 0)
        {
            std::cout<< u8"\033[2J\033[1;1H"; std::cout << (Float_t)i/(Float_t)total_entries*100 << "%" << std::endl;
            time_t time_left = (time(NULL)-start_time)*(float)(total_entries-i)/(float)(i);

            std::cout << " time left: ";
            if (time_left/3600 > 0) cout << time_left/3600 <<"h ";
            if ((time_left%3600)/60 > 0 || time_left/3600 == 0) cout << (time_left%3600)/60 << "m ";
            cout << (time_left%3600)%60<< "s " <<std::endl;
        }
    }
    tracktree->Write();
    combined_root->Close();
    return 0;
}