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
// #include "ChannelMap/ChannelMap/include/ChannelMap.h"
// #include "Map.h"
#include "ctime"
// #include "FHCalMapper.h"
#include "DataFormat.h"
using namespace std;

int main(int argc, char **argv)
{
    TString source_path = (TString)argv[1];
    TString run_name = (TString)argv[2];
    // FHCalMapper mapper("/home/strizhak/Downloads/fhcal_geo.json");

    // const Int_t total_channels = 320;
    // const Int_t total_channels = mapper.getADCSerialList().size() * 64;
    // CreateMap();
    TFile *source_file = TFile::Open(source_path + run_name);
    TCanvas *canv;
}