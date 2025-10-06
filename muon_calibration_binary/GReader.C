
#include <string.h>
#include <iostream>
using namespace std;
#include "FHCalMapper.h"
int main()
{
    try
    {
        FHCalMapper mapper("/home/strizhak/Downloads/fhcal_geo.json");

        // Get channel by ADC serial number and channel
        auto channel = mapper.getChannelInfo(255344442, 32);
        if (channel)
        {
            std::cout << "Found channel: " << channel->ch << " " << std::get<1>(channel->num_geo) << std::endl;
        }

        // Get channel by geometric coordinates
        auto channel_by_geo = mapper.getByGeo(1, 6, 0);
        if (channel_by_geo)
        {
            std::cout << "Channel at geo coords: " << channel_by_geo->ch << std::endl;
        }

        // Get all channels for an ADC
        auto channels = mapper.filterADCchannels(255344442);
        std::cout << "ADC has " << channels.size() << " channels" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}