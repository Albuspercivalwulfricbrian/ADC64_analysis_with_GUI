#include "UltraFastHistogramReader.h"
#include <iostream>
#include <TROOT.h>
#include <TError.h>
#include <algorithm>
#include <stdexcept>

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

    m_tree->SetEstimate(entriesToProcess);

    ampData.resize(entriesToProcess);
    chargeData.resize(entriesToProcess);
    timeData.resize(entriesToProcess);
    oneMinusR2Data.resize(entriesToProcess);

    Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;

    try
    {
        // STEP 1: Reading amplitude
        if (progressCallback)
            progressCallback(0.0f);
        TString ampFormula = m_isPeaksInfo ? TString::Format("channel_%i.amp()", m_channel + 1) : TString::Format("channel_%i.amp", m_channel + 1);
        m_tree->Draw(ampFormula.Data(), "", "goff", entriesToProcess, 0);
        if (m_tree->GetSelectedRows() < entriesToProcess)
            throw std::runtime_error("ROOT read error (Amp)");
        std::copy(m_tree->GetV1(), m_tree->GetV1() + entriesToProcess, ampData.begin());

        // STEP 2: Reading time
        if (progressCallback)
            progressCallback(0.25f);
        TString timeFormula = m_isPeaksInfo ? TString::Format("channel_%i.time()", m_channel + 1) : TString::Format("channel_%i.time", m_channel + 1);
        m_tree->Draw(timeFormula.Data(), "", "goff", entriesToProcess, 0);
        if (m_tree->GetSelectedRows() < entriesToProcess)
            throw std::runtime_error("ROOT read error (Time)");
        std::copy(m_tree->GetV1(), m_tree->GetV1() + entriesToProcess, timeData.begin());

        // STEP 3: Reading Charge and R2 simultaneously using 2D TTree::Draw ("Y:X")
        if (progressCallback)
            progressCallback(0.50f);
        TString chargeVar = m_isPeaksInfo ? TString::Format("channel_%i.charge()", m_channel + 1) : TString::Format("channel_%i.charge", m_channel + 1);
        TString r2Var = TString::Format("channel_%i.FP.r2", m_channel + 1);
        TString combinedFormula = TString::Format("%s:%s", r2Var.Data(), chargeVar.Data());

        m_tree->Draw(combinedFormula.Data(), "", "goff", entriesToProcess, 0);
        if (m_tree->GetSelectedRows() < entriesToProcess)
            throw std::runtime_error("ROOT read error (Charge/R2)");

        Double_t *r2Array = m_tree->GetV1();
        Double_t *chargeArray = m_tree->GetV2();

        for (Long64_t i = 0; i < entriesToProcess; ++i)
        {
            chargeData[i] = static_cast<float>(chargeArray[i]);
            oneMinusR2Data[i] = 1.0f - static_cast<float>(r2Array[i]);
        }

        if (progressCallback)
            progressCallback(1.0f);
    }
    catch (...)
    {
        gErrorIgnoreLevel = oldErrorIgnoreLevel;
        throw;
    }
    gErrorIgnoreLevel = oldErrorIgnoreLevel;
}
