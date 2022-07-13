/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
SpectralDelayAudioProcessorEditor::SpectralDelayAudioProcessorEditor (SpectralDelayAudioProcessor& p)
    : AudioProcessorEditor (&p),
        specDelayLabel1_("", "Spectral Delay"),
        dryMixLabel_("", "Dry level"),
        wetMixLabel_("", "Wet level"),
        fftSizeLabel_("", "FFT Size:"),
        hopSizeLabel_("", "Hop Size:"),
        windowTypeLabel_("", "Window Type:"),
        processor (p)
{
    
    //setting up combo boxes
    
    addAndMakeVisible(&fftSizeComboBox_);
    fftSizeComboBox_.setEditableText(false);
    fftSizeComboBox_.setJustificationType(Justification::left);
    fftSizeComboBox_.addItem("64", 64);
    fftSizeComboBox_.addItem("128", 128);
    fftSizeComboBox_.addItem("256", 256);
    fftSizeComboBox_.addItem("512", 512);
    fftSizeComboBox_.addItem("1024", 1024);
    fftSizeComboBox_.addItem("2048", 2048);
    fftSizeComboBox_.addListener(this);
    
    addAndMakeVisible(&hopSizeComboBox_);
    hopSizeComboBox_.setEditableText(false);
    hopSizeComboBox_.setJustificationType(Justification::left);
    hopSizeComboBox_.addItem("1 Window", SpectralDelayAudioProcessor::kHopSize1Window);
    hopSizeComboBox_.addItem("1/2 Window", SpectralDelayAudioProcessor::kHopSize1_2Window);
    hopSizeComboBox_.addItem("1/4 Window", SpectralDelayAudioProcessor::kHopSize1_4Window);
    hopSizeComboBox_.addItem("1/8 Window", SpectralDelayAudioProcessor::kHopSize1_8Window);
    hopSizeComboBox_.addListener(this);
    
    addAndMakeVisible(&windowTypeComboBox_);
    windowTypeComboBox_.setEditableText(false);
    windowTypeComboBox_.setJustificationType(Justification::left);
    windowTypeComboBox_.addItem("Rectangular", SpectralDelayAudioProcessor::kWindowRectangular);
    windowTypeComboBox_.addItem("Bartlett", SpectralDelayAudioProcessor::kWindowBartlett);
    windowTypeComboBox_.addItem("Hann", SpectralDelayAudioProcessor::kWindowHann);
    windowTypeComboBox_.addItem("Hamming", SpectralDelayAudioProcessor::kWindowHamming);
    windowTypeComboBox_.addListener(this);
    
    //setting up sliders
    
    addAndMakeVisible(&specDelaySlider1_);
    specDelaySlider1_.addListener(this);
    specDelaySlider1_.setRange(1.0, 215.0, 0.01);
    specDelaySlider1_.setSliderStyle(Slider::Rotary);
    specDelaySlider1_.setValue(processor.getParameter(SpectralDelayAudioProcessor::kspecDelayParam));
    
    
    addAndMakeVisible(&dryMixSlider_);
    dryMixSlider_.addListener(this);
    dryMixSlider_.setRange(0.0, 1.0, 0.01);
    dryMixSlider_.setSliderStyle(Slider::Rotary);
    dryMixSlider_.setValue(processor.getParameter(SpectralDelayAudioProcessor::kdryMixParam));
    
    
    addAndMakeVisible(&wetMixSlider_);
    wetMixSlider_.addListener(this);
    wetMixSlider_.setRange(0.0, 1.0, 0.01);
    wetMixSlider_.setSliderStyle(Slider::Rotary);
    wetMixSlider_.setValue(processor.getParameter(SpectralDelayAudioProcessor::kwetMixParam));
    
    
    addAndMakeVisible(&specDelayLabel1_);
    addAndMakeVisible(&wetMixLabel_);
    addAndMakeVisible(&dryMixLabel_);
    addAndMakeVisible(&fftSizeLabel_);
    addAndMakeVisible(&hopSizeLabel_);
    addAndMakeVisible(&windowTypeLabel_);
    
    addAndMakeVisible(processor.spect);
    
    startTimer(50);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (550, 600);
}

SpectralDelayAudioProcessorEditor::~SpectralDelayAudioProcessorEditor()
{
}

void SpectralDelayAudioProcessorEditor:: timerCallback(){
    
    //setting values of sliders
    
    specDelaySlider1_.setValue(processor.specDelayLength1_, dontSendNotification);
  //  wetMixSlider_.setValue(processor.wetMix_, dontSendNotification);
  //  dryMixSlider_.setValue(processor.dryMix_, dontSendNotification);
    
    //setting values of combo box
    
    fftSizeComboBox_.setSelectedId(processor.fftSelectedSize_, dontSendNotification);
    hopSizeComboBox_.setSelectedId(processor.hopSelectedSize_, dontSendNotification);
    windowTypeComboBox_.setSelectedId(processor.windowType_, dontSendNotification);
    
    //rendering next line of spectrogram if fftblock is ready.
    
   if (processor.spect->nextFFTBlockReady)
    {
        processor.spect->drawNextLineOfSpectrogram();
        processor.spect->nextFFTBlockReady = false;
        processor.spect->repaint();
    }
    
    
}

void SpectralDelayAudioProcessorEditor:: sliderValueChanged(Slider* slider){
    
   if (slider == &specDelaySlider1_){
        processor.setParameterNotifyingHost(SpectralDelayAudioProcessor::kspecDelayParam, (float)specDelaySlider1_.getValue());
        
    }
    
    else if (slider == &dryMixSlider_){
        processor.setParameterNotifyingHost(SpectralDelayAudioProcessor::kdryMixParam, (float)dryMixSlider_.getValue());
    }
    
    else if (slider == &wetMixSlider_){
        processor.setParameterNotifyingHost(SpectralDelayAudioProcessor::kwetMixParam, (float)wetMixSlider_.getValue());
        
    }
}

void SpectralDelayAudioProcessorEditor:: comboBoxChanged(ComboBox *combobox){
    
    if (combobox == &fftSizeComboBox_) {
        processor.setParameterNotifyingHost(SpectralDelayAudioProcessor::kfftSizeParam, (float)fftSizeComboBox_.getSelectedId());
    }
    else if (combobox == &hopSizeComboBox_) {
        processor.setParameterNotifyingHost(SpectralDelayAudioProcessor::kHopSizeParam, (float)hopSizeComboBox_.getSelectedId());
    }
    else if (combobox == &windowTypeComboBox_) {
        processor.setParameterNotifyingHost(SpectralDelayAudioProcessor::kWindowTypeParam, (float)windowTypeComboBox_.getSelectedId());
    }
    
    
}

//==============================================================================
void SpectralDelayAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.setOpacity (1.0f);

}

void SpectralDelayAudioProcessorEditor::resized()
{
    specDelaySlider1_.setBounds(30, 90, 150, 75);
    dryMixSlider_.setBounds(210, 90, 150, 75);
    wetMixSlider_.setBounds(390, 90, 150, 75);
    specDelayLabel1_.setBounds(27, 60, 150, 75);
    dryMixLabel_.setBounds(210, 60, 150, 75);
    wetMixLabel_.setBounds(390, 60, 150, 75);
    fftSizeLabel_.setBounds(20, 10, 150, 30);
    hopSizeLabel_.setBounds(200, 10, 150, 30);
    windowTypeLabel_.setBounds(380, 10, 150, 30);
    fftSizeComboBox_.setBounds(20, 35, 150, 30);
    hopSizeComboBox_.setBounds(200, 35, 150, 30);
    windowTypeComboBox_.setBounds(380, 35, 150, 30);
   processor.spect->setBounds(0, 239, 550, 360);
    
    
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
