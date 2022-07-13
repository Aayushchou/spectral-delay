/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

//Requires fftw - 3 library to run.

#include "../JuceLibraryCode/JuceHeader.h"
#include "SpectroComponent.h"
#include <fftw3.h>



//==============================================================================
/**
*/
class SpectralDelayAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    SpectralDelayAudioProcessor();
    ~SpectralDelayAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;
    
    const String getParameterText (int index) override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    int getNumParameters() override;
    float getParameter (int index) override;
    void setParameter (int index, float newValue) override;
    const String getParameterName (int index) override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    //Adjustable parameters
    
    float dryMix_;
    float wetMix_;
    float specDelayLength1_;
    float amp_;
    float ph_;
    
    int delayArray[2048];
    int period[2048];
    int ampData[512][512];
    int phData[512][512];
    int fftSelectedSize_;
    int hopSelectedSize_;       
    int windowType_;
    
    enum Parameters
    {
        kspecDelayParam = 0,
        kdryMixParam,
        kwetMixParam,
        kfftSizeParam,
        kHopSizeParam,
        kWindowTypeParam,
        kNumParameters
    };
    
    enum HopSize
    {
        kHopSize1Window = 1,
        kHopSize1_2Window,
        kHopSize1_4Window,
        kHopSize1_8Window
    };
    
    enum Window
    {
        kWindowRectangular = 1,
        kWindowBartlett,
        kWindowHann,
        kWindowHamming
    };
    

    
    ScopedPointer<SpectrogramComponent> spect = new SpectrogramComponent();
    
    // Circular buffer gathers audio samples from the input until enough are available
    // for the FFT calculation
    AudioSampleBuffer inputBuffer_;
    int inputBufferLength_;
    int inputBufferWritePosition_;
    
    // Circular buffer that collects output samples from the FFT overlap-add process
    // before they are ready to be sent to the output stream
    AudioSampleBuffer outputBuffer_;
    int outputBufferLength_;
    int outputBufferReadPosition_, outputBufferWritePosition_;

private:
    // Methods to initialise and de-initialise the FFT machinery
    void initFFT(int length);
    void deinitFFT();
    
    // Methods to initialise and de-initialise the window
    void initWindow(int length, int windowType);
    void deinitWindow();
    
    // Methods to update the buffering for the given hop size and the output scaling
    void updateHopSize();
    void updateScaleFactor();
    
    // Whether the FFT has been initialised and is therefore ready to go
    bool fftInitialised_;
    
    // Variables for calculating the FFT and IFFT: complex data structures and the
    // "plan" used by the fftw library to calculate the transforms.
    fftw_complex *fftTimeDomain_, *fftFrequencyDomain_;
    fftw_plan fftForwardPlan_, fftBackwardPlan_;
    
    // Size of the FFT (generally a power of two) and the hop size (in samples, generally a fraction of FFT size)
    int fftActualTransformSize_;
    int hopActualSize_;
    
    // Amount by which to scale the inverse FFT to return to original amplitude: depends on the
    // transform size (because of fftw implementation) and the hop size (because of inherent overlap)
    double fftScaleFactor_;
    
    
    //Audio buffer to store spetral delay data
    
    AudioSampleBuffer specDelayAmpBuffer;
    int specDelayAmpBufferlength;
    int specDelayAmpWritePos, specDelayAmpReadPos;
    
    AudioSampleBuffer specDelayPhBuffer;
    int specDelayPhBufferlength;
    int specDelayPhWritePos, specDelayPhReadPos;
    

    
    // How many samples since the last FFT?
    int samplesSinceLastFFT_;
    
    // Stored window function for pre-processing input frames
    double *windowBuffer_;
    int windowBufferLength_;
    
    // Whether or not prepareToPlay() has been called, i.e. that resources are in use
    bool preparedToPlay_;
    
    // Spin lock that prevents the FFT settings from changing in the middle of the audio
    // thread.
    SpinLock fftSpinLock_;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralDelayAudioProcessor)
};
