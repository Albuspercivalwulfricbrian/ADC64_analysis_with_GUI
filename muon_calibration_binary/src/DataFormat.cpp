#include "DataFormat.h"

const std::map<uint32_t, int16_t> DataFormat::adcmap =
    {
        // {51232681, 0},
        // {101830393,1},
        {0x0cd97915, 1},
        {0x0f229ac3, 0},
        {0x0f229ad5, 4},
        {0x0f383f3a, 2},
        {0x0f383f49, 3},

};

const int32_t DataFormat::total_channels = DataFormat::adcmap.size() * 64;

void DataFormat::setName(const char *a)
{
    snprintf(fileName, sizeof(fileName), "%s", a);
}
