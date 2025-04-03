#include "DataFileReader.h"
// #include <string>

int main(int argc, char **argv)
{
    DataFileReader DFR;

    // string dir, fname, tempdir;
    if(argc == 2) {    
        argv[1];
        // cout << argv[1] << endl;
        Progress* analysis_process = new Progress(std::string(argv[1]));
        // DFR.setName(argv[1]); 

        DataFileReader DFR;
        DFR.setName(argv[1]); 
        DFR.CreateRootFile();
        // DFR.ConsequentialEventsReading(analysis_process);
        vector<float> a = DFR.DrawAverageWaveform(47000,51000,10000,0);
        for (auto e : a) cout << e << ", ";
        // DFR.SaveRootFile();  


    }
    // else if (argc == 3)
    // {

    // }

    return 0;
}