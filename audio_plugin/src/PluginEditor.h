/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.2

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.
 
  Copyright (c) Meta Platforms, Inc. All Rights Reserved
  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
/**
 * @file PluginEditor.h
 * @brief JUCE audio plugin example using the interface to the proposed core
 *
 * Mostly derived/copied from the audio plugin code found here:
 * https://github.com/jananifernandez/HADES
 * Copyright (c) 2021 - Janani Fernandez & Leo McCormack,  (GPLv3 License)
 * And from here:
 * https://github.com/leomccormack/SPARTA
 * Copyright (c) 2018-2022 - Leo McCormack,  (GPLv3 License)
 *
 * @author Leo McCormack
 * @date 10th August 2022
 */


#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "ListenerWindow.h"
#include "../resources/log2dSlider.h"
#include "../resources/ProposedLookAndFeel.h"

typedef enum _POSSIBLE_WARNINGS{
    k_warning_none,
    k_warning_frameSize,
    k_warning_supported_fs,
    k_warning_mismatch_fs,
    k_warning_osc_connection_fail,
    k_warning_NinputCH,
    k_warning_NoutputCH
}POSSIBLE_WARNINGS;



//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class PluginEditor  : public AudioProcessorEditor,
                      public MultiTimer,
                      private FilenameComponentListener,
                      public juce::Slider::Listener,
                      public juce::Button::Listener,
                      public juce::ComboBox::Listener
{
public:
    //==============================================================================
    PluginEditor (PluginProcessor* ownerFilter);
    ~PluginEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.


    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    PluginProcessor* hVst;
    void* hInt;
    void timerCallback(int timerID) override;
    double progress = 0.0;
    ProgressBar progressbar;

    /* Look and Feel */
    ProposedLookAndFeel LAF;

    /* Other GUI component handles */
    std::unique_ptr<log2dSlider> streamBalance2dSlider;
    std::unique_ptr<ListenerWindow> translationWindow;

    /* openGL */
#ifndef PLUGIN_EDITOR_DISABLE_OPENGL
    std::unique_ptr<OpenGLGraphicsContextCustomShader> shader;
    OpenGLContext openGLContext;
#endif

    /* sofa file loading */
    FilenameComponent fileChooserMAIR, fileChooserHRIR;
    void filenameComponentChanged (FilenameComponent* fileComponentChanged) override  {
        if(fileComponentChanged == &fileChooserMAIR){
            File currentSOFAFile = fileChooserMAIR.getCurrentFile();
            String directory = currentSOFAFile.getFullPathName();
            currentSOFAFile = currentSOFAFile.getChildFile(directory);
            directory = currentSOFAFile.getFullPathName();
            const char* new_cstring = (const char*)directory.toUTF8();
            interface_setSofaFilePathMAIR(hInt, new_cstring);
        }
        else if (fileComponentChanged == &fileChooserHRIR){
            File currentSOFAFile = fileChooserHRIR.getCurrentFile();
            String directory = currentSOFAFile.getFullPathName();
            currentSOFAFile = currentSOFAFile.getChildFile(directory);
            directory = currentSOFAFile.getFullPathName();
            const char* new_cstring = (const char*)directory.toUTF8();
            interface_setSofaFilePathHRIR(hInt, new_cstring);
        }
    }

    /* warnings */
    POSSIBLE_WARNINGS currentWarning;

    /* tooltips */
    SharedResourcePointer<TooltipWindow> tipWindow;
    std::unique_ptr<juce::ComboBox> pluginDescription; /* Dummy combo box to provide plugin description tooltip */
    HyperlinkButton publicationLink { "(Related Plugins)", { "https://leomccormack.github.io/sparta-site/" } };

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Slider> s_diff2dir;
    std::unique_ptr<juce::Label> label_IR_fs_array;
    std::unique_ptr<juce::Label> label_DAW_fs_array;
    std::unique_ptr<juce::Label> label_N_nMics;
    std::unique_ptr<juce::Label> label_N_nDirs_array;
    std::unique_ptr<juce::Label> label_IR_length_array;
    std::unique_ptr<juce::ToggleButton> TBuseDefaultHRIRs;
    std::unique_ptr<juce::Label> label_HRIR_fs_bin;
    std::unique_ptr<juce::Label> label_DAW_fs_bin;
    std::unique_ptr<juce::Label> label_N_nDirs_bin;
    std::unique_ptr<juce::Label> label_IR_length_bin;
    std::unique_ptr<juce::Slider> SL_analysis_avg;
    std::unique_ptr<juce::Slider> SL_synthesis_avg;
    std::unique_ptr<juce::TextEditor> te_oscport;
    std::unique_ptr<juce::Slider> s_yaw;
    std::unique_ptr<juce::Slider> s_pitch;
    std::unique_ptr<juce::Slider> s_roll;
    std::unique_ptr<juce::ToggleButton> t_flipYaw;
    std::unique_ptr<juce::ToggleButton> t_flipPitch;
    std::unique_ptr<juce::ToggleButton> t_flipRoll;
    std::unique_ptr<juce::Slider> s_x;
    std::unique_ptr<juce::Slider> s_y;
    std::unique_ptr<juce::Slider> s_z;
    std::unique_ptr<juce::ToggleButton> t_flipX;
    std::unique_ptr<juce::ToggleButton> t_flipY;
    std::unique_ptr<juce::ToggleButton> t_flipZ;
    std::unique_ptr<juce::ComboBox> CBdofOption;
    std::unique_ptr<juce::Slider> SL_source_distance;
    std::unique_ptr<juce::ComboBox> CBListenerViewOption;
    std::unique_ptr<juce::Slider> SL_BSM2Ambi_cutoff;
    std::unique_ptr<juce::Slider> SL_maxAnalysisFreq;
    std::unique_ptr<juce::Slider> SL_linear2parametric;
    std::unique_ptr<juce::ToggleButton> TBenableSourceDirectivity;
    std::unique_ptr<juce::ComboBox> CBdistMapOption;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

