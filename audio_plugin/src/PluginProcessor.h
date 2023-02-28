/**
 * @file PluginProcessor.h
 * @brief JUCE audio plugin example using the interface to the proposed core
 *
 * Mostly derived/copied from the audio plugin code found here:
 * https://github.com/jananifernandez/HADES
 * Copyright (c) 2021 - Janani Fernandez & Leo McCormack,  (GPLv3 License)
 * And from here:
 * https://github.com/leomccormack/SPARTA
 * Copyright (c) 2018-2022 - Leo McCormack,  (GPLv3 License)
 *
 * Copyright (c) Meta Platforms, Inc. All Rights Reserved
 * @author Leo McCormack
 * @date 10th August 2022
 */

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "JuceHeader.h" 
#include "interface.h"
#include <thread> 
#define BUILD_VER_SUFFIX "alpha"
#define DEFAULT_OSC_PORT 9000
#ifndef MIN
# define MIN(a,b) (( (a) < (b) ) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b) (( (a) > (b) ) ? (a) : (b))
#endif

typedef enum _TIMERS{
    TIMER_PROCESSING_RELATED = 1,
    TIMER_GUI_RELATED
}TIMERS;

enum {
    /* For the default VST GUI */
    k_listenerX,
    k_listenerY,
    k_listenerZ,
    k_listenerYaw,
    k_listenerPitch,
    k_listenerRoll,
    k_lin2parBalance,
    k_dir2diffBalance,
    k_enableSourceDirectivity,
    k_3dofOR6dof,
    k_distanceMapMode,
    k_favour2D,
    k_enableEPbeams,
    k_bsmMaxFreq,
    k_analysisMaxFreq,
    k_maglsMaxFreq,
    k_enableDiffEQhrtf,
    k_enableDiffEQatf,
    
    k_NumOfParameters
};

class PluginProcessor  : public AudioProcessor,
                         public MultiTimer,
                         private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
                         public VSTCallbackHandler
{
public:
    /* Get functions */
    void* getFXHandle() { return hInt; }
    int getCurrentBlockSize(){ return nHostBlockSize; }
    int getCurrentNumInputs(){ return nNumInputs; }
    int getCurrentNumOutputs(){ return nNumOutputs; }
    
    /* VST CanDo */
    pointer_sized_int handleVstManufacturerSpecific (int32 /*index*/, pointer_sized_int /*value*/, void* /*ptr*/, float /*opt*/) override { return 0; };
    pointer_sized_int handleVstPluginCanDo (int32 /*index*/, pointer_sized_int /*value*/, void* ptr, float /*opt*/) override{
        auto text = (const char*) ptr;
        auto matches = [=](const char* s) { return strcmp (text, s) == 0; };
        if (matches ("wantsChannelCountNotifications"))
            return 1;
        return 0;
    }
    
    /* OSC */
    void oscMessageReceived(const OSCMessage& message) override;
    void setOscPortID(int newID){
        osc.disconnect();
        osc_port_ID = newID;
        osc_connected = osc.connect(osc_port_ID);
    }
    int getOscPortID(){ return osc_port_ID; }
    bool getOscPortConnected(){ return osc_connected; }

private:
    void* hInt;             /* interface handle */
    int nNumInputs;         /* current number of input channels */
    int nNumOutputs;        /* current number of output channels */
    int nSampleRate;        /* current host sample rate */
    int nHostBlockSize;     /* typical host block size to expect, in samples */
    
    OSCReceiver osc;         /* OSC receiver object */
    bool osc_connected;      /* flag. 0: not connected, 1: connect to "osc_port_ID"  */
    int osc_port_ID;         /* port ID */
    
    void timerCallback(int timerID) override {
        switch(timerID){
            case TIMER_PROCESSING_RELATED:
                /* reinitialise codec if needed */
                if(interface_getCoreStatus(hInt) == CORE_STATUS_NOT_INITIALISED){
                    try{
                        std::thread threadInit(interface_initCore, hInt);
                        threadInit.detach();
                    } catch (const std::exception& exception) {
                        std::cout << "Could not create thread" << exception.what() << std::endl;
                    }
                }
                break;
                
            case TIMER_GUI_RELATED:
                /* handled in PluginEditor; */
                break;
        }
    }

    /***************************************************************************\
                                    JUCE Functions
    \***************************************************************************/
public:
    PluginProcessor();
    ~PluginProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    int getNumParameters() override;
    float getParameter (int index) override;
    void setParameter (int index, float newValue) override;
    void setParameterRaw(int index, float newValue);
    const String getParameterName (int index) override;
    const String getParameterText (int index) override;
    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair(int index) const override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void changeProgramName(int index, const String& newName) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
