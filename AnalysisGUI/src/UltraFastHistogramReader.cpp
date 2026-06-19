#include "UltraFastHistogramReader.h"
#include <iostream>
#include <TROOT.h>
#include <TError.h>
#include <algorithm>
#include <stdexcept>
#include "thread"
UltraFastHistogramReader::UltraFastHistogramReader(TTree *tree, int channel)
    : m_tree(tree), m_channel(channel), m_isPeaksInfo(false)
{
    if (!m_tree)
        return;
    m_isPeaksInfo = isPeaksInfoTree(tree, channel);
}

UltraFastHistogramReader::~UltraFastHistogramReader() {}

bool UltraFastHistogramReader::isPeaksInfoTree(TTree *tree, int channel)
{
    TBranch *branch = tree->GetBranch(TString::Format("channel_%i", channel + 1));
    if (!branch)
        return false;
    TString className = branch->GetClassName();
    return (className == "PeaksInfo");
}
void UltraFastHistogramReader::readData(std::vector<float> &ampData,
                                        std::vector<float> &chargeData,
                                        std::vector<float> &timeData,
                                        std::vector<float> &oneMinusR2Data,
                                        std::function<void(float)> progressCallback,
                                        Long64_t maxEntries)
{
    if (!m_tree)
        return;

    Long64_t actualTotalEntries = m_tree->GetEntries();
    Long64_t entriesToProcess = actualTotalEntries;

    if (maxEntries > 0 && maxEntries < actualTotalEntries)
    {
        entriesToProcess = maxEntries;
    }
    if (entriesToProcess <= 0)
        return;

    // Clear and pre-allocate
    ampData.clear();
    chargeData.clear();
    timeData.clear();
    oneMinusR2Data.clear();

    ampData.reserve(entriesToProcess);
    chargeData.reserve(entriesToProcess);
    timeData.reserve(entriesToProcess);
    oneMinusR2Data.reserve(entriesToProcess);

    Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;

    // Process in batches for progress reporting
    Long64_t batchSize = 50000;
    Long64_t processedEntries = 0;

    TString ampFormula = m_isPeaksInfo ? TString::Format("channel_%i.amp()", m_channel + 1) : TString::Format("channel_%i.amp", m_channel + 1);
    TString timeFormula = m_isPeaksInfo ? TString::Format("channel_%i.time()", m_channel + 1) : TString::Format("channel_%i.time", m_channel + 1);
    TString chargeFormula = m_isPeaksInfo ? TString::Format("channel_%i.charge()", m_channel + 1) : TString::Format("channel_%i.charge", m_channel + 1);
    TString r2Formula = TString::Format("channel_%i.FP.r2", m_channel + 1);

    while (processedEntries < entriesToProcess)
    {
        Long64_t currentBatchSize = std::min(batchSize, entriesToProcess - processedEntries);

        // Read amplitude
        m_tree->Draw(ampFormula.Data(), "", "goff", currentBatchSize, processedEntries);
        if (m_tree->GetSelectedRows() < currentBatchSize)
            throw std::runtime_error("ROOT read error (Amp)");

        size_t currentSize = ampData.size();
        ampData.resize(currentSize + currentBatchSize);
        std::copy(m_tree->GetV1(), m_tree->GetV1() + currentBatchSize, ampData.begin() + currentSize);

        // Read time
        m_tree->Draw(timeFormula.Data(), "", "goff", currentBatchSize, processedEntries);
        if (m_tree->GetSelectedRows() < currentBatchSize)
            throw std::runtime_error("ROOT read error (Time)");

        currentSize = timeData.size();
        timeData.resize(currentSize + currentBatchSize);
        std::copy(m_tree->GetV1(), m_tree->GetV1() + currentBatchSize, timeData.begin() + currentSize);

        // Read charge (SEPARATE, like original working code)
        m_tree->Draw(chargeFormula.Data(), "", "goff", currentBatchSize, processedEntries);
        if (m_tree->GetSelectedRows() < currentBatchSize)
            throw std::runtime_error("ROOT read error (Charge)");

        currentSize = chargeData.size();
        chargeData.resize(currentSize + currentBatchSize);
        std::copy(m_tree->GetV1(), m_tree->GetV1() + currentBatchSize, chargeData.begin() + currentSize);

        // Read R2 (SEPARATE, just like amp/time/charge)
        m_tree->Draw(r2Formula.Data(), "", "goff", currentBatchSize, processedEntries);
        if (m_tree->GetSelectedRows() < currentBatchSize)
            throw std::runtime_error("ROOT read error (R2)");

        currentSize = oneMinusR2Data.size();
        oneMinusR2Data.resize(currentSize + currentBatchSize);
        for (Long64_t i = 0; i < currentBatchSize; ++i)
        {
            oneMinusR2Data[currentSize + i] = 1.0f - static_cast<float>(m_tree->GetV1()[i]);
        }

        processedEntries += currentBatchSize;

        if (progressCallback)
        {
            float progress = static_cast<float>(processedEntries) / entriesToProcess;
            progressCallback(progress);
        }
    }

    gErrorIgnoreLevel = oldErrorIgnoreLevel;
}