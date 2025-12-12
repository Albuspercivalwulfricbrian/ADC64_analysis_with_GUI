#include "UltraFastHistogramReader.h"
#include <iostream>
#include <TROOT.h>
#include <TError.h>
#include <TGraph.h>
#include <algorithm>
#include <cstring>  // For memcpy
#include <thread>

UltraFastHistogramReader::UltraFastHistogramReader(TTree* tree, int channel)
    : m_tree(tree), m_channel(channel), m_isPeaksInfo(false)
{
    if (!m_tree) return;
    
    // Determine if it's PeaksInfo or short_energy_ChannelEntry
    m_isPeaksInfo = isPeaksInfoTree(tree, channel);
    
    if (m_isPeaksInfo) {
        m_formulaAmp = TString::Format("channel_%i.amp()", channel + 1);
        m_formulaCharge = TString::Format("channel_%i.charge()", channel + 1);
        m_formulaTime = TString::Format("channel_%i.time()", channel + 1);
    } else {
        m_formulaAmp = TString::Format("channel_%i.amp", channel + 1);
        m_formulaCharge = TString::Format("channel_%i.charge", channel + 1);
        m_formulaTime = TString::Format("channel_%i.time", channel + 1);
    }
}

UltraFastHistogramReader::~UltraFastHistogramReader()
{
    // Clean up any temporary objects
}

bool UltraFastHistogramReader::isPeaksInfoTree(TTree* tree, int channel)
{
    TBranch* branch = tree->GetBranch(TString::Format("channel_%i", channel + 1));
    if (!branch) return false;
    
    TString className = branch->GetClassName();
    return (className == "PeaksInfo");
}

void UltraFastHistogramReader::readDataAllAtOnce(std::vector<float>& ampData, 
                                                std::vector<float>& chargeData,
                                                std::vector<float>& timeData,
                                                std::function<void(float)> progressCallback)
{
    if (!m_tree) return;
    
    Long64_t nentries = m_tree->GetEntries();
    if (nentries <= 0) return;
    
    // Clear and resize
    ampData.resize(nentries);
    chargeData.resize(nentries);
    timeData.resize(nentries);
    
    // Create formula for all three columns at once
    TString formulaAll;
    if (m_isPeaksInfo) {
        formulaAll = TString::Format("channel_%i.amp():channel_%i.charge():channel_%i.time()", 
                                     m_channel + 1, m_channel + 1, m_channel + 1);
    } else {
        formulaAll = TString::Format("channel_%i.amp:channel_%i.charge:channel_%i.time", 
                                     m_channel + 1, m_channel + 1, m_channel + 1);
    }
    
    // Disable messages
    Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;
    
    try {
        // Draw ALL THREE columns at once - ONE I/O operation!
        m_tree->Draw(formulaAll.Data(), "", "goff", nentries, 0);
        
        // Get arrays (V1, V2, V3 contain our data)
        Double_t* ampArray = m_tree->GetV1();
        Double_t* chargeArray = m_tree->GetV2();
        Double_t* timeArray = m_tree->GetV3();
        
        if (ampArray && chargeArray && timeArray) {
            // Fast parallel copy
            #pragma omp parallel for
            for (Long64_t i = 0; i < nentries; i++) {
                ampData[i] = static_cast<float>(ampArray[i]);
                chargeData[i] = static_cast<float>(chargeArray[i]);
                timeData[i] = static_cast<float>(timeArray[i]);
            }
        }
        
        if (progressCallback) progressCallback(1.0f);
        
        std::cout << "UltraFastHistogramReader: Processed " 
                  << nentries << " entries" << std::endl;
        
    } catch (...) {
        std::cerr << "Error in readDataAllAtOnce" << std::endl;
        ampData.clear();
        chargeData.clear();
        timeData.clear();
    }
    
    gErrorIgnoreLevel = oldErrorIgnoreLevel;
}
void UltraFastHistogramReader::readDataAllAtOnceMixed(std::vector<uint32_t>& ampData, 
                                                     std::vector<float>& chargeData,
                                                     std::vector<float>& timeData,
                                                     std::function<void(float)> progressCallback)
{
    if (!m_tree) return;
    
    Long64_t nentries = m_tree->GetEntries();
    if (nentries <= 0) return;
    
    // Clear and resize
    ampData.resize(nentries);
    chargeData.resize(nentries);
    timeData.resize(nentries);
    
    // Create formula for all three columns at once
    TString formulaAll;
    if (m_isPeaksInfo) {
        formulaAll = TString::Format("channel_%i.amp():channel_%i.charge():channel_%i.time()", 
                                     m_channel + 1, m_channel + 1, m_channel + 1);
    } else {
        formulaAll = TString::Format("channel_%i.amp:channel_%i.charge:channel_%i.time", 
                                     m_channel + 1, m_channel + 1, m_channel + 1);
    }
    
    // Disable messages
    Int_t oldErrorIgnoreLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kFatal;
    
    try {
        // STEP 1: Reading data (20% of work)
        if (progressCallback) {
            progressCallback(0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay
        }
        
        std::cout << "Reading data from ROOT file..." << std::endl;
        m_tree->Draw(formulaAll.Data(), "", "goff", nentries, 0);
        
        // Send progress update after reading
        if (progressCallback) {
            progressCallback(0.2f);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Get arrays
        Double_t* ampArray = m_tree->GetV1();
        Double_t* chargeArray = m_tree->GetV2();
        Double_t* timeArray = m_tree->GetV3();
        
        if (!ampArray || !chargeArray || !timeArray) {
            throw std::runtime_error("Failed to get data arrays from ROOT");
        }
        
        // STEP 2: Copying data (80% of work)
        std::cout << "Copying data to vectors..." << std::endl;
        
        // Calculate optimal chunk size - update progress every 1% of copying
        const Long64_t PROGRESS_STEPS = 100;
        const Long64_t entriesPerStep = std::max<Long64_t>(nentries / PROGRESS_STEPS, 1000);
        
        Long64_t processed = 0;
        float lastProgressSent = 0.2f; // Start from 20%
        
        for (Long64_t i = 0; i < nentries; i++) {
            ampData[i] = static_cast<uint32_t>(ampArray[i]);
            chargeData[i] = static_cast<float>(chargeArray[i]);
            timeData[i] = static_cast<float>(timeArray[i]);
            
            processed++;
            
            // Update progress every entriesPerStep entries
            if (processed % entriesPerStep == 0) {
                float copyProgress = 0.2f + (0.8f * static_cast<float>(processed) / static_cast<float>(nentries));
                
                // Only send update if progress has changed significantly (1% or more)
                if (copyProgress - lastProgressSent >= 0.01f || i == nentries - 1) {
                    if (progressCallback) {
                        progressCallback(copyProgress);
                        lastProgressSent = copyProgress;
                        
                        // Small delay to allow GUI to update
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                }
            }
        }
        
        // Send final update
        if (progressCallback) {
            progressCallback(1.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        
        std::cout << "UltraFastHistogramReader: Successfully processed " 
                  << nentries << " entries" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in readDataAllAtOnceMixed: " << e.what() << std::endl;
        ampData.clear();
        chargeData.clear();
        timeData.clear();
        throw;
    }
    
    gErrorIgnoreLevel = oldErrorIgnoreLevel;
}