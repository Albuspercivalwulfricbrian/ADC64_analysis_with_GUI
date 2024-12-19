#include "json.hpp"
#include "iostream"
#include "fstream"
#include "vector"
#include "sstream"
#include "iomanip"
#include "filesystem"
// #include "string"
// #include "string_view"
// #include "format"


// for convenience
using json = nlohmann::json;

using namespace std;
int main(int argc, char *argv[])
{
    json ChannelMap;
    json ChannelInfoArray;
    json ChannelInfo;

    fstream inFile;
    string path = std::filesystem::path{argv[0]}.parent_path()/"";
    cout << path << endl;
    inFile.open(path+"ChannelMap.txt");
    std::ofstream o(path+"ChannelMap.json");

    string line;
    while(getline(inFile, line))
    {
        int counter = 0;
        std::istringstream iss(line);
        int number = 0;
        int connector = 0;
        //cout << line << endl;
        while(iss >> number)
        {

            counter++;
            if (counter == 1) {connector = number; continue;}

            ChannelInfo["channel"] = number;
            ChannelInfo["connector"] = connector;
            ChannelInfo["x3"] = counter-1;
            ChannelInfo["x2"] = (connector-1)/3+1;
            ChannelInfo["x1"] = (connector-1)%3+1;

            ChannelInfoArray[std::to_string(number)] = ChannelInfo;

        }
        //cout << << std::setw(4) << ChannelMap << std::endl;
    }
    ChannelMap["channels"] = ChannelInfoArray;
    o << std::setw(4) << ChannelMap << std::endl;


    return 0;
}
