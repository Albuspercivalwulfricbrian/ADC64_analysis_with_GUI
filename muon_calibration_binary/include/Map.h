/* 
    @file   Map.h
    @brief  File to set module geometry. Using ChannelMap json geometry file generation will be possible
*/
#ifndef MAP_H0
#define MAP_H0
#include <iostream>
#include <vector>
#include <cstdint>
// #pragma once
namespace MAP_H
{
    const int32_t ADCnumber = 2;

    int32_t Map[64*ADCnumber+1][3] = {0};

    inline std::vector<std::vector<int32_t>> mapz {{33, 41, 34, 42, 35, 43, 36},            // 4
                            {44, 37, 45, 38, 46, 39, 47},           // 5
                            {40, 48, 1, 9, 2, 10, 3}, 
                            {11, 4, 12, 5, 13, 6, 14},            // 4
                            {7, 15, 8, 16, 17, 25, 18},           // 5
                            {26, 19, 27, 20, 28, 21, 29},         // 6
                            {22, 30, 23, 31, 24, 32, 49},         // 7
                            {57, 50, 58, 51, 59, 52, 60},         // 8
                            {53, 61, 54, 62, 55, 63, 56},


                            {97, 105, 98, 106, 99, 107, 100},            // 4
                            {108, 101, 109, 102, 110, 103, 111},           // 5
                            {104, 112, 65, 73, 66, 74, 67}, 
                            {75, 68, 76, 69, 77, 70, 78},            // 4
                            {71, 79, 72, 80, 81, 89, 82},           // 5
                            {90, 83, 91, 84, 92, 85, 93},         // 6
                            {86, 94, 87, 95, 88, 96, 113},         
                            {121, 114, 122, 115, 123, 116, 124},        
                            {117, 125, 118, 126, 119, 127, 120}
                            };

                            

    inline std::vector<std::vector<int32_t>> mapxy  {
                                    {1, 1},
                                    {2, 1},
                                    {3, 1},
                                    {1, 2},
                                    {2, 2},
                                    {3, 2},
                                    {1, 3},
                                    {2, 3},
                                    {3, 3},

                                    {1, 4},
                                    {2, 4},
                                    {3, 4},
                                    {1, 5},
                                    {2, 5},
                                    {3, 5},
                                    {1, 6},
                                    {2, 6},
                                    {3, 6}
                                };

    void CreateMap()
    {
        for (int32_t i = 0; i < 64*ADCnumber+1; i++) 
        {
            for (int32_t j = 0; j < 3; j++) Map[i][j] = 0;
        }
        cout << mapxy.size() << endl;
        for (int32_t i = 0; i < mapxy.size(); i++)
        {
            int32_t xx = mapxy[i][0];
            int32_t yy = mapxy[i][1];

            for (int32_t j = 0; j < 7; j++)
            {
                int32_t zz = j+1;
                int32_t chch = mapz[i][j];
                Map[chch][0] = xx;
                Map[chch][1] = yy;
                Map[chch][2] = zz; 
                if (yy==1) std::cout << "X = " << xx << "; Y = " << yy << "; Z =  " << zz << "; Ch = " << chch << " : ";
            }
            std::cout << std::endl;
        }
    }
}
#endif MAP_H0