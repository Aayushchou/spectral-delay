/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>

//==============================================================================
SpectralDelayAudioProcessor::SpectralDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ), inputBuffer_(2, 1), outputBuffer_(2, 1)
#endif
{
    // Set default values:
    
    specDelayLength1_ = 1.79;
    specDelayAmpBufferlength = 10000;
    specDelayPhBufferlength = 10000;
    dryMix_ = 1;
    wetMix_ = 0.8;
    
    fftSelectedSize_ = 512;
    hopSelectedSize_ = kHopSize1_4Window;
    windowType_ = kWindowHann;
    
    fftInitialised_ = false;
    fftActualTransformSize_ = 0;
    inputBufferLength_ = 1;
    outputBufferLength_ = 1;
    inputBufferWritePosition_ = outputBufferWritePosition_ = outputBufferReadPosition_ = 0;
    samplesSinceLastFFT_ = 0;
    windowBuffer_ = 0;
    windowBufferLength_ = 0;
    preparedToPlay_ = false;
    fftScaleFactor_ = 0.0;
    
    specDelayAmpReadPos = 0;
    specDelayAmpWritePos = 0;
    
    specDelayPhReadPos = 0;
    specDelayPhWritePos = 0;
    

    
}

SpectralDelayAudioProcessor::~SpectralDelayAudioProcessor()
{
    //Release spectrogram resources.
    
    
    // Release FFT resources if allocated. This should be handled by
    // releaseResources() but in the event it doesn't happen, this avoids
    // a leak. Harmless to call it twice.
    deinitFFT();
    deinitWindow();
}

//==============================================================================
const String SpectralDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectralDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectralDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectralDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectralDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectralDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectralDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

const String SpectralDelayAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

void SpectralDelayAudioProcessor::setCurrentProgram (int index)
{
}

const String SpectralDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectralDelayAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SpectralDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    initFFT(fftSelectedSize_);
    initWindow(fftSelectedSize_, windowType_);
    preparedToPlay_ = true;
    
    // Set values of the spectral delay buffer length based on the sample 
    
    specDelayAmpBufferlength = 10000;
    if(specDelayAmpBufferlength < 1)
        specDelayAmpBufferlength = 1;
    specDelayAmpBuffer.setSize(2, specDelayAmpBufferlength);
    specDelayAmpBuffer.clear();
    
    specDelayPhBufferlength = 10000;
    if(specDelayPhBufferlength < 1)
        specDelayPhBufferlength = 1;
    specDelayPhBuffer.setSize(2, specDelayPhBufferlength);
    specDelayPhBuffer.clear();
    
    // offset should be (since it is specified in seconds, and we need to convert it to a number
    // of samples)
    specDelayAmpReadPos = (int)(specDelayAmpWritePos - (specDelayLength1_ + fftActualTransformSize_) + specDelayAmpBufferlength) % specDelayAmpBufferlength;
    specDelayPhReadPos = (int)(specDelayPhWritePos - (specDelayLength1_) + specDelayPhBufferlength) % specDelayPhBufferlength;
}

void SpectralDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    deinitFFT();
    deinitWindow();
    preparedToPlay_ = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectralDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SpectralDelayAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    //making copy of audio buffer img, which would be used to display the spectrogram
    AudioSampleBuffer img;
    img.makeCopyOf(buffer);
    
    
    
    int channel, inwritepos, sampsincefft;
    int outreadpos, outwritepos;
    

    // Grab the lock that prevents the FFT settings from changing
    fftSpinLock_.enter();
    
    // Check that we're initialised and ready to go. If not, set output to 0
    if(!fftInitialised_)
    {
        for (channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            buffer.clear (channel, 0, buffer.getNumSamples());
        }
        
        fftSpinLock_.exit();
        return;
    }
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        
        
        // channelData is an array of length numSamples which contains the audio for one channel
        float* channelData = buffer.getWritePointer(channel);
        float* ampBufferDelayData = specDelayAmpBuffer.getWritePointer(jmin (channel, specDelayAmpBuffer.getNumChannels() - 1));
        float* phBufferDelayData = specDelayPhBuffer.getWritePointer(jmin (channel, specDelayPhBuffer.getNumChannels() - 1));
        // inputBufferData is the circular buffer for collecting input samples for the FFT
        float* inputBufferData = inputBuffer_.getWritePointer(jmin (channel, inputBuffer_.getNumChannels() - 1));
        float* outputBufferData = outputBuffer_.getWritePointer(jmin (channel, inputBuffer_.getNumChannels() - 1));
        
        // State variables need to be temporarily cached for each channel. We don't want the
        // operations on one channel to affect the identical behaviour of the next channel
        inwritepos = inputBufferWritePosition_;
        outwritepos = outputBufferWritePosition_;
        outreadpos = outputBufferReadPosition_;
        sampsincefft = samplesSinceLastFFT_;
        
        for (int i = 0; i < numSamples; ++i)
        {
            const float in = channelData[i];
            
            // Store the next buffered sample in the output. Do this first before anything
            // changes the output buffer-- we will have at least one FFT size worth of data
            // stored and ready to go. Set the result to 0 when finished in preparation for the
            // next overlap/add procedure.
            channelData[i] = outputBufferData[outreadpos];
            outputBufferData[outreadpos] = 0.0;
            if(++outreadpos >= outputBufferLength_)
                outreadpos = 0;
            
            // Store the current sample in the input buffer, incrementing the write pointer. Also
            // increment how many samples we've stored since the last transform. If it reaches the
            // hop size, perform an FFT and any frequency-domain processing.
            inputBufferData[inwritepos] = in;
            if (++inwritepos >= inputBufferLength_)
                inwritepos = 0;
            if (++sampsincefft >= hopActualSize_)
            {
                sampsincefft = 0;
                
                // Find the index of the starting sample in the buffer. When the buffer length
                // is equal to the transform size, this will be the current write position but
                // this code is more general for larger buffers.
                int inputBufferStartPosition = (inwritepos + inputBufferLength_
                                                - fftActualTransformSize_) % inputBufferLength_;
                
                // Window the buffer and copy it into the FFT input
                int inputBufferIndex = inputBufferStartPosition;
                for(int fftBufferIndex = 0; fftBufferIndex < fftActualTransformSize_; fftBufferIndex++)
                {
                    // Set real part to windowed signal; imaginary part to 0.
                    fftTimeDomain_[fftBufferIndex][1] = 0.0;
                    if(fftBufferIndex >= windowBufferLength_) // Safety check, in case window isn't ready
                        fftTimeDomain_[fftBufferIndex][0] = 0.0;
                    else
                        fftTimeDomain_[fftBufferIndex][0] = windowBuffer_[fftBufferIndex]
                        * inputBufferData[inputBufferIndex];
                    inputBufferIndex++;
                    if(inputBufferIndex >= inputBufferLength_)
                        inputBufferIndex = 0;
                }
                
                // Perform the FFT on the windowed data, going into the frequency domain.
                // Result will be in fftFrequencyDomain_
                fftw_execute(fftForwardPlan_);
                
                // ********** PHASE VOCODER PROCESSING GOES HERE **************
                // This is the place where frequency-domain calculations are made
                // on the transformed signal. Put the result back into fftFrequencyDomain_
                
                

                
                
                for (int i = 0; i < fftActualTransformSize_; i++){
                    period[i] = 52*sin(i * 1/(fftActualTransformSize_/specDelayLength1_))+52;
                    if (period[i] < 1){period[i] = 1;}
                    delayArray[i] = 105*cos(i * 1/(fftActualTransformSize_/period[i]))+105;
                    if (delayArray[i] < 3) delayArray[i] = 3;
                    if (delayArray[i] > 212) delayArray[i] = 212;
                    
                }
                
                
                int dpAmpLength;
                int dprAmp;
                int dprAmp2 = 0;
                int dpwAmp = specDelayPhWritePos;
                
                
                
                
                
                for (int i = 0; i<=fftActualTransformSize_/2; i++)
                {
                    dpAmpLength = (delayArray[i] * fftActualTransformSize_/(getSampleRate()/1000)); //convert to miliseconds

                    dprAmp = (int)(dpwAmp - (dpAmpLength) + specDelayAmpBufferlength) % specDelayAmpBufferlength;
                    dprAmp2 = (int)(dpwAmp - (dpAmpLength) + specDelayAmpBufferlength + 90) % specDelayAmpBufferlength;
                    
                    double amplitude = sqrt(fftFrequencyDomain_[i][0]*fftFrequencyDomain_[i][0]+fftFrequencyDomain_[i][1]*fftFrequencyDomain_[i][1]);
                    double phase = atan2(fftFrequencyDomain_[i][1], fftFrequencyDomain_[i][0]);
                    
                    //Apply delay to amplitude buffer
                    
                    const float in = fftFrequencyDomain_[i][0];
                    float outAmp = 0.0;
                    outAmp = (0.7 * in) + (0.3 * ampBufferDelayData[dprAmp]) + (0.2 * ampBufferDelayData[dprAmp2]);
                    ampBufferDelayData[dpwAmp] = in;
                    
                    //Apply delay to phase buffer
                    
                    const float inPh = phase;
                    float outPh = 0.0;
                    outPh = (0.7 * inPh) + (0.3 * phBufferDelayData[dprAmp]) + (0.2 * phBufferDelayData[dprAmp2]);
                    phBufferDelayData[dpwAmp] = inPh;
                    
                    
                    //Increment pointers and set to 0 if they are more than the size of buffer.
                    
                    if (++dprAmp >= specDelayAmpBufferlength) dprAmp = 0;
                    if (++dprAmp2 >= specDelayAmpBufferlength) dprAmp2 = 0;
                    if (++dpwAmp >= specDelayAmpBufferlength) dpwAmp = 0;
                    
                    outAmp = outAmp / 768; //normalise values
                    outPh = outPh / 768;
                    
                    // (⊙_⊙) turn back to real-imaginary form
                 fftFrequencyDomain_[i][0] = outAmp*cos(outPh);
                 fftFrequencyDomain_[i][1] = outAmp*sin(outPh);
                  
                    
                    if(i > 0 && i < fftActualTransformSize_ / 2) {
                        fftFrequencyDomain_[fftActualTransformSize_ - i][0] = outAmp * cos(outPh);
                        fftFrequencyDomain_[fftActualTransformSize_ - i][1] = - outAmp * sin(outPh);
                        
                    }
                    
                }
                
                
                
                
                
                // In this example, we don't do anything except reconstruct the original
                // signal to show that the whole infrastructure works.
                // ************************************************************
                
                // Perform the inverse FFT to get back to the time domain. Result wll be
                // in fftTimeDomain_. If we've done it right (kept the frequency domain
                // symmetric), the time domain resuld should be strictly real allowing us
                // to ignore the imaginary part.
                fftw_execute(fftBackwardPlan_);
                
                specDelayAmpReadPos = dprAmp;
                specDelayAmpWritePos = dpwAmp;
                
                // Add the result to the output buffer, starting at the current write position
                // (Output buffer will have been zeroed after reading the last time around)
                // Output needs to be scaled by the transform size to get back to original amplitude:
                // this is a property of how fftw is implemented. Scaling will also need to be adjusted
                // based on hop size to get the same output level (smaller hop size produces more overlap
                // and hence higher signal level)
                int outputBufferIndex = outwritepos;
                for(int fftBufferIndex = 0; fftBufferIndex < fftActualTransformSize_; fftBufferIndex++)
                {
                    outputBufferData[outputBufferIndex] += fftTimeDomain_[fftBufferIndex][0] * fftScaleFactor_ * 830;
                    if(++outputBufferIndex >= outputBufferLength_)
                        outputBufferIndex = 0;
                }
                
                // Advance the write position within the buffer by the hop size
                outwritepos = (outwritepos + hopActualSize_) % outputBufferLength_;
            }
        }
        

        //pointer to copy of buffer which will be used to render spectrogram image
        
        float* imgData = img.getWritePointer (channel);
        
        for (int i = 0; i < numSamples; ++i){
            imgData[i] = channelData[i];
        }
        
        //going through all the samples from the copy of the audio buffer and pushing into fft algorithm
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            spect->pushNextSampleIntoFifo(imgData[i]);
        
    }
    
    // Having made a local copy of the state variables for each channel, now transfer the result
    // back to the main state variable so they will be preserved for the next call of processBlock()
    inputBufferWritePosition_ = inwritepos;
    outputBufferWritePosition_ = outwritepos;
    outputBufferReadPosition_ = outreadpos;
    samplesSinceLastFFT_ = sampsincefft;
    

    

 

    
    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
        
    }
    
    fftSpinLock_.exit();
    
}

//==============================================================================
bool SpectralDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SpectralDelayAudioProcessor::createEditor()
{
    return new SpectralDelayAudioProcessorEditor (*this);
}

//==============================================================================
void SpectralDelayAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Create an outer XML element..
    XmlElement xml("spectralplugin");
    

    xml.setAttribute("fftSize", fftSelectedSize_);
    xml.setAttribute("hopSize", hopSelectedSize_);
    xml.setAttribute("windowType", windowType_);
    //xml.setAttribute("specDelayLength1", specDelayLength1_);
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary(xml, destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpectralDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if(xmlState != 0)
    {
        // make sure that it's actually our type of XML object..
        if(xmlState->hasTagName("spectralplugin"))
        {
            // ok, now pull out our parameters..
            
            fftSelectedSize_  = (int)xmlState->getDoubleAttribute("fftSize", fftSelectedSize_);
            hopSelectedSize_  = (int)xmlState->getDoubleAttribute("hopSize", hopSelectedSize_);
            windowType_  = (int)xmlState->getDoubleAttribute("windowType", windowType_);
       //     specDelayLength1_ = (int)xmlState->getDoubleAttribute("specDelayLength1", specDelayLength1_);
            
            if(preparedToPlay_)
            {
                // Update settings if currently playing, else wait until prepareToPlay() called
                initFFT(fftSelectedSize_);
                initWindow(fftSelectedSize_, windowType_);
            }
        }
    }
}

void SpectralDelayAudioProcessor::setParameter(int index, float newValue){
    
    switch(index){
        case kspecDelayParam:
            specDelayLength1_ = newValue;
           // specDelayAmpReadPos = (int)(specDelayAmpWritePos - (specDelayLength1_ * getSampleRate()) + specDelayAmpBufferlength) % ////specDelayAmpBufferlength;
            break;
        case kdryMixParam:
            dryMix_ = newValue;
            break;
        case kwetMixParam:
            wetMix_ = newValue;
            break;
        case kfftSizeParam:
            if((int)newValue != fftSelectedSize_)
            {
                fftSelectedSize_ = (int)newValue;
                if(preparedToPlay_)
                {
                    // Update settings if currently playing, else wait until prepareToPlay() called
                    initFFT(fftSelectedSize_);
                    initWindow(fftSelectedSize_, windowType_);
                }
            }
            break;
        case kHopSizeParam:
            hopSelectedSize_ = (int)newValue;
            if(preparedToPlay_)
                updateHopSize();
            break;
        case kWindowTypeParam:
            // Recalculate window if needed
            if((int)newValue != windowType_)
            {
                windowType_ = (int)newValue;
                if(preparedToPlay_)
                    initWindow(fftActualTransformSize_, (int)newValue);
            }
            break;
        default:
            break;
    }
    

}
float SpectralDelayAudioProcessor::getParameter(int index){
    switch(index){
        case kspecDelayParam: return specDelayLength1_;
        case kdryMixParam: return dryMix_;
        case kwetMixParam: return wetMix_;
        case kfftSizeParam: return (float)fftSelectedSize_;
        case kHopSizeParam: return (float)hopSelectedSize_;
        case kWindowTypeParam: return (float)windowType_;
        default:    return 0.0f;
    }
}

int SpectralDelayAudioProcessor::getNumParameters(){
    return kNumParameters; 
}

const String SpectralDelayAudioProcessor::getParameterName (int index) {
    switch (index)
    {
        case kspecDelayParam:      return "Spectral Delay";
        case kwetMixParam:      return "wet mix";
        case kdryMixParam:       return "dry mix";
        case kfftSizeParam:    return "FFT size";
        case kHopSizeParam:    return "hop size";
        case kWindowTypeParam: return "window type";
        default:               break;
    }
    return String::empty;
    
}

//==============================================================================
// Initialise the FFT data structures for a given length transform
void SpectralDelayAudioProcessor::initFFT(int length)
{
    if(fftInitialised_)
        deinitFFT();
    
    // Save the current length so we know how big our results are later
    fftActualTransformSize_ = length;
    
    // Here we allocate the complex-number buffers for the FFT. This uses
    // a convenient wrapper on the more general fftw_malloc()
    fftTimeDomain_ = fftw_alloc_complex(length);
    fftFrequencyDomain_ = fftw_alloc_complex(length);
    
    // FFTW_ESTIMATE doesn't necessarily produce the fastest executing code (FFTW_MEASURE
    // will get closer) but it carries a minimum startup cost. FFTW_MEASURE might stall for
    // several seconds which would be annoying in an audio plug-in context.
    fftForwardPlan_ = fftw_plan_dft_1d(fftActualTransformSize_, fftTimeDomain_,
                                       fftFrequencyDomain_, FFTW_FORWARD, FFTW_ESTIMATE);
    fftBackwardPlan_ = fftw_plan_dft_1d(fftActualTransformSize_, fftFrequencyDomain_,
                                        fftTimeDomain_, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    // Allocate the buffer that the samples will be collected in
    inputBufferLength_ = fftActualTransformSize_;
    inputBuffer_.setSize(2, inputBufferLength_);
    inputBuffer_.clear();
    inputBufferWritePosition_ = 0;
    samplesSinceLastFFT_ = 0;
    
    // Allocate the output buffer to be twice the size of the FFT
    // This will be enough for all hop size cases
    outputBufferLength_ = 2*fftActualTransformSize_;
    outputBuffer_.setSize(2, outputBufferLength_);
    outputBuffer_.clear();
    outputBufferReadPosition_ = 0;
    
    updateHopSize();
    
    fftInitialised_ = true;
}

// Free the FFT data structures
void SpectralDelayAudioProcessor::deinitFFT()
{
    if(!fftInitialised_)
        return;
    
    // Prevent this variable from changing while an audio callback is running.
    // Once it has changed, the next audio callback will find that it's not
    // initialised and will return silence instead of attempting to work with the
    // (invalid) FFT structures. This produces an audible glitch but no crash,
    // and is the simplest way to handle parameter changes in this example code.
    fftSpinLock_.enter();
    fftInitialised_ = false;
    fftSpinLock_.exit();
    
    fftw_destroy_plan(fftForwardPlan_);
    fftw_destroy_plan(fftBackwardPlan_);
    fftw_free(fftTimeDomain_);
    fftw_free(fftFrequencyDomain_);
    
    // Leave the input buffer in memory until the plugin is released
}

//==============================================================================
// Create a new window of a given length and type
void SpectralDelayAudioProcessor::initWindow(int length, int windowType)
{
    if(windowBuffer_ != 0)
        deinitWindow();
    if(length == 0) // Sanity check
        return;
    
    // Allocate memory for the window
    windowBuffer_ = (double *)malloc(length * sizeof(double));
    
    // Write the length as a double here to simplify the code below (otherwise
    // typecasts would be wise)
    double windowLength = length;
    
    // Set values for the window, depending on its type
    for(int i = 0; i < length; i++)
    {
        // Window functions are typically defined to be symmetrical. This will cause a
        // problem in the overlap-add process: the windows instead need to be periodic
        // when arranged end-to-end. As a result we calculate the window of one sample
        // larger than usual, and drop the last sample. (This works as long as N is even.)
        // See Julius Smith, "Spectral Audio Signal Processing" for details.
        switch(windowType)
        {
            case kWindowBartlett:
                windowBuffer_[i] = (2.0/(windowLength + 2.0))*
                (0.5*(windowLength + 2.0) - abs((double)i - 0.5*windowLength));
                break;
            case kWindowHann:
                windowBuffer_[i] = 0.5*(1.0 - cos(2.0*M_PI*(double)i/windowLength));
                break;
            case kWindowHamming:
                windowBuffer_[i] = 0.54 - 0.46*cos(2.0*M_PI*(double)i/windowLength);
                break;
            case kWindowRectangular:
            default:
                windowBuffer_[i] = 1.0;
                break;
        }
    }
    
    windowBufferLength_ = length;
    updateScaleFactor();
}

// Free the window buffer
void SpectralDelayAudioProcessor::deinitWindow()
{
    if(windowBuffer_ == 0)
        return;
    
    // Delay clearing the window until the audio thread is not running
    // to avoid a crash if the code tries to access an invalid window
    fftSpinLock_.enter();
    windowBufferLength_ = 0;
    fftSpinLock_.exit();
    
    free(windowBuffer_);
    windowBuffer_ = 0;
}

// Update the actual hop size depending on the window size and hop size settings
// Hop size is expressed as a fraction of a window in the parameters.
void SpectralDelayAudioProcessor::updateHopSize()
{
    switch(hopSelectedSize_)
    {
        case kHopSize1Window:
            hopActualSize_ = fftActualTransformSize_;
            break;
        case kHopSize1_2Window:
            hopActualSize_ = fftActualTransformSize_ / 2;
            break;
        case kHopSize1_4Window:
            hopActualSize_ = fftActualTransformSize_ / 4;
            break;
        case kHopSize1_8Window:
            hopActualSize_ = fftActualTransformSize_ / 8;
            break;
    }
    
    // Update the factor by which samples are scaled to preserve unity gain
    updateScaleFactor();
    
    // Read pointer lags the write pointer to allow for FFT buffers to accumulate and
    // be processed. Total latency is sum of the FFT size and the hop size.
    outputBufferWritePosition_ = hopActualSize_ + fftActualTransformSize_;
}

// Update the factor by which each output sample is scaled. This needs to update
// every time FFT size, hop size, and window type are changed.
void SpectralDelayAudioProcessor::updateScaleFactor()
{
    // The gain needs to be normalised by the sum of the window, which implicitly
    // accounts for the length of the transform and the window type. From there
    // we also update based on hop size: smaller hop means more overlap means the
    // overall gain should be reduced.
    double windowSum = 0.0;
    
    for(int i = 0; i < windowBufferLength_; i++)
    {
        windowSum += windowBuffer_[i];
    }
    
    if(windowSum == 0.0)
        fftScaleFactor_ = 0.0; // Catch invalid cases and mute output
    else
    {
        switch(hopSelectedSize_)
        {
            case kHopSize1Window:   // 0dB
                fftScaleFactor_ = 1.0/(double)windowSum;
                break;
            case kHopSize1_2Window: // -6dB
                fftScaleFactor_ = 0.5/(double)windowSum;
                break;
            case kHopSize1_4Window: // -12dB
                fftScaleFactor_ = 0.25/(double)windowSum;
                break;
            case kHopSize1_8Window: // -18dB
                fftScaleFactor_ = 0.125/(double)windowSum;
                break;
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectralDelayAudioProcessor();
}
