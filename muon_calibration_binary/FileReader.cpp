#include "BinaryDataStructs.h"
#include <string>

int main(int argc, char **argv)
{
    string dir, fname, tempdir;
    if(argc == 4) {
        // dir = string(argv[1]);
        // tempdir = string(argv[2]);
        // fname = string(argv[3]);        
        argv[1];
        argv[2];
        argv[3];

        // argv[2];
        // argv[4];
    }
    // DataFileReader DFR(argv[1]);
    cout << argv[1] << " " << argv[2] <<" " <<argv[3] << endl;
    DataFileReader DFR(argv[1], argv[2], argv[3]);
    
    DFR.ConsequentialEventsReading();
    return 0;
}