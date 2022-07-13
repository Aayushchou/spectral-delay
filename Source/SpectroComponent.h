/*
 ==============================================================================
 
 This file is part of the JUCE library.
 Copyright (c) 2017 - ROLI Ltd.
 
 JUCE is an open source library subject to commercial or open-source
 licensing.
 
 By using JUCE, you agree to the terms of both the JUCE 5 End-User License
 Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
 27th April 2017).
 
 End User License Agreement: www.juce.com/juce-5-licence
 Privacy Policy: www.juce.com/juce-5-privacy-policy
 
 Or: You may also use this code under the terms of the GPL v3 (see
 www.gnu.org/licenses).
 
 JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 DISCLAIMED.
 
 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"

class SpectrogramComponent : public Component,
private Timer
{
public:
    SpectrogramComponent();
    ~SpectrogramComponent();
    
    void paint (Graphics& g) override;
    
    void timerCallback() override;
    
    void pushNextSampleIntoFifo (float sample) noexcept;
    
    void drawNextLineOfSpectrogram();
    
    enum
    {
        fftOrder = 10,
        fftSize  = 1 << fftOrder
    };
    
    bool nextFFTBlockReady;
    Image spectrogramImage;
    dsp::FFT forwardFFT;
    
    
    float fifo [fftSize];
    float fftData [2 * fftSize];
    int fifoIndex;
    
private:

    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};

