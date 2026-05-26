#ifndef ULTRAFASTHISTOGRAMREADER_H
#define ULTRAFASTHISTOGRAMREADER_H

#include <TTree.h>
#include <TString.h>
#include <vector>
#include <functional>

class UltraFastHistogramReader
{
public:
    UltraFastHistogramReader(TTree *tree, int channel);
    ~UltraFastHistogramReader();

    void readData(std::vector<float> &ampData,
                  std::vector<float> &chargeData,
                  std::vector<float> &timeData,
                  std::vector<float> &oneMinusR2Data, // Добавлен вектор для 1-R2
                  std::function<void(float)> progressCallback,
                  Long64_t maxEntries = -1);

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
