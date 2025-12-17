#include "UltraFastHistogramReader.h"
#include <iostream>
#include <TROOT.h>
#include <TError.h>
#include <TGraph.h>
#include <algorithm>
#include <cstring> // For memcpy
#include <thread>

UltraFastHistogramReader::UltraFastHistogramReader(TTree *tree, int channel)
    : m_tree(tree), m_channel(channel), m_isPeaksInfo(false)
{
    if (!m_tree)
        return;

    // Determine if it's PeaksInfo or short_energy_ChannelEntry
    m_isPeaksInfo = isPeaksInfoTree(tree, channel);

    if (m_isPeaksInfo)
    {
        m_formulaAmp = TString::Format("channel_%i.amp()", channel + 1);
        m_formulaCharge = TString::Format("channel_%i.charge()", channel + 1);
        m_formulaTime = TString::Format("channel_%i.time()", channel + 1);
    }
    else
    {
        m_formulaAmp = TString::Format("channel_%i.amp", channel + 1);
        m_formulaCharge = TString::Format("channel_%i.charge", channel + 1);
        m_formulaTime = TString::Format("channel_%i.time", channel + 1);
    }
}

UltraFastHistogramReader::~UltraFastHistogramReader()
{
    // Clean up any temporary objects
}

bool UltraFastHistogramReader::isPeaksInfoTree(TTree *tree, int channel)
{
    TBranch *branch = tree->GetBranch(TString::Format("channel_%i", channel + 1));
    if (!branch)
        return false;

    TString className = branch->GetClassName();
    return (className == "PeaksInfo");
}

// Main function - renamed from readDataAllAtOnceMixed
void UltraFastHistogramReader::readData(std::vector<float> &ampData,
                                        std::vector<float> &chargeData,
                                        std::vector<float> &timeData,
                                        std::function<void(float)> progressCallback)
{
    if (!m_tree)
        return;

    Long64_t nentries = m_tree->GetEntries();
    if (nentries <= 0)
        return;

    // Clear and resize
    ampData.resize(nentries);
    chargeData.resize(nentries);
    timeData.resize(nentries);

    // Allocate temporary buffers for Double_t data
    std::vector<Double_t> tempAmp(nentries);
    std::vector<Double_t> tempCharge(nentries);
    std::vector<Double_t> tempTime(nentries);

    // Disable messages
    Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;

    try
    {
        // STEP 1: Reading amplitude data (33% of work)
        if (progressCallback)
        {
            progressCallback(0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "Reading amplitude data from ROOT file..." << std::endl;

        // Read amplitude column
        TString ampFormula = m_isPeaksInfo ? TString::Format("channel_%i.amp()", m_channel + 1) : TString::Format("channel_%i.amp", m_channel + 1);

        m_tree->Draw(ampFormula.Data(), "", "goff", nentries, 0);

        // Copy amplitude data
        Double_t *ampArray = m_tree->GetV1();
        if (!ampArray)
        {
            throw std::runtime_error("Failed to get amplitude array from ROOT");
        }

        std::memcpy(tempAmp.data(), ampArray, nentries * sizeof(Double_t));

        if (progressCallback)
        {
            progressCallback(0.33f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // STEP 2: Reading charge data (66% of work)
        std::cout << "Reading charge data from ROOT file..." << std::endl;

        // Read charge column
        TString chargeFormula = m_isPeaksInfo ? TString::Format("channel_%i.charge()", m_channel + 1) : TString::Format("channel_%i.charge", m_channel + 1);

        m_tree->Draw(chargeFormula.Data(), "", "goff", nentries, 0);

        // Copy charge data
        Double_t *chargeArray = m_tree->GetV1();
        if (!chargeArray)
        {
            throw std::runtime_error("Failed to get charge array from ROOT");
        }

        std::memcpy(tempCharge.data(), chargeArray, nentries * sizeof(Double_t));

        if (progressCallback)
        {
            progressCallback(0.66f);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // STEP 3: Reading time data (100% of work)
        std::cout << "Reading time data from ROOT file..." << std::endl;

        // Read time column
        TString timeFormula = m_isPeaksInfo ? TString::Format("channel_%i.time()", m_channel + 1) : TString::Format("channel_%i.time", m_channel + 1);

        m_tree->Draw(timeFormula.Data(), "", "goff", nentries, 0);

        // Copy time data
        Double_t *timeArray = m_tree->GetV1();
        if (!timeArray)
        {
            throw std::runtime_error("Failed to get time array from ROOT");
        }

        std::memcpy(tempTime.data(), timeArray, nentries * sizeof(Double_t));

        // STEP 4: Convert and copy to output vectors with progress
        std::cout << "Copying data to output vectors..." << std::endl;

        const Long64_t PROGRESS_STEPS = 33;
        const Long64_t entriesPerStep = std::max<Long64_t>(nentries / PROGRESS_STEPS, 1000);

        Long64_t processed = 0;
        float lastProgressSent = 0.66f;

        for (Long64_t i = 0; i < nentries; i++)
        {
            ampData[i] = static_cast<float>(tempAmp[i]);
            chargeData[i] = static_cast<float>(tempCharge[i]);
            timeData[i] = static_cast<float>(tempTime[i]);

            processed++;

            if (processed % entriesPerStep == 0)
            {
                float copyProgress = 0.66f + (0.34f * static_cast<float>(processed) / static_cast<float>(nentries));

                if (copyProgress - lastProgressSent >= 0.01f || i == nentries - 1)
                {
                    if (progressCallback)
                    {
                        progressCallback(copyProgress);
                        lastProgressSent = copyProgress;
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    }
                }
            }
        }

        // Send final update
        if (progressCallback)
        {
            progressCallback(1.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        std::cout << "UltraFastHistogramReader: Successfully processed "
                  << nentries << " entries" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in readData: " << e.what() << std::endl;
        ampData.clear();
        chargeData.clear();
        timeData.clear();
        throw;
    }

    gErrorIgnoreLevel = oldErrorIgnoreLevel;
}