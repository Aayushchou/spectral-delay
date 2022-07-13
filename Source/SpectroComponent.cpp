/*
  ==============================================================================

    SpectroComponent.cpp
    Created: 20 Mar 2018 10:27:09am
    Author:  Aayush Choudhury

  ==============================================================================
*/

#include "PluginProcessor.h"



SpectrogramComponent:: SpectrogramComponent()
: forwardFFT (fftOrder),
spectrogramImage (Image::RGB, 512, 512, true),
fifoIndex (0),
nextFFTBlockReady (false)
{
    setOpaque (true);
    startTimerHz (60);
    setSize (700, 500);
}

SpectrogramComponent::~SpectrogramComponent()
{
}


void SpectrogramComponent:: paint (Graphics& g)
{
    g.fillAll (Colours::black);
    
    g.setOpacity (1.0f);
    g.drawImage (spectrogramImage, getLocalBounds().toFloat());
}

void SpectrogramComponent:: timerCallback()
{
    if (nextFFTBlockReady)
    {
        drawNextLineOfSpectrogram();
        nextFFTBlockReady = false;
        repaint();
    }
}

void SpectrogramComponent:: pushNextSampleIntoFifo (float sample) noexcept
{
    // if the fifo contains enough data, set a flag to say
    // that the next line should now be rendered..
    if (fifoIndex == fftSize)
    {
        if (! nextFFTBlockReady)
        {
            zeromem (fftData, sizeof (fftData));
            memcpy (fftData, fifo, sizeof (fifo));
            nextFFTBlockReady = true;
        }
        
        fifoIndex = 0;
    }
    
    fifo[fifoIndex++] = sample;
}

void SpectrogramComponent:: drawNextLineOfSpectrogram()
{
    const int rightHandEdge = spectrogramImage.getWidth() - 1;
    const int imageHeight = spectrogramImage.getHeight();
    
    // first, shuffle our image leftwards by 1 pixel..
    spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);
    
    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);
    
    // find the range of values produced, so we can scale our rendering to
    // show up the detail clearly
    Range<float> maxLevel = FloatVectorOperations::findMinAndMax (fftData, fftSize / 2);
    
    for (int y = 1; y < imageHeight; ++y)
    {
        const float skewedProportionY = 1.0f - std::exp (std::log (y / (float) imageHeight) * 0.2f);
        const int fftDataIndex = jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
        const float level = jmap (fftData[fftDataIndex], 0.0f, jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
        
        spectrogramImage.setPixelAt (rightHandEdge, y, Colour::fromHSV (level, 1.0f, level, 1.0f));
    }
}
