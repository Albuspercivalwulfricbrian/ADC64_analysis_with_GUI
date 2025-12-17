#ifndef ULTRAFASTHISTOGRAMREADER_H
#define ULTRAFASTHISTOGRAMREADER_H

#include <vector>
#include <functional>
#include <TTree.h>
#include <TString.h>

class UltraFastHistogramReader
{
public:
    UltraFastHistogramReader(TTree *tree, int channel);
    ~UltraFastHistogramReader();

    // Single, main function - all floats
    void readData(std::vector<float> &ampData,
                  std::vector<float> &chargeData,
                  std::vector<float> &timeData,
                  std::function<void(float)> progressCallback = nullptr);

private:
    bool isPeaksInfoTree(TTree *tree, int channel);

    TTree *m_tree;
    int m_channel;
    bool m_isPeaksInfo;
    TString m_formulaAmp;
    TString m_formulaCharge;
    TString m_formulaTime;
};

#endif // ULTRAFASTHISTOGRAMREADER_H