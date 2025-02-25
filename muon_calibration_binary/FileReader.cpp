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
        DFR.ConsequentialEventsReading(analysis_process);
        // DFR.SaveRootFile();  


    }
    // else if (argc == 3)
    // {

    // }

    return 0;
}