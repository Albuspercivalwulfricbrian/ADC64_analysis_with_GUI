#ifndef HIFO
#define HIFO 1
#include "ChannelEntry.h"
class HitInfo
{
    public:
    Int_t X=-1;
    Int_t Y=-1;
    Int_t Z=-1;
    Float_t charge = 0;
    Int_t amp = 0;
    Float_t time = 0;
    Int_t channel = -100;
    Float_t zl_rms = 999;
    IntegralInfo II;

};

class TrackInfo
{
    public:
    std::vector<HitInfo> hits = {};

    void Reset()
    {
        hits = {};
        hits_reduced = {};
    }
    int GetCurrentTrackSize()
    {
        return hits.size();
    }
    void AddHit(const HitInfo& hit)
    {
        hits.push_back(hit);
    }

    std::vector<HitInfo> GetReduced()
    {
        return hits_reduced;
    }

    void SetReducedCharge(int numb, Float_t energy)
    {
        hits_reduced[numb].charge = energy;
    }
    void SearchTrueTrack(int threshold)
    {
        for (auto p : hits)
        {
            if (    
                (p.time > 2630 && p.time < 2700) && //!Event selection cut on time
                (p.zl_rms < 40) && //!Event selection cut on base line quality

                (p.charge > threshold)
                ) 
                hits_reduced.push_back(p);
        }
        if (hits_reduced.size()>0)    
        {
            for (int i = 0; i < hits_reduced.size()-1; i++)
            {
                int flag = 0;
                for (int j = i+1; j < hits_reduced.size(); j++)
                {
                    if ((abs(hits_reduced[j].X-hits_reduced[i].X)<=1) && (abs(hits_reduced[j].Y-hits_reduced[i].Y)<=1) && (abs(hits_reduced[j].Z-hits_reduced[i].Z)<=1)
                     && (2*abs(hits_reduced[j].charge-hits_reduced[i].charge)/abs(hits_reduced[j].charge+hits_reduced[i].charge)<0.4)
                     )
                    {
                        flag = 1;
                        break;
                    }    
                }
                if (flag == 0) {hits_reduced[i].charge = -1000;}
            }

            hits_reduced.erase(
                std::remove_if(
                    hits_reduced.begin(), 
                    hits_reduced.end(),
                    [](HitInfo const & p) { return p.charge == -1000; }
                ), hits_reduced.end()); 
            Float_t avgTime = 0;
            for (int i = 0; i < hits_reduced.size(); i++) avgTime+=(Float_t)hits_reduced[i].time/(Float_t)hits_reduced.size();
        }  
    }

    bool isVertical() //! simple function to check is muon track is vertical without track reconstruction procedure
    {
        int maxX = -100;
        int minX = 100;
        int maxZ = -100;
        int minZ = 100;
        int maxY = -100;
        int minY = 100;
        Float_t avX = 0;
        Float_t avY = 0;
        Float_t avZ = 0;
        Float_t totalCharge = 0;
        for (int i = 0; i < hits_reduced.size(); i++)
        {
            avX +=(Float_t)(hits_reduced[i].X)*(hits_reduced[i].charge);
            avY +=(Float_t)(hits_reduced[i].Y)*(hits_reduced[i].charge);
            avZ +=(Float_t)(hits_reduced[i].Z)*(hits_reduced[i].charge);
            totalCharge+=hits_reduced[i].charge;
            if (hits_reduced[i].X<minX) minX = hits_reduced[i].X;
            if (hits_reduced[i].X>maxX) maxX = hits_reduced[i].X;
            if (hits_reduced[i].Z<minZ) minZ = hits_reduced[i].Z;
            if (hits_reduced[i].Z>maxZ) maxZ = hits_reduced[i].Z;            
            if (hits_reduced[i].Y<minY) minY = hits_reduced[i].Y;
            if (hits_reduced[i].Y>maxY) maxY = hits_reduced[i].Y;
            
        }
        avX/=totalCharge;
        avY/=totalCharge;
        avZ/=totalCharge;
        avAngle = 0;
        for (int i = 0; i < hits_reduced.size(); i++)
        {
            avAngle += pow((((Float_t)hits_reduced[i].X-avX)*((Float_t)hits_reduced[i].X-avX)+((Float_t)hits_reduced[i].Z-avZ)*((Float_t)hits_reduced[i].Z-avZ))/
            (((Float_t)hits_reduced[i].X-avX)*((Float_t)hits_reduced[i].X-avX)+((Float_t)hits_reduced[i].Z-avZ)*((Float_t)hits_reduced[i].Z-avZ)+((Float_t)hits_reduced[i].Y-avY)*((Float_t)hits_reduced[i].Y-avY))
            ,0.5)*(hits_reduced[i].charge);
        }
        avAngle/=totalCharge;
        int flag = 0;

        if (avAngle <=0.95 && avAngle >= 0.7  ) return 1;
        else return 0;
    }

    Float_t GetAvAngle()
    {
        return avAngle;
    }
    private:
    std::vector<HitInfo> hits_reduced = {}; //!
    Float_t avAngle = 0; //!


};

#endif