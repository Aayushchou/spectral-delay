# Spectral Delay Phase Vocoder

## Requirements

Requires FFTW library to compile 


http://www.fftw.org


How to make it work in XCode on the mac 

https://ndworkblog.wordpress.com/2016/11/13/installing-fftw3-on-mac-for-use-in-xcode/


## Motivation 

Manipulating individual frequency bins after performing FFT allows for a lot of flexibility in the ability to shape the timbre of a certain sound. I initially created a pureData patch where I implemented a spectral delay by writing the output of rfft~ onto separate buffers, then applying a delay to the bins. This created a very interesting effect of what sounded like stars falling from the sky. The input audio is stored into neighbouring bins, in case it doesn’t fit in the bin size and when these neighbouring bins are played in different times, theres an attenuation in amplitude for certain bins, which creates an interesting effect. My goal was to use this idea to implement a phase vocoder using JUCE, which determines its pitch shifts using information from previous bins, sounding ghostly and somewhat spooky.

## Implementation 

The audio is transformed into the frequency domain using the fftw3 library. In the frequency domain, the amplitude data and the phase data are stored in two separate buffers. A Multi-tap delay is implemented which causes the buffers to read from a previous real and imaginary output of the FFT and stores this information in the current instance of the buffers. The delay is an integer multiple of the FFT size therefore the delay causes the delay buffer to read values of bins from previous FFT frames. This causes the current frequency bin to have values from bins from previous frames, resulting in multiple pitches being heard and causing an effect similar to a pitch shift. The delay is applied separately to the phase and magnitude buffers, although the value of the delay is the same for both. Before performing IFFT, it is important to note when using fftw3, the second half of the output values is the conjugate of the first half, therefore it is important to keep the frequency domain symmetric. After performing IFFT, the signal is brought back together through overlapping and adding and is then normalised in accordance to the number of overlaps. Higher number of overlaps require larger attenuations.

# Parameters and GUI

The plug-in consists of three parameters, dry mix, wet mix and spectral delay. The dry mix and wet mix determine how much of the original signal is played back as compared to the processed signal. The spectral delay parameter determines the period of a sine wave with the equation:

period[i] = 52sin(i/(fftTransformSize/specDelay))+52

This equation is then used to determine the period for a cosine wave with the equation:

delayArray[i] = 105cos(i/(fftTransformSize/period[i]))+105

The output of the cosine wave is stored in an array which determines the amount of delay for each individual bin. This is done this way as modulating the amount of delay for each individual bin produces a more dynamic change in the spectral contents of the signal. The signals are shifted and scaled so that the delay doesn’t exceed 2500ms. 4 overlaps are used and the delay buffer length is 10000, therefore that maximum delay possible would be 10000/4 = 2500ms.
The GUI design is fairly straightforward, it consists of: combo boxes to let the user choose hop size, window type and FFT size, the sliders discussed previous and a spectrogram displaying spectral information about the audio output. The spectrogram was an essential component as it allows the user to analyse the affect of the plug-in on their audio as they are manipulating it. The spectrogram works by sending FFT data of the output audio onto an array, which is then rendered as an image using the JUCE class spectrogramImage.
