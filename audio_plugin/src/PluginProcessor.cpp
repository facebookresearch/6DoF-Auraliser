/**
 * @file PluginProcessor.cpp
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

#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginProcessor::PluginProcessor()
{
	nSampleRate = 48000;
	interface_create(&hInt);
    
    /* specify here on which UDP port number to receive incoming OSC messages */
    osc_port_ID = DEFAULT_OSC_PORT;
    osc.connect(osc_port_ID);
    /* tell the component to listen for OSC messages */
    osc.addListener(this);

    startTimer(TIMER_PROCESSING_RELATED, 40); 
}

PluginProcessor::~PluginProcessor()
{
	interface_destroy(&hInt);
}

void PluginProcessor::oscMessageReceived(const OSCMessage& message)
{
    /* if rotation angles are sent as an array \ypr[3] */
    if (message.size() == 3 && message.getAddressPattern().toString().compare("ypr")) {
        if (message[0].isFloat32())
            interface_setYaw(hInt, message[0].getFloat32());
        if (message[1].isFloat32())
            interface_setPitch(hInt, message[1].getFloat32());
        if (message[2].isFloat32())
            interface_setRoll(hInt, message[2].getFloat32());
        return;
    }
//    else if (message.size() == 7 && message.getAddressPattern().toString().compare("xyzquat")) {
//        if (message[0].isFloat32())
//            setParameterRaw(0, message[0].getFloat32());
//        if (message[1].isFloat32())
//            setParameterRaw(1, message[1].getFloat32());
//        if (message[2].isFloat32())
//            setParameterRaw(2, message[2].getFloat32());
//        if (message[3].isFloat32())
//            rotator_setQuaternionW(hRot, message[3].getFloat32());
//        if (message[4].isFloat32())
//            rotator_setQuaternionX(hRot, message[4].getFloat32());
//        if (message[5].isFloat32())
//            rotator_setQuaternionY(hRot, message[5].getFloat32());
//        if (message[6].isFloat32())
//            rotator_setQuaternionY(hRot, message[6].getFloat32());
//        return;
//    }

    else if (message.size() == 6 && message.getAddressPattern().toString().compare("xyzypr")) {
        if (message[0].isFloat32())
            setParameterRaw(0, message[0].getFloat32());
        if (message[1].isFloat32())
            setParameterRaw(1, message[1].getFloat32());
        if (message[2].isFloat32())
            setParameterRaw(2, message[2].getFloat32());
        if (message[3].isFloat32())
            interface_setYaw(hInt, message[3].getFloat32());
        if (message[4].isFloat32())
            interface_setPitch(hInt, message[4].getFloat32());
        if (message[5].isFloat32())
            interface_setRoll(hInt, message[5].getFloat32());

        return;
    }
    
    /* if rotation angles are sent individually: */
    if(message.getAddressPattern().toString().compare("yaw"))
        interface_setYaw(hInt, message[0].getFloat32());
    else if(message.getAddressPattern().toString().compare("pitch"))
        interface_setPitch(hInt, message[0].getFloat32());
    else if(message.getAddressPattern().toString().compare("roll"))
        interface_setRoll(hInt, message[0].getFloat32());
}

void PluginProcessor::setParameter (int index, float newValue)
{
	switch (index) {
        case k_listenerYaw:      interface_setYaw(hInt, (newValue-0.5f)*360.0f ); break;
        case k_listenerPitch:    interface_setPitch(hInt, (newValue - 0.5f)*180.0f); break;
        case k_listenerRoll:     interface_setRoll(hInt, (newValue - 0.5f)*180.0f); break;
        case k_listenerX:        interface_setX(hInt, (newValue - 0.5f)*(INTERFACE_PERIMETER_DISTANCE_M*2)); break;
        case k_listenerY:        interface_setY(hInt, (newValue - 0.5f)*(INTERFACE_PERIMETER_DISTANCE_M*2)); break;
        case k_listenerZ:        interface_setZ(hInt, (newValue - 0.5f)*(INTERFACE_PERIMETER_DISTANCE_M*2)); break;
        case k_lin2parBalance:   interface_setLinear2ParametricBalance(hInt, newValue); break;
        case k_dir2diffBalance:  interface_setStreamBalanceAllBands(hInt, newValue*2.0f); break;
        case k_enableSourceDirectivity: interface_setEnableSourceDirectivity(hInt, newValue>0.5f ? 1 : 0); break;
        case k_3dofOR6dof:       interface_setDOFoption(hInt, newValue>0.5f ? CORE_6DOF : CORE_3DOF_ROTATIONS); break;
        case k_distanceMapMode:  interface_setDistanceMapMode(hInt, (INTERFACE_DISTANCE_MAPS)(newValue*(INTERFACE_NUM_DISTANCE_MAPS)+1.0f)); break;
            
        case k_favour2D:         interface_setFavour2Daccuracy(hInt, newValue>0.5f ? 1 : 0); break;
        case k_enableEPbeams:    interface_setEnableEPbeamformers(hInt, newValue>0.5f ? 1 : 0); break;
        case k_bsmMaxFreq:       interface_setMaximumBSMFreq(hInt, newValue*16e3f); break;
        case k_analysisMaxFreq:  interface_setMaximumAnalysisFreq(hInt, newValue*16e3f); break;
        case k_maglsMaxFreq:     interface_setMaximumMagLSFreq(hInt, newValue*16e3f); break;
        case k_enableDiffEQhrtf:     interface_setEnableDiffEQ_HRTFs(hInt, newValue>0.5f ? 1 : 0); break;
        case k_enableDiffEQatf:     interface_setEnableDiffEQ_ATFs(hInt, newValue>0.5f ? 1 : 0); break;

		default: break;
	}
}

void PluginProcessor::setParameterRaw(int index, float newValue)
{
    if (index < 3) {
        switch (index){
            case 0: interface_setX(hInt, newValue);
            case 1: interface_setY(hInt, newValue);
            case 2: interface_setZ(hInt, newValue);
        } 
    }
}

void PluginProcessor::setCurrentProgram (int /*index*/)
{
}

float PluginProcessor::getParameter (int index)
{
    switch (index) {
        case k_listenerYaw:          return (interface_getYaw(hInt)/360.0f) + 0.5f;
        case k_listenerPitch:        return (interface_getPitch(hInt)/180.0f) + 0.5f;
        case k_listenerRoll:         return (interface_getRoll(hInt)/180.0f) + 0.5f;
        case k_listenerX:            return (interface_getX(hInt)/(INTERFACE_PERIMETER_DISTANCE_M*2)) + 0.5f;
        case k_listenerY:            return (interface_getY(hInt)/(INTERFACE_PERIMETER_DISTANCE_M*2)) + 0.5f;
        case k_listenerZ:            return (interface_getZ(hInt)/(INTERFACE_PERIMETER_DISTANCE_M*2)) + 0.5f;
        case k_lin2parBalance:       return (interface_getLinear2ParametricBalance(hInt));
        case k_dir2diffBalance:      return interface_getStreamBalanceAllBands(hInt)/2.0f;
        case k_enableSourceDirectivity: return (interface_getEnableSourceDirectivity(hInt))>0.5 ? 1.0f : 0.0f;
        case k_3dofOR6dof:
            switch((int)interface_getDOFoption(hInt)){
                case CORE_3DOF_ROTATIONS: return 0.0f;
                case CORE_6DOF: return 1.0f;
            }
        case k_distanceMapMode:      return ((float)interface_getDistanceMapMode(hInt)-1.0f)/(float)INTERFACE_NUM_DISTANCE_MAPS;
            
        case k_favour2D:         return (interface_getFavour2Daccuracy(hInt))>0.5 ? 1.0f : 0.0f;
        case k_enableEPbeams:    return (interface_getEnableEPbeamformers(hInt))>0.5 ? 1.0f : 0.0f;
        case k_bsmMaxFreq:       return interface_getMaximumBSMFreq(hInt)/16e3f;
        case k_analysisMaxFreq:  return interface_getMaximumAnalysisFreq(hInt)/16e3f;
        case k_maglsMaxFreq:     return interface_getMaximumMagLSFreq(hInt)/16e3f;
        case k_enableDiffEQhrtf:     return (interface_getEnableDiffEQ_HRTFs(hInt))>0.5 ? 1.0f : 0.0f;
        case k_enableDiffEQatf:     return (interface_getEnableDiffEQ_ATFs(hInt))>0.5 ? 1.0f : 0.0f;
            
		default: return 0.0f;
	}
}

int PluginProcessor::getNumParameters()
{
	return k_NumOfParameters;
}

const String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

const String PluginProcessor::getParameterName (int index)
{
    switch (index) {
        case k_listenerYaw:         return "yaw";
        case k_listenerPitch:       return "pitch";
        case k_listenerRoll:        return "roll";
        case k_listenerX:           return "X";
        case k_listenerY:           return "Y";
        case k_listenerZ:           return "Z";
        case k_lin2parBalance:      return "lin2parBalance";
        case k_dir2diffBalance:     return "dir2diffBalance";
        case k_enableSourceDirectivity: return "enableSourceD";
        case k_3dofOR6dof:          return "DoF";
        case k_distanceMapMode:     return "DistMapMode";
        case k_favour2D:         return "favour2D";
        case k_enableEPbeams:    return "enableEPbeams";
        case k_bsmMaxFreq:       return "bsmMaxFreq";
        case k_analysisMaxFreq:  return "analysisMaxFreq";
        case k_maglsMaxFreq:     return "maglsMaxFreq";
        case k_enableDiffEQhrtf:     return "enableDiffEQhrtf";
        case k_enableDiffEQatf:     return "enableDiffEQatf";
        default: return "NULL";
	}
}

const String PluginProcessor::getParameterText(int index)
{
    switch (index) {
        case k_listenerYaw:          return String(interface_getYaw(hInt));
        case k_listenerPitch:        return String(interface_getPitch(hInt));
        case k_listenerRoll:         return String(interface_getRoll(hInt));
        case k_listenerX:            return String(interface_getX(hInt));
        case k_listenerY:            return String(interface_getY(hInt));
        case k_listenerZ:            return String(interface_getZ(hInt));
        case k_lin2parBalance:       return String(interface_getLinear2ParametricBalance(hInt));
        case k_dir2diffBalance:      return String(interface_getStreamBalanceAllBands(hInt));
        case k_enableSourceDirectivity: return interface_getEnableSourceDirectivity(hInt) ? "enabled" : "disabled";
        case k_3dofOR6dof:           return interface_getDOFoption(hInt) == CORE_6DOF ? "6" : "3";
        case k_distanceMapMode:
            switch(interface_getDistanceMapMode(hInt)){
                case INTERFACE_DISTANCE_MAP_USE_PARAM: return "use slider";
                case INTERFACE_DISTANCE_MAP_1SRC:      return "1src";
                case INTERFACE_DISTANCE_MAP_2SRC:      return "2src";
                case INTERFACE_DISTANCE_MAP_3SRC:      return "3src";
            }
            
        case k_favour2D:         return (interface_getFavour2Daccuracy(hInt)) ? "yes" : "no";
        case k_enableEPbeams:    return (interface_getEnableEPbeamformers(hInt)) ? "enabled" : "disabled";
        case k_bsmMaxFreq:       return String(interface_getMaximumBSMFreq(hInt));
        case k_analysisMaxFreq:  return String(interface_getMaximumAnalysisFreq(hInt));
        case k_maglsMaxFreq:     return String(interface_getMaximumMagLSFreq(hInt));
        case k_enableDiffEQhrtf:     return (interface_getEnableDiffEQ_HRTFs(hInt)) ? "enabled" : "disabled";
        case k_enableDiffEQatf:     return (interface_getEnableDiffEQ_ATFs(hInt)) ? "enabled" : "disabled";
            
        default: return "NULL";
    }
}

const String PluginProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String PluginProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

const String PluginProcessor::getProgramName (int /*index*/)
{
    return String();
}

bool PluginProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool PluginProcessor::isOutputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& /*layouts*/) const
{
    return true;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

void PluginProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    nHostBlockSize = samplesPerBlock;
    nNumInputs =  getTotalNumInputChannels();
    nNumOutputs = getTotalNumOutputChannels();
    nSampleRate = (int)(sampleRate + 0.5);
    
    interface_init(hInt, sampleRate);
    AudioProcessor::setLatencySamples(interface_getProcessingDelay(hInt));
}

void PluginProcessor::releaseResources()
{
}

void PluginProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{
    int nCurrentBlockSize = nHostBlockSize = buffer.getNumSamples();
    nNumInputs = jmin(getTotalNumInputChannels(), buffer.getNumChannels());
    nNumOutputs = jmin(getTotalNumOutputChannels(), buffer.getNumChannels());
    float** bufferData = buffer.getArrayOfWritePointers(); 
    float* pFrameData[INTERFACE_MAX_NUM_CHANNELS];
    int frameSize = interface_getFrameSize();

    if(nCurrentBlockSize % frameSize == 0){ /* divisible by frame size */
        for(int frame = 0; frame < nCurrentBlockSize/frameSize; frame++) {
            for(int ch = 0; ch < buffer.getNumChannels(); ch++)
                pFrameData[ch] = &bufferData[ch][frame*frameSize];

            /* perform processing */
            interface_process(hInt, pFrameData, pFrameData, nNumInputs, nNumOutputs, frameSize);
        }
    }
    else
        buffer.clear();
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; 
}

AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (this);
}

//==============================================================================
void PluginProcessor::getStateInformation (MemoryBlock& destData)
{
	/* Create an outer XML element.. */ 
	XmlElement xml("PROPOSEDRENDERERAUDIOPLUGINSETTINGS");
 
    for(int band=0; band<interface_getNumberOfBands(hInt); band++){
        xml.setAttribute("Balance"+String(band), interface_getStreamBalance(hInt, band));
    }
    xml.setAttribute("analysisAveraging", String(interface_getAnalysisAveraging(hInt)));
    xml.setAttribute("synthesisAveraging", String(interface_getSynthesisAveraging(hInt)));
    
    xml.setAttribute("YAW", interface_getYaw(hInt));
    xml.setAttribute("PITCH", interface_getPitch(hInt));
    xml.setAttribute("ROLL", interface_getRoll(hInt));
    xml.setAttribute("FLIP_YAW", interface_getFlipYaw(hInt));
    xml.setAttribute("FLIP_PITCH", interface_getFlipPitch(hInt));
    xml.setAttribute("FLIP_ROLL", interface_getFlipRoll(hInt));
    xml.setAttribute("X", interface_getX(hInt));
    xml.setAttribute("Y", interface_getY(hInt));
    xml.setAttribute("Z", interface_getZ(hInt));
    xml.setAttribute("FLIP_X", interface_getFlipX(hInt));
    xml.setAttribute("FLIP_Y", interface_getFlipY(hInt));
    xml.setAttribute("FLIP_Z", interface_getFlipZ(hInt));
    
    xml.setAttribute("enabledDiffEQ_HRTFs", interface_getEnableDiffEQ_HRTFs(hInt));
    xml.setAttribute("enabledDiffEQ_ATFs", interface_getEnableDiffEQ_ATFs(hInt));
    
    xml.setAttribute("OSC_PORT", osc_port_ID);

    if(!interface_getUseDefaultHRIRsflag(hInt))
        xml.setAttribute("SofaFilePath_HRIR", String(interface_getSofaFilePathHRIR(hInt)));
    xml.setAttribute("SofaFilePath_MAIR", String(interface_getSofaFilePathMAIR(hInt)));
    
	copyXmlToBinary(xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	/* This getXmlFromBinary() function retrieves XML from the binary blob */
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
	if (xmlState != nullptr) {
		/* make sure that it's actually the correct type of XML object */
		if (xmlState->hasTagName("PROPOSEDRENDERERAUDIOPLUGINSETTINGS")) {

            for(int band=0; band<interface_getNumberOfBands(hInt); band++){
                if(xmlState->hasAttribute("Balance"+String(band)))
                    interface_setStreamBalance(hInt, (float)xmlState->getDoubleAttribute("Balance"+String(band),0), band);
            }
            if(xmlState->hasAttribute("SofaFilePath_MAIR")){
                String directory = xmlState->getStringAttribute("SofaFilePath_MAIR", "no_file");
                const char* new_cstring = (const char*)directory.toUTF8();
                interface_setSofaFilePathMAIR(hInt, new_cstring);
            }

            if(xmlState->hasAttribute("SofaFilePath_HRIR")){
                String directory = xmlState->getStringAttribute("SofaFilePath_HRIR", "no_file");
                const char* new_cstring = (const char*)directory.toUTF8();
                interface_setSofaFilePathHRIR(hInt, new_cstring);
            }

            if(xmlState->hasAttribute("analysisAveraging"))
                interface_setAnalysisAveraging(hInt, (float)xmlState->getDoubleAttribute("analysisAveraging",0.5));
            if(xmlState->hasAttribute("synthesisAveraging"))
                interface_setSynthesisAveraging(hInt, (float)xmlState->getDoubleAttribute("synthesisAveraging",0.5));
             
            if(xmlState->hasAttribute("YAW"))
                interface_setYaw(hInt, (float)xmlState->getDoubleAttribute("YAW", 0.0f));
            if(xmlState->hasAttribute("PITCH"))
                interface_setPitch(hInt, (float)xmlState->getDoubleAttribute("PITCH", 0.0f));
            if(xmlState->hasAttribute("ROLL"))
                interface_setRoll(hInt, (float)xmlState->getDoubleAttribute("ROLL", 0.0f));
            if(xmlState->hasAttribute("FLIP_YAW"))
                interface_setFlipYaw(hInt, xmlState->getIntAttribute("FLIP_YAW", 0));
            if(xmlState->hasAttribute("FLIP_PITCH"))
                interface_setFlipPitch(hInt, xmlState->getIntAttribute("FLIP_PITCH", 0));
            if(xmlState->hasAttribute("FLIP_ROLL"))
                interface_setFlipRoll(hInt, xmlState->getIntAttribute("FLIP_ROLL", 0));

            if (xmlState->hasAttribute("X"))
                interface_setX(hInt, (float)xmlState->getDoubleAttribute("X", 0.0f));
            if (xmlState->hasAttribute("Y"))
                interface_setY(hInt, (float)xmlState->getDoubleAttribute("Y", 0.0f));
            if (xmlState->hasAttribute("Z"))
                interface_setZ(hInt, (float)xmlState->getDoubleAttribute("Z", 0.0f));
            if (xmlState->hasAttribute("FLIP_X"))
                interface_setFlipX(hInt, xmlState->getIntAttribute("FLIP_X", 0));
            if (xmlState->hasAttribute("FLIP_Y"))
                interface_setFlipY(hInt, xmlState->getIntAttribute("FLIP_Y", 0));
            if (xmlState->hasAttribute("FLIP_Z"))
                interface_setFlipZ(hInt, xmlState->getIntAttribute("FLIP_Z", 0));
            
            if(xmlState->hasAttribute("enabledDiffEQ_HRTFs"))
                interface_setEnableDiffEQ_HRTFs(hInt, xmlState->getIntAttribute("enabledDiffEQ_HRTFs", 0));
            if(xmlState->hasAttribute("enabledDiffEQ_ATFs"))
                interface_setEnableDiffEQ_ATFs(hInt, xmlState->getIntAttribute("enabledDiffEQ_ATFs", 0));
            
            if(xmlState->hasAttribute("OSC_PORT")){
                osc_port_ID = xmlState->getIntAttribute("OSC_PORT", DEFAULT_OSC_PORT);
                osc.connect(osc_port_ID);
            }
            

            interface_refreshSettings(hInt);
        }
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

