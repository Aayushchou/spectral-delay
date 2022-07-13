/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"



//==============================================================================
/**
*/
class SpectralDelayAudioProcessorEditor  : public AudioProcessorEditor, public Slider::Listener, public Timer, public ComboBox::Listener
{
public:
    SpectralDelayAudioProcessorEditor (SpectralDelayAudioProcessor&);
    ~SpectralDelayAudioProcessorEditor();
    
    void comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) override;
    
    
    //Functions for dealing with the sliders.
    
    void sliderValueChanged(juce::Slider *slider) override;
    void timerCallback() override;
    

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    

private:
    
    //Declaring all the labels and pointers
    
    Label specDelayLabel1_;
    Label dryMixLabel_;
    Label wetMixLabel_;
    Label fftSizeLabel_;
    Label hopSizeLabel_;
    Label windowTypeLabel_;
    Slider specDelaySlider1_;
    Slider dryMixSlider_;
    Slider wetMixSlider_;
    ComboBox fftSizeComboBox_;
    ComboBox hopSizeComboBox_;
    ComboBox windowTypeComboBox_;
    
    
    
    ScopedPointer<ResizableCornerComponent> resizer_;
    ComponentBoundsConstrainer resizeLimits_;
    
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpectralDelayAudioProcessor& processor;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralDelayAudioProcessorEditor)
};
