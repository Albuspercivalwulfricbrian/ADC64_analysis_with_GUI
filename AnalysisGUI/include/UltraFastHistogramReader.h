#ifndef ULTRAFASTHISTOGRAMREADER_H
#define ULTRAFASTHISTOGRAMREADER_H

#include <vector>
#include <functional>
#include <TTree.h>
#include <TString.h>

class UltraFastHistogramReader {
public:
    UltraFastHistogramReader(TTree* tree, int channel);
    ~UltraFastHistogramReader();
    
    // Version 1: All float vectors
    void readDataAllAtOnce(std::vector<float>& ampData, 
                          std::vector<float>& chargeData,
                          std::vector<float>& timeData,
                          std::function<void(float)> progressCallback = nullptr);
    
    // Version 2: Mixed types (amplitude as uint32_t)
    void readDataAllAtOnceMixed(std::vector<uint32_t>& ampData, 
                               std::vector<float>& chargeData,
                               std::vector<float>& timeData,
                               std::function<void(float)> progressCallback = nullptr);
    
    static bool isPeaksInfoTree(TTree* tree, int channel);
    
private:
    TTree* m_tree;
    int m_channel;
    TString m_formulaAmp;
    TString m_formulaCharge;
    TString m_formulaTime;
    bool m_isPeaksInfo;
};

#endif // ULTRAFASTHISTOGRAMREADER_H