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

//[Headers] You can add your own extra header files here...
/**
 * @file PluginEditor.cpp
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

//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//[/MiscUserDefs]

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter), progressbar(progress), fileChooserMAIR ("File", File(), true, false, false,
      "*.sofa;*.nc;", String(),
      "Load SOFA File"), fileChooserHRIR ("File", File(), true, false, false,
      "*.sofa;*.nc;", String(),
      "Load SOFA File")
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    s_diff2dir.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_diff2dir.get());
    s_diff2dir->setRange (0, 2, 0.01);
    s_diff2dir->setSliderStyle (juce::Slider::LinearVertical);
    s_diff2dir->setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    s_diff2dir->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    s_diff2dir->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6d));
    s_diff2dir->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_diff2dir->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_diff2dir->addListener (this);

    s_diff2dir->setBounds (384, 354, 40, 74);

    label_IR_fs_array.reset (new juce::Label ("new label",
                                              juce::String()));
    addAndMakeVisible (label_IR_fs_array.get());
    label_IR_fs_array->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_IR_fs_array->setJustificationType (juce::Justification::centredLeft);
    label_IR_fs_array->setEditable (false, false, false);
    label_IR_fs_array->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_IR_fs_array->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_IR_fs_array->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_IR_fs_array->setBounds (164, 166, 51, 20);

    label_DAW_fs_array.reset (new juce::Label ("new label",
                                               juce::String()));
    addAndMakeVisible (label_DAW_fs_array.get());
    label_DAW_fs_array->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_DAW_fs_array->setJustificationType (juce::Justification::centredLeft);
    label_DAW_fs_array->setEditable (false, false, false);
    label_DAW_fs_array->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_DAW_fs_array->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_DAW_fs_array->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_DAW_fs_array->setBounds (164, 189, 51, 20);

    label_N_nMics.reset (new juce::Label ("new label",
                                          juce::String()));
    addAndMakeVisible (label_N_nMics.get());
    label_N_nMics->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_N_nMics->setJustificationType (juce::Justification::centredLeft);
    label_N_nMics->setEditable (false, false, false);
    label_N_nMics->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_N_nMics->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_N_nMics->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_N_nMics->setBounds (164, 97, 51, 20);

    label_N_nDirs_array.reset (new juce::Label ("new label",
                                                juce::String()));
    addAndMakeVisible (label_N_nDirs_array.get());
    label_N_nDirs_array->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_N_nDirs_array->setJustificationType (juce::Justification::centredLeft);
    label_N_nDirs_array->setEditable (false, false, false);
    label_N_nDirs_array->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_N_nDirs_array->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_N_nDirs_array->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_N_nDirs_array->setBounds (164, 120, 51, 20);

    label_IR_length_array.reset (new juce::Label ("new label",
                                                  juce::String()));
    addAndMakeVisible (label_IR_length_array.get());
    label_IR_length_array->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_IR_length_array->setJustificationType (juce::Justification::centredLeft);
    label_IR_length_array->setEditable (false, false, false);
    label_IR_length_array->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_IR_length_array->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_IR_length_array->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_IR_length_array->setBounds (164, 143, 51, 20);

    TBuseDefaultHRIRs.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (TBuseDefaultHRIRs.get());
    TBuseDefaultHRIRs->setButtonText (juce::String());
    TBuseDefaultHRIRs->addListener (this);

    TBuseDefaultHRIRs->setBounds (400, 91, 32, 24);

    label_HRIR_fs_bin.reset (new juce::Label ("new label",
                                              juce::String()));
    addAndMakeVisible (label_HRIR_fs_bin.get());
    label_HRIR_fs_bin->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_HRIR_fs_bin->setJustificationType (juce::Justification::centredLeft);
    label_HRIR_fs_bin->setEditable (false, false, false);
    label_HRIR_fs_bin->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_HRIR_fs_bin->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_HRIR_fs_bin->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_HRIR_fs_bin->setBounds (377, 166, 51, 20);

    label_DAW_fs_bin.reset (new juce::Label ("new label",
                                             juce::String()));
    addAndMakeVisible (label_DAW_fs_bin.get());
    label_DAW_fs_bin->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_DAW_fs_bin->setJustificationType (juce::Justification::centredLeft);
    label_DAW_fs_bin->setEditable (false, false, false);
    label_DAW_fs_bin->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_DAW_fs_bin->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_DAW_fs_bin->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_DAW_fs_bin->setBounds (377, 189, 51, 20);

    label_N_nDirs_bin.reset (new juce::Label ("new label",
                                              juce::String()));
    addAndMakeVisible (label_N_nDirs_bin.get());
    label_N_nDirs_bin->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_N_nDirs_bin->setJustificationType (juce::Justification::centredLeft);
    label_N_nDirs_bin->setEditable (false, false, false);
    label_N_nDirs_bin->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_N_nDirs_bin->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_N_nDirs_bin->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_N_nDirs_bin->setBounds (377, 121, 51, 20);

    label_IR_length_bin.reset (new juce::Label ("new label",
                                                juce::String()));
    addAndMakeVisible (label_IR_length_bin.get());
    label_IR_length_bin->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    label_IR_length_bin->setJustificationType (juce::Justification::centredLeft);
    label_IR_length_bin->setEditable (false, false, false);
    label_IR_length_bin->setColour (juce::Label::outlineColourId, juce::Colour (0x68a3a2a2));
    label_IR_length_bin->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label_IR_length_bin->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    label_IR_length_bin->setBounds (377, 143, 51, 20);

    SL_analysis_avg.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (SL_analysis_avg.get());
    SL_analysis_avg->setRange (0, 1, 0.01);
    SL_analysis_avg->setSliderStyle (juce::Slider::LinearHorizontal);
    SL_analysis_avg->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
    SL_analysis_avg->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    SL_analysis_avg->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    SL_analysis_avg->addListener (this);

    SL_analysis_avg->setBounds (328, 218, 98, 22);

    SL_synthesis_avg.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (SL_synthesis_avg.get());
    SL_synthesis_avg->setRange (0, 1, 0.01);
    SL_synthesis_avg->setSliderStyle (juce::Slider::LinearHorizontal);
    SL_synthesis_avg->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
    SL_synthesis_avg->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    SL_synthesis_avg->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    SL_synthesis_avg->addListener (this);

    SL_synthesis_avg->setBounds (328, 242, 98, 22);

    te_oscport.reset (new juce::TextEditor ("new text editor"));
    addAndMakeVisible (te_oscport.get());
    te_oscport->setMultiLine (false);
    te_oscport->setReturnKeyStartsNewLine (false);
    te_oscport->setReadOnly (false);
    te_oscport->setScrollbarsShown (true);
    te_oscport->setCaretVisible (false);
    te_oscport->setPopupMenuEnabled (true);
    te_oscport->setColour (juce::TextEditor::textColourId, juce::Colours::white);
    te_oscport->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00ffffff));
    te_oscport->setColour (juce::TextEditor::outlineColourId, juce::Colour (0x68a3a2a2));
    te_oscport->setText (juce::String());

    te_oscport->setBounds (133, 244, 88, 18);

    s_yaw.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_yaw.get());
    s_yaw->setRange (-180, 180, 0.01);
    s_yaw->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_yaw->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_yaw->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff315b6d));
    s_yaw->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff5c5d5e));
    s_yaw->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_yaw->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_yaw->addListener (this);

    s_yaw->setBounds (635, 365, 61, 56);

    s_pitch.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_pitch.get());
    s_pitch->setRange (-180, 180, 0.01);
    s_pitch->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_pitch->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_pitch->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff315b6e));
    s_pitch->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff5c5d5e));
    s_pitch->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_pitch->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_pitch->addListener (this);

    s_pitch->setBounds (698, 365, 61, 56);

    s_roll.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_roll.get());
    s_roll->setRange (-180, 180, 0.01);
    s_roll->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_roll->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_roll->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff315b6d));
    s_roll->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff5c5d5e));
    s_roll->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_roll->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_roll->addListener (this);

    s_roll->setBounds (761, 365, 61, 56);

    t_flipYaw.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (t_flipYaw.get());
    t_flipYaw->setButtonText (juce::String());
    t_flipYaw->addListener (this);

    t_flipYaw->setBounds (668, 420, 24, 23);

    t_flipPitch.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (t_flipPitch.get());
    t_flipPitch->setButtonText (juce::String());
    t_flipPitch->addListener (this);

    t_flipPitch->setBounds (731, 420, 24, 23);

    t_flipRoll.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (t_flipRoll.get());
    t_flipRoll->setButtonText (juce::String());
    t_flipRoll->addListener (this);

    t_flipRoll->setBounds (794, 420, 24, 23);

    s_x.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_x.get());
    s_x->setRange (-180, 180, 0.01);
    s_x->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_x->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_x->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff315b6d));
    s_x->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff5c5d5e));
    s_x->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_x->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_x->addListener (this);

    s_x->setBounds (446, 365, 61, 56);

    s_y.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_y.get());
    s_y->setRange (-180, 180, 0.01);
    s_y->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_y->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_y->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff315b6e));
    s_y->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff5c5d5e));
    s_y->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_y->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_y->addListener (this);

    s_y->setBounds (509, 365, 61, 56);

    s_z.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_z.get());
    s_z->setRange (-180, 180, 0.01);
    s_z->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_z->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_z->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff315b6d));
    s_z->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff5c5d5e));
    s_z->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_z->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_z->addListener (this);

    s_z->setBounds (572, 365, 61, 56);

    t_flipX.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (t_flipX.get());
    t_flipX->setButtonText (juce::String());
    t_flipX->addListener (this);

    t_flipX->setBounds (477, 420, 24, 23);

    t_flipY.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (t_flipY.get());
    t_flipY->setButtonText (juce::String());
    t_flipY->addListener (this);

    t_flipY->setBounds (540, 420, 24, 23);

    t_flipZ.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (t_flipZ.get());
    t_flipZ->setButtonText (juce::String());
    t_flipZ->addListener (this);

    t_flipZ->setBounds (603, 420, 24, 23);

    CBdofOption.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (CBdofOption.get());
    CBdofOption->setEditableText (false);
    CBdofOption->setJustificationType (juce::Justification::centredLeft);
    CBdofOption->setTextWhenNothingSelected (TRANS("Default"));
    CBdofOption->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    CBdofOption->addListener (this);

    CBdofOption->setBounds (128, 221, 96, 18);

    SL_source_distance.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (SL_source_distance.get());
    SL_source_distance->setRange (0.5, 3, 0.01);
    SL_source_distance->setSliderStyle (juce::Slider::LinearHorizontal);
    SL_source_distance->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
    SL_source_distance->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    SL_source_distance->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    SL_source_distance->addListener (this);

    SL_source_distance->setBounds (128, 288, 98, 22);

    CBListenerViewOption.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (CBListenerViewOption.get());
    CBListenerViewOption->setEditableText (false);
    CBListenerViewOption->setJustificationType (juce::Justification::centredLeft);
    CBListenerViewOption->setTextWhenNothingSelected (TRANS("Default"));
    CBListenerViewOption->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    CBListenerViewOption->addListener (this);

    CBListenerViewOption->setBounds (740, 38, 96, 14);

    SL_BSM2Ambi_cutoff.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (SL_BSM2Ambi_cutoff.get());
    SL_BSM2Ambi_cutoff->setRange (0, 16000, 1);
    SL_BSM2Ambi_cutoff->setSliderStyle (juce::Slider::LinearHorizontal);
    SL_BSM2Ambi_cutoff->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
    SL_BSM2Ambi_cutoff->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    SL_BSM2Ambi_cutoff->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    SL_BSM2Ambi_cutoff->addListener (this);

    SL_BSM2Ambi_cutoff->setBounds (328, 265, 98, 22);

    SL_maxAnalysisFreq.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (SL_maxAnalysisFreq.get());
    SL_maxAnalysisFreq->setRange (0, 16000, 1);
    SL_maxAnalysisFreq->setSliderStyle (juce::Slider::LinearHorizontal);
    SL_maxAnalysisFreq->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
    SL_maxAnalysisFreq->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    SL_maxAnalysisFreq->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    SL_maxAnalysisFreq->addListener (this);

    SL_maxAnalysisFreq->setBounds (328, 289, 98, 22);

    SL_linear2parametric.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (SL_linear2parametric.get());
    SL_linear2parametric->setRange (0, 1, 0.01);
    SL_linear2parametric->setSliderStyle (juce::Slider::LinearHorizontal);
    SL_linear2parametric->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
    SL_linear2parametric->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    SL_linear2parametric->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    SL_linear2parametric->addListener (this);

    SL_linear2parametric->setBounds (328, 312, 98, 22);

    TBenableSourceDirectivity.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (TBenableSourceDirectivity.get());
    TBenableSourceDirectivity->setButtonText (juce::String());
    TBenableSourceDirectivity->addListener (this);

    TBenableSourceDirectivity->setBounds (194, 310, 32, 24);

    CBdistMapOption.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (CBdistMapOption.get());
    CBdistMapOption->setEditableText (false);
    CBdistMapOption->setJustificationType (juce::Justification::centredLeft);
    CBdistMapOption->setTextWhenNothingSelected (TRANS("Default"));
    CBdistMapOption->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    CBdistMapOption->addListener (this);

    CBdistMapOption->setBounds (128, 266, 96, 18);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (842, 448);


    //[Constructor] You can add your own custom stuff here..

    /* handles */
    hVst = ownerFilter;
    hInt = hVst->getFXHandle();

    /* init OpenGL */
#ifndef PLUGIN_EDITOR_DISABLE_OPENGL
    openGLContext.setMultisamplingEnabled(true);
    openGLContext.attachTo(*this);
#endif

    /* Look and Feel */
    LAF.setDefaultColours();
    setLookAndFeel(&LAF);

    /* add combobox options */
    CBdofOption->addItem (TRANS("0DoF"), CORE_0DOF);
    CBdofOption->addItem (TRANS("1DoF (y)"), CORE_1DOF_ROTATIONS);
    CBdofOption->addItem (TRANS("3DoF (ypr)"), CORE_3DOF_ROTATIONS);
    CBdofOption->addItem (TRANS("3DoF (xyz)"), CORE_3DOF_TRANSLATIONS);
    CBdofOption->addItem (TRANS("6DoF"), CORE_6DOF);
    CBListenerViewOption->addItem (TRANS("top-down"), LISTENER_WINDOW_TOPDOWN_VIEW);
    CBListenerViewOption->addItem (TRANS("forwards"), LISTENER_WINDOW_FORWARDS_VIEW);
    CBdistMapOption->addItem(TRANS("Use Slider"), INTERFACE_DISTANCE_MAP_USE_PARAM);
    CBdistMapOption->addItem(TRANS("1src"), INTERFACE_DISTANCE_MAP_1SRC);
    CBdistMapOption->addItem(TRANS("2src"), INTERFACE_DISTANCE_MAP_2SRC);
    CBdistMapOption->addItem(TRANS("3src"), INTERFACE_DISTANCE_MAP_3SRC);

    /* create 2d Sliders */
    streamBalance2dSlider.reset (new log2dSlider(360, 62, 100, 20e3, 0, 2, 2));
    addAndMakeVisible (streamBalance2dSlider.get());
    streamBalance2dSlider->setAlwaysOnTop(true);
    streamBalance2dSlider->setTopLeftPosition(25, 360);
    streamBalance2dSlider->setRefreshValuesFLAG(true);

    /* create listener window */
    translationWindow.reset (new ListenerWindow(ownerFilter));
    addAndMakeVisible (translationWindow.get());
    translationWindow->setAlwaysOnTop(true);
    translationWindow->setBounds(440, 58, 390, 290);
    //translationWindow->setTopLeftPosition(440, 58);

    /* file loaders */
    addAndMakeVisible (fileChooserMAIR);
    fileChooserMAIR.addListener (this);
    fileChooserMAIR.setBounds (20, 65, 198, 20);
    StringArray filenames;
    filenames.add(interface_getSofaFilePathMAIR(hInt));
    fileChooserMAIR.setRecentlyUsedFilenames(filenames);
    fileChooserMAIR.setFilenameIsEditable(true);
    addAndMakeVisible (fileChooserHRIR);
    fileChooserHRIR.addListener (this);
    fileChooserHRIR.setBounds (229, 65, 198, 20);
    StringArray filenames2;
    filenames2.add(interface_getSofaFilePathMAIR(hInt));
    fileChooserHRIR.setRecentlyUsedFilenames(filenames2);
    fileChooserHRIR.setFilenameIsEditable(true);

    /* ProgressBar */
    progress = 0.0;
    progressbar.setBounds(getLocalBounds().getCentreX()-175, getLocalBounds().getCentreY()-17, 350, 35);
    progressbar.ProgressBar::setAlwaysOnTop(true);
    progressbar.setColour(ProgressBar::backgroundColourId, Colours::gold);
    progressbar.setColour(ProgressBar::foregroundColourId, Colours::white);

    /* grab current parameter settings */
    s_diff2dir->setValue(interface_getStreamBalanceAllBands(hInt), dontSendNotification);
    TBuseDefaultHRIRs->setToggleState((bool)interface_getUseDefaultHRIRsflag(hInt), dontSendNotification);
    SL_analysis_avg->setValue((double)interface_getAnalysisAveraging(hInt), dontSendNotification);
    SL_synthesis_avg->setValue((double)interface_getSynthesisAveraging(hInt), dontSendNotification);
    label_IR_fs_array->setText(String(interface_getIRsamplerateArray(hInt)), dontSendNotification);
    label_DAW_fs_array->setText(String(interface_getDAWsamplerate(hInt)), dontSendNotification);
    label_N_nMics->setText(String(interface_getNmicsArray(hInt)), dontSendNotification);
    label_N_nDirs_array->setText(String(interface_getNDirsArray(hInt)), dontSendNotification);
    label_IR_length_array->setText(String(interface_getIRlengthArray(hInt)), dontSendNotification);
    label_HRIR_fs_bin->setText(String(interface_getIRsamplerateBin(hInt)), dontSendNotification);
    label_DAW_fs_bin->setText(String(interface_getDAWsamplerate(hInt)), dontSendNotification);
    label_N_nDirs_bin->setText(String(interface_getNDirsBin(hInt)), dontSendNotification);
    label_IR_length_bin->setText(String(interface_getIRlengthBin(hInt)), dontSendNotification);
    s_yaw->setValue(interface_getYaw(hInt), dontSendNotification);
    s_pitch->setValue(interface_getPitch(hInt), dontSendNotification);
    s_roll->setValue(interface_getRoll(hInt), dontSendNotification);
    t_flipYaw->setToggleState((bool)interface_getFlipYaw(hInt), dontSendNotification);
    t_flipPitch->setToggleState((bool)interface_getFlipPitch(hInt), dontSendNotification);
    t_flipRoll->setToggleState((bool)interface_getFlipRoll(hInt), dontSendNotification);
    te_oscport->setText(String(hVst->getOscPortID()), dontSendNotification);
    CBListenerViewOption->setSelectedId((int)translationWindow->getViewPoint());
    SL_source_distance->setValue(interface_getSourceDistance(hInt), dontSendNotification);
    float range = INTERFACE_PERIMETER_DISTANCE_M*1.5;
    s_x->setRange(-range, range, 0.01f);
    s_y->setRange(-range, range, 0.01f);
    s_z->setRange(-range, range, 0.01f);
    s_x->setValue(interface_getX(hInt), dontSendNotification);
    s_y->setValue(interface_getY(hInt), dontSendNotification);
    s_z->setValue(interface_getZ(hInt), dontSendNotification);
    CBdofOption->setSelectedId((int)interface_getDOFoption(hInt), dontSendNotification);
    TBenableSourceDirectivity->setToggleState((bool)interface_getEnableSourceDirectivity(hInt), dontSendNotification);
    SL_linear2parametric->setValue(interface_getLinear2ParametricBalance(hInt), dontSendNotification);
    CBdistMapOption->setSelectedId((int)interface_getDistanceMapMode(hInt), dontSendNotification);

    /* tooltips */
//    CBdecOrder->setTooltip("Decoding order. Note that the plug-in will require (order+1)^2 Ambisonic (spherical harmonic) signals as input.");
//    fileChooser.setTooltip("Optionally, a custom HRIR set may be loaded via the SOFA standard. Note that if the plug-in fails to load the specified .sofa file, it will revert to the default HRIR data.");
//    SL_diffusionLevel->setTooltip("Slider to control the decorrelation of the ambient stream (0: none, 1: fully decorrelated)");
//    s_cov_avg->setTooltip("Covariance matrix averaging coefficient (one-pole).");
//    s_synth_avg->setTooltip("Synthesis matrix averaging coefficient (one-pole).");
//    s_diff2dir->setTooltip("This sets the diffuse-to-direct balance for all frequencies (default is in the middle). Use the 2-D slider to change the balance for specific frequencies.");
//    s_decBal->setTooltip("This sets the decoding balance between linear (Ambisonics) and parametric (COMPASS) for all frequencies. Use the 2-D slider to change the balance for specific frequencies.");
//    CBchannelOrder->setTooltip("Ambisonic channel ordering convention (Note that AmbiX: ACN/SN3D).");
//    CBnormalisation->setTooltip("Ambisonic normalisation scheme (Note that AmbiX: ACN/SN3D).");
//    TBenableRotation->setTooltip("Enables/Disables sound-field rotation prior to decoding.");
//    s_yaw->setTooltip("Sets the 'Yaw' rotation angle (in degrees).");
//    s_pitch->setTooltip("Sets the 'Pitch' rotation angle (in degrees).");
//    s_roll->setTooltip("Sets the 'Roll' rotation angle (in degrees).");
//    t_flipYaw->setTooltip("Flips the sign (+/-) of the 'Yaw' rotation angle.");
//    t_flipPitch->setTooltip("Flips the sign (+/-) of the 'Pitch' rotation angle.");
//    t_flipRoll->setTooltip("Flips the sign (+/-) of the 'Roll' rotation angle.");
//    te_oscport->setTooltip("The OSC port at which to receive the rotation angles. To facilitate head-tracking, send the rotation angles (in degrees) to this port ID as a 3-element vector 'ypr[3]', following the yaw-pitch-roll convention.");
//    TBrpyFlag->setTooltip("If enabled, the plug-in will use the roll-pitch-yaw rotation order convention. If disabled, it will use the yaw-pitch-roll convention.");
//    label_N_dirs->setTooltip("Number of HRIR directions in the current HRIR set.");
//    label_N_Tri->setTooltip("Number of triangles found when computing the Convex Hull of the HRIR grid.");
//    label_HRIR_fs->setTooltip("Sampling rate used when measuring/modelling the HRIRs.");
//    label_DAW_fs->setTooltip("Current sampling rate, as dictated by the DAW/Host.");
//    CBdoaEstimator->setTooltip("Direction of arrival (DoA) estimator to use. Multiple Signal Classification (MUSIC), or Estimation of Signal Parameters via Rotational Invariance Techniques (ESPRIT). MUSIC is generally the more robust DoA estimator and is therefore the default, but the method relies on scanning a dense number of directions on the sphere to determine the source directions; therefore, it's quite computationally expensive. ESPRIT, on the other hand, does not rely on any scanning grid and instead directly extracts the source directions. This makes the method computationally efficient, with the penalty of (often slightly) less performance than MUSIC and is more restricted in the number of simultaneous directions it can estimate.");
//    CBprocMode->setTooltip("COMPASS formulation to use for processing.");
    /* tooltips */
    TBuseDefaultHRIRs->setTooltip("If this is 'ticked', the plug-in is using the default HRIR set from the Spatial_Audio_Framework.");
    s_yaw->setTooltip("Sets the 'Yaw' rotation angle (in degrees).");
    s_pitch->setTooltip("Sets the 'Pitch' rotation angle (in degrees).");
    s_roll->setTooltip("Sets the 'Roll' rotation angle (in degrees).");
    t_flipYaw->setTooltip("Flips the sign (+/-) of the 'Yaw' rotation angle.");
    t_flipPitch->setTooltip("Flips the sign (+/-) of the 'Pitch' rotation angle.");
    t_flipRoll->setTooltip("Flips the sign (+/-) of the 'Roll' rotation angle.");
    te_oscport->setTooltip("The OSC port at which to receive the rotation angles. To facilitate head-tracking, send the rotation angles (in degrees) to this port ID as a 3-element vector 'ypr[3]', following the yaw-pitch-roll convention.");

    /* Plugin description */
    pluginDescription.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (pluginDescription.get());
    pluginDescription->setBounds (0, 0, 200, 32);
    pluginDescription->setAlpha(0.0f);
    pluginDescription->setEnabled(false);
//    pluginDescription->setTooltip(TRANS(""));
    addAndMakeVisible (publicationLink);
    publicationLink.setColour (HyperlinkButton::textColourId, Colours::lightblue);
    publicationLink.setBounds(getBounds().getWidth()-80, 4, 80, 12);
    publicationLink.setJustificationType(Justification::centredLeft);

    /* Specify screen refresh rate */
    startTimer(TIMER_GUI_RELATED, 60);

    currentWarning = k_warning_none;

    //[/Constructor]
}

PluginEditor::~PluginEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    s_diff2dir = nullptr;
    label_IR_fs_array = nullptr;
    label_DAW_fs_array = nullptr;
    label_N_nMics = nullptr;
    label_N_nDirs_array = nullptr;
    label_IR_length_array = nullptr;
    TBuseDefaultHRIRs = nullptr;
    label_HRIR_fs_bin = nullptr;
    label_DAW_fs_bin = nullptr;
    label_N_nDirs_bin = nullptr;
    label_IR_length_bin = nullptr;
    SL_analysis_avg = nullptr;
    SL_synthesis_avg = nullptr;
    te_oscport = nullptr;
    s_yaw = nullptr;
    s_pitch = nullptr;
    s_roll = nullptr;
    t_flipYaw = nullptr;
    t_flipPitch = nullptr;
    t_flipRoll = nullptr;
    s_x = nullptr;
    s_y = nullptr;
    s_z = nullptr;
    t_flipX = nullptr;
    t_flipY = nullptr;
    t_flipZ = nullptr;
    CBdofOption = nullptr;
    SL_source_distance = nullptr;
    CBListenerViewOption = nullptr;
    SL_BSM2Ambi_cutoff = nullptr;
    SL_maxAnalysisFreq = nullptr;
    SL_linear2parametric = nullptr;
    TBenableSourceDirectivity = nullptr;
    CBdistMapOption = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    setLookAndFeel(nullptr);
//    decBalance2dSlider = nullptr;
//    streamBalance2dSlider = nullptr;
    //[/Destructor]
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colours::white);

    {
        int x = 0, y = 201, width = 842, height = 247;
        juce::Colour fillColour1 = juce::Colours::black, fillColour2 = juce::Colour (0xff1c1c1c);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             448.0f - 201.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             360.0f - 201.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        int x = 0, y = 30, width = 842, height = 171;
        juce::Colour fillColour1 = juce::Colours::black, fillColour2 = juce::Colour (0xff1c1c1c);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             32.0f - 30.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             80.0f - 30.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        float x = 1.0f, y = 2.0f, width = 840.0f, height = 31.0f;
        juce::Colour fillColour1 = juce::Colours::black, fillColour2 = juce::Colour (0xff1c1c1c);
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             0.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             fillColour2,
                                             842.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             false));
        g.fillRoundedRectangle (x, y, width, height, 5.000f);
        g.setColour (strokeColour);
        g.drawRoundedRectangle (x, y, width, height, 5.000f, 2.000f);
    }

    {
        int x = 223, y = 58, width = 213, height = 158;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 12, y = 58, width = 212, height = 158;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 12, y = 58, width = 212, height = 33;
        juce::Colour fillColour = juce::Colour (0x08f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 66, y = 33, width = 149, height = 30;
        juce::String text (TRANS("Load Array IRs"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 12, y = 335, width = 424, height = 105;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 21, y = 331, width = 331, height = 30;
        juce::String text (TRANS("Direct to Residual Balance Per Frequency Band"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 19, y = 415, width = 35, height = 30;
        juce::String text (TRANS("100"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 174, y = 415, width = 35, height = 30;
        juce::String text (TRANS("1k"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 328, y = 415, width = 35, height = 30;
        juce::String text (TRANS("10k"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 210, y = 415, width = 91, height = 30;
        juce::String text (TRANS("Frequency (Hz)"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 371, y = 415, width = 35, height = 30;
        juce::String text (TRANS("20k"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 413, y = 413, width = 27, height = 30;
        juce::String text (TRANS("Res"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (11.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 413, y = 342, width = 17, height = 30;
        juce::String text (TRANS("Dir"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (11.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 0, y = 0, width = 842, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 0, width = 2, height = 448;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 1298, y = 0, width = 2, height = 448;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 446, width = 1234, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 22, y = 91, width = 138, height = 30;
        juce::String text (TRANS("Number of Sensors:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 114, width = 170, height = 30;
        juce::String text (TRANS("Number of Directions:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 137, width = 162, height = 30;
        juce::String text (TRANS("IR length: "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 159, width = 162, height = 30;
        juce::String text (TRANS("IR SampleRate:  "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 184, width = 162, height = 30;
        juce::String text (TRANS("DAW SampleRate:  "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 12, y = 1, width = 120, height = 32;
        juce::String text (TRANS("6DoF |"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (18.80f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 72, y = 1, width = 106, height = 32;
        juce::String text (TRANS("Auraliser"));
        juce::Colour fillColour = juce::Colour (0xffff3636);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (18.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 290, y = 33, width = 149, height = 30;
        juce::String text (TRANS("Load HRIRs"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 223, y = 58, width = 213, height = 58;
        juce::Colour fillColour = juce::Colour (0x08f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 232, y = 88, width = 173, height = 30;
        juce::String text (TRANS("Use Default HRIR set:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 230, y = 115, width = 170, height = 30;
        juce::String text (TRANS("Number of Directions:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 230, y = 137, width = 162, height = 30;
        juce::String text (TRANS("HRIR length: "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 230, y = 161, width = 162, height = 30;
        juce::String text (TRANS("HRIR SampleRate:  "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 230, y = 184, width = 162, height = 30;
        juce::String text (TRANS("DAW SampleRate:  "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 12, y = 215, width = 424, height = 121;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 236, y = 214, width = 162, height = 30;
        juce::String text (TRANS("Analysis Avg:  "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 236, y = 238, width = 162, height = 30;
        juce::String text (TRANS("Synthesis Avg:  "));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 1232, y = 0, width = 2, height = 448;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 840, y = 0, width = 2, height = 448;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 654, y = 346, width = 49, height = 30;
        juce::String text (TRANS("\\ypr[0]"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 695, y = 346, width = 45, height = 30;
        juce::String text (TRANS("Pitch"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 750, y = 346, width = 54, height = 30;
        juce::String text (TRANS("Roll"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 751, y = 417, width = 62, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 623, y = 417, width = 62, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 687, y = 417, width = 62, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 618, y = 346, width = 64, height = 30;
        juce::String text (TRANS("Yaw"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 727, y = 346, width = 40, height = 30;
        juce::String text (TRANS("\\ypr[1]"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 784, y = 346, width = 40, height = 30;
        juce::String text (TRANS("\\ypr[2]"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 458, y = 346, width = 49, height = 30;
        juce::String text (TRANS("\\xyz[0]"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 504, y = 346, width = 45, height = 30;
        juce::String text (TRANS("Y"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 559, y = 346, width = 54, height = 30;
        juce::String text (TRANS("Z"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 427, y = 346, width = 64, height = 30;
        juce::String text (TRANS("X"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (12.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 531, y = 346, width = 40, height = 30;
        juce::String text (TRANS("\\xyz[1]"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 588, y = 346, width = 40, height = 30;
        juce::String text (TRANS("\\xyz[2]"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 560, y = 417, width = 62, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 432, y = 417, width = 62, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 496, y = 417, width = 62, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 22, y = 214, width = 162, height = 30;
        juce::String text (TRANS("Rendering Mode:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 238, width = 162, height = 30;
        juce::String text (TRANS("OSC Port:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 236, y = 262, width = 162, height = 30;
        juce::String text (TRANS("Max BSM (Hz):"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 236, y = 286, width = 162, height = 30;
        juce::String text (TRANS("Max Ana (Hz):"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 283, width = 162, height = 30;
        juce::String text (TRANS("Source dist (m):"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 236, y = 307, width = 162, height = 30;
        juce::String text (TRANS("lin2par balance:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 307, width = 162, height = 30;
        juce::String text (TRANS("Enable Source Directivity:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 22, y = 260, width = 162, height = 30;
        juce::String text (TRANS("Dist Map Option:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (13.50f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    //[UserPaint] Add your own custom painting code here..

    g.setColour(Colours::white);
	g.setFont(Font(11.00f, Font::plain));
	g.drawText(TRANS("Ver ") + JucePlugin_VersionString + BUILD_VER_SUFFIX + TRANS(", Build Date ") + __DATE__ + TRANS(" "),
		195, 16, 530, 11,
		Justification::centredLeft, true);

    /* display warning message */
    g.setColour(Colours::red);
    g.setFont(Font(11.00f, Font::plain));
    switch (currentWarning){
        case k_warning_none:
            break;
        case k_warning_frameSize:
            g.drawText(TRANS("Set frame size to multiple of ") + String(interface_getFrameSize()),
                       getBounds().getWidth()-225, 18, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_supported_fs:
            g.drawText(TRANS("Sample rate (") + String(interface_getDAWsamplerate(hInt)) + TRANS(") is unsupported"),
                       getBounds().getWidth()-225, 18, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_mismatch_fs:
            g.drawText(TRANS("Sample rate mismatch between DAW/IRs/HRIRs"),
                       getBounds().getWidth()-225, 18, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NinputCH:
            g.drawText(TRANS("Insufficient number of input channels (") + String(hVst->getTotalNumInputChannels()) +
                       TRANS("/") + String(interface_getNmicsArray(hInt)) + TRANS(")"),
                       getBounds().getWidth()-225, 18, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NoutputCH:
            g.drawText(TRANS("Insufficient number of output channels (") + String(hVst->getTotalNumOutputChannels()) +
                       TRANS("/") + String(2) + TRANS(")"),
                       getBounds().getWidth()-225, 18, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_osc_connection_fail:
            g.drawText(TRANS("Failed to connect to the selected OSC port"),
                       getBounds().getWidth()-225, 18, 530, 11,
                       Justification::centredLeft, true);
            break;
    }

    //[/UserPaint]
}

void PluginEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..

	repaint(0,0, 842, 448);
    //[/UserResized]
}

void PluginEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == s_diff2dir.get())
    {
        //[UserSliderCode_s_diff2dir] -- add your slider handling code here..
        interface_setStreamBalanceAllBands(hInt, (float)s_diff2dir->getValue());
        streamBalance2dSlider->setRefreshValuesFLAG(true);
        //[/UserSliderCode_s_diff2dir]
    }
    else if (sliderThatWasMoved == SL_analysis_avg.get())
    {
        //[UserSliderCode_SL_analysis_avg] -- add your slider handling code here..
        interface_setAnalysisAveraging(hInt, (float)SL_analysis_avg->getValue());
        //[/UserSliderCode_SL_analysis_avg]
    }
    else if (sliderThatWasMoved == SL_synthesis_avg.get())
    {
        //[UserSliderCode_SL_synthesis_avg] -- add your slider handling code here..
        interface_setSynthesisAveraging(hInt, (float)SL_synthesis_avg->getValue());
        //[/UserSliderCode_SL_synthesis_avg]
    }
    else if (sliderThatWasMoved == s_yaw.get())
    {
        //[UserSliderCode_s_yaw] -- add your slider handling code here..
        interface_setYaw(hInt, (float)s_yaw->getValue());
        //[/UserSliderCode_s_yaw]
    }
    else if (sliderThatWasMoved == s_pitch.get())
    {
        //[UserSliderCode_s_pitch] -- add your slider handling code here..
        interface_setPitch(hInt, (float)s_pitch->getValue());
        //[/UserSliderCode_s_pitch]
    }
    else if (sliderThatWasMoved == s_roll.get())
    {
        //[UserSliderCode_s_roll] -- add your slider handling code here..
        interface_setRoll(hInt, (float)s_roll->getValue());
        //[/UserSliderCode_s_roll]
    }
    else if (sliderThatWasMoved == s_x.get())
    {
        //[UserSliderCode_s_x] -- add your slider handling code here..
        interface_setX(hInt, (float)s_x->getValue());
        //[/UserSliderCode_s_x]
    }
    else if (sliderThatWasMoved == s_y.get())
    {
        //[UserSliderCode_s_y] -- add your slider handling code here..
        interface_setY(hInt, (float)s_y->getValue());
        //[/UserSliderCode_s_y]
    }
    else if (sliderThatWasMoved == s_z.get())
    {
        //[UserSliderCode_s_z] -- add your slider handling code here..
        interface_setZ(hInt, (float)s_z->getValue());
        //[/UserSliderCode_s_z]
    }
    else if (sliderThatWasMoved == SL_source_distance.get())
    {
        //[UserSliderCode_SL_source_distance] -- add your slider handling code here..
        interface_setSourceDistance(hInt, (float)SL_source_distance->getValue());
        //[/UserSliderCode_SL_source_distance]
    }
    else if (sliderThatWasMoved == SL_BSM2Ambi_cutoff.get())
    {
        //[UserSliderCode_SL_BSM2Ambi_cutoff] -- add your slider handling code here..
        interface_setMaximumBSMFreq(hInt, (float)SL_BSM2Ambi_cutoff->getValue());
        //[/UserSliderCode_SL_BSM2Ambi_cutoff]
    }
    else if (sliderThatWasMoved == SL_maxAnalysisFreq.get())
    {
        //[UserSliderCode_SL_maxAnalysisFreq] -- add your slider handling code here..
        interface_setMaximumAnalysisFreq(hInt, (float)SL_maxAnalysisFreq->getValue());
        //[/UserSliderCode_SL_maxAnalysisFreq]
    }
    else if (sliderThatWasMoved == SL_linear2parametric.get())
    {
        //[UserSliderCode_SL_linear2parametric] -- add your slider handling code here..
        interface_setLinear2ParametricBalance(hInt, (float)SL_linear2parametric->getValue());
        //[/UserSliderCode_SL_linear2parametric]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void PluginEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == TBuseDefaultHRIRs.get())
    {
        //[UserButtonCode_TBuseDefaultHRIRs] -- add your button handler code here..
        interface_setUseDefaultHRIRsflag(hInt, (int)TBuseDefaultHRIRs->getToggleState());
        //[/UserButtonCode_TBuseDefaultHRIRs]
    }
    else if (buttonThatWasClicked == t_flipYaw.get())
    {
        //[UserButtonCode_t_flipYaw] -- add your button handler code here..
        interface_setFlipYaw(hInt, (int)t_flipYaw->getToggleState());
        //[/UserButtonCode_t_flipYaw]
    }
    else if (buttonThatWasClicked == t_flipPitch.get())
    {
        //[UserButtonCode_t_flipPitch] -- add your button handler code here..
        interface_setFlipPitch(hInt, (int)t_flipPitch->getToggleState());
        //[/UserButtonCode_t_flipPitch]
    }
    else if (buttonThatWasClicked == t_flipRoll.get())
    {
        //[UserButtonCode_t_flipRoll] -- add your button handler code here..
        interface_setFlipRoll(hInt, (int)t_flipRoll->getToggleState());
        //[/UserButtonCode_t_flipRoll]
    }
    else if (buttonThatWasClicked == t_flipX.get())
    {
        //[UserButtonCode_t_flipX] -- add your button handler code here..
        interface_setFlipX(hInt, (int)t_flipX->getToggleState());
        //[/UserButtonCode_t_flipX]
    }
    else if (buttonThatWasClicked == t_flipY.get())
    {
        //[UserButtonCode_t_flipY] -- add your button handler code here..
        interface_setFlipY(hInt, (int)t_flipY->getToggleState());
        //[/UserButtonCode_t_flipY]
    }
    else if (buttonThatWasClicked == t_flipZ.get())
    {
        //[UserButtonCode_t_flipZ] -- add your button handler code here..
        interface_setFlipZ(hInt, (int)t_flipZ->getToggleState());
        //[/UserButtonCode_t_flipZ]
    }
    else if (buttonThatWasClicked == TBenableSourceDirectivity.get())
    {
        //[UserButtonCode_TBenableSourceDirectivity] -- add your button handler code here..
        interface_setEnableSourceDirectivity(hInt, (int)TBenableSourceDirectivity->getToggleState());
        //[/UserButtonCode_TBenableSourceDirectivity]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void PluginEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == CBdofOption.get())
    {
        //[UserComboBoxCode_CBdofOption] -- add your combo box handling code here..
        interface_setDOFoption(hInt, (INTERFACE_DOF_OPTIONS)CBdofOption->getSelectedId());
        //[/UserComboBoxCode_CBdofOption]
    }
    else if (comboBoxThatHasChanged == CBListenerViewOption.get())
    {
        //[UserComboBoxCode_CBListenerViewOption] -- add your combo box handling code here..
        translationWindow->setViewPoint((LISTENER_WINDOW_VIEWPOINTS) CBListenerViewOption->getSelectedId());
        //[/UserComboBoxCode_CBListenerViewOption]
    }
    else if (comboBoxThatHasChanged == CBdistMapOption.get())
    {
        //[UserComboBoxCode_CBdistMapOption] -- add your combo box handling code here..
        interface_setDistanceMapMode(hInt, (INTERFACE_DISTANCE_MAPS)CBdistMapOption->getSelectedId());
        //[/UserComboBoxCode_CBdistMapOption]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void PluginEditor::timerCallback(int timerID)
{
    switch(timerID){
        case TIMER_PROCESSING_RELATED:
            /* Handled in PluginProcessor */
            break;

        case TIMER_GUI_RELATED:

            /* parameters whos values can change internally should be periodically refreshed */
            if(TBuseDefaultHRIRs->getToggleState() != interface_getUseDefaultHRIRsflag(hInt))
                TBuseDefaultHRIRs->setToggleState(interface_getUseDefaultHRIRsflag(hInt), dontSendNotification);
            if(s_yaw->getValue() != interface_getYaw(hInt))
                s_yaw->setValue(interface_getYaw(hInt), dontSendNotification);
            if(s_pitch->getValue() != interface_getPitch(hInt))
                s_pitch->setValue(interface_getPitch(hInt), dontSendNotification);
            if(s_roll->getValue() != interface_getRoll(hInt))
                s_roll->setValue(interface_getRoll(hInt), dontSendNotification);
            if(s_x->getValue() != interface_getX(hInt))
                s_x->setValue(interface_getX(hInt), dontSendNotification);
            if(s_y->getValue() != interface_getY(hInt))
                s_y->setValue(interface_getY(hInt), dontSendNotification);
            if(s_z->getValue() != interface_getZ(hInt))
                s_z->setValue(interface_getZ(hInt), dontSendNotification);
            s_diff2dir->setValue(interface_getStreamBalanceAllBands(hInt), dontSendNotification);
            label_IR_fs_array->setText(String(interface_getIRsamplerateArray(hInt)), dontSendNotification);
            label_DAW_fs_array->setText(String(interface_getDAWsamplerate(hInt)), dontSendNotification);
            label_N_nMics->setText(String(interface_getNmicsArray(hInt)), dontSendNotification);
            label_N_nDirs_array->setText(String(interface_getNDirsArray(hInt)), dontSendNotification);
            label_IR_length_array->setText(String(interface_getIRlengthArray(hInt)), dontSendNotification);
            label_HRIR_fs_bin->setText(String(interface_getIRsamplerateBin(hInt)), dontSendNotification);
            label_DAW_fs_bin->setText(String(interface_getDAWsamplerate(hInt)), dontSendNotification);
            label_N_nDirs_bin->setText(String(interface_getNDirsBin(hInt)), dontSendNotification);
            label_IR_length_bin->setText(String(interface_getIRlengthBin(hInt)), dontSendNotification);
            SL_analysis_avg->setValue(interface_getAnalysisAveraging(hInt), dontSendNotification);
            SL_synthesis_avg->setValue(interface_getSynthesisAveraging(hInt), dontSendNotification);
            SL_maxAnalysisFreq->setValue(interface_getMaximumAnalysisFreq(hInt), dontSendNotification);
            SL_BSM2Ambi_cutoff->setValue(interface_getMaximumBSMFreq(hInt), dontSendNotification);
            SL_linear2parametric->setValue(interface_getLinear2ParametricBalance(hInt), dontSendNotification);

            /* refresh */
            int nPoints;
            float* pX_vector;
            float* pY_values;
            if (streamBalance2dSlider->getRefreshValuesFLAG() && interface_getCoreStatus(hInt)==CORE_STATUS_INITIALISED){
                interface_setStreamBalanceFromLocal(hInt);
                interface_getStreamBalanceLocalPtrs(hInt, &pX_vector, &pY_values, &nPoints);
                streamBalance2dSlider->setDataHandles(pX_vector, pY_values, nPoints);
                streamBalance2dSlider->repaint();
                streamBalance2dSlider->setRefreshValuesFLAG(false);
            }
            interface_getGridDirectionsXYZLocalPtrs(hInt, &pX_vector, &nPoints);
            interface_getHistogramLocalPtrs(hInt, &pY_values, &nPoints);
            translationWindow->setDataHandles(pX_vector, pY_values, nPoints);
            translationWindow->refresh();

            /* Progress bar */
            if(interface_getCoreStatus(hInt)==CORE_STATUS_INITIALISING){
                addAndMakeVisible(progressbar);
                progress = (double)interface_getProgressBar0_1(hInt);
                char text[INTERFACE_PROGRESSBARTEXT_CHAR_LENGTH];
                interface_getProgressBarText(hInt, (char*)text);
                progressbar.setTextToDisplay(String(text));
            }
            else
                removeChildComponent(&progressbar);

            /* Some parameters shouldn't be editable during initialisation*/
            if(interface_getCoreStatus(hInt)==CORE_STATUS_INITIALISING){
                if(fileChooserMAIR.isEnabled())
                    fileChooserMAIR.setEnabled(false);
                if(fileChooserHRIR.isEnabled())
                    fileChooserHRIR.setEnabled(false);
            }
            else {
                if(!fileChooserMAIR.isEnabled())
                    fileChooserMAIR.setEnabled(true);
                if(!fileChooserHRIR.isEnabled())
                    fileChooserHRIR.setEnabled(true);
            }

            /* display warning message, if needed */
            if ((hVst->getCurrentBlockSize() % interface_getFrameSize()) != 0){
                currentWarning = k_warning_frameSize;
                repaint(0,0,getWidth(),32);
            }
            else if ( !((interface_getDAWsamplerate(hInt) == 44.1e3) || (interface_getDAWsamplerate(hInt) == 48e3)) ){
                currentWarning = k_warning_supported_fs;
                repaint(0,0,getWidth(),32);
            }
            else if ((interface_getDAWsamplerate(hInt) != interface_getIRsamplerateArray(hInt)) ||
                      interface_getDAWsamplerate(hInt) != interface_getIRsamplerateBin(hInt)){
                currentWarning = k_warning_mismatch_fs;
                repaint(0,0,getWidth(),32);
            }
            else if (hVst->getCurrentNumInputs() < interface_getNmicsArray(hInt)){
                currentWarning = k_warning_NinputCH;
                repaint(0,0,getWidth(),32);
            }
            else if (hVst->getCurrentNumOutputs() < 2){
                currentWarning = k_warning_NoutputCH;
                repaint(0,0,getWidth(),32);
            }
            else if(!hVst->getOscPortConnected()){
                currentWarning = k_warning_osc_connection_fail;
                repaint(0,0,getWidth(),32);
            }
            else if(currentWarning){
                currentWarning = k_warning_none;
                repaint(0,0,getWidth(),32);
            }

            /* check if OSC port has changed */
            if(hVst->getOscPortID() != te_oscport->getText().getIntValue())
                hVst->setOscPortID(te_oscport->getText().getIntValue());
            break;
    }
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PluginEditor" componentName=""
                 parentClasses="public AudioProcessorEditor, public MultiTimer, private FilenameComponentListener"
                 constructorParams="PluginProcessor* ownerFilter" variableInitialisers="AudioProcessorEditor(ownerFilter), progressbar(progress), fileChooserMAIR (&quot;File&quot;, File(), true, false, false,&#10;                       &quot;*.sofa;*.nc;&quot;, String(),&#10;                       &quot;Load SOFA File&quot;), fileChooserHRIR (&quot;File&quot;, File(), true, false, false,&#10;                       &quot;*.sofa;*.nc;&quot;, String(),&#10;                       &quot;Load SOFA File&quot;)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="842" initialHeight="448">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 201 842 247" fill="linear: 8 448, 8 360, 0=ff000000, 1=ff1c1c1c"
          hasStroke="0"/>
    <RECT pos="0 30 842 171" fill="linear: 8 32, 8 80, 0=ff000000, 1=ff1c1c1c"
          hasStroke="0"/>
    <ROUNDRECT pos="1 2 840 31" cornerSize="5.0" fill="linear: 0 32, 842 32, 0=ff000000, 1=ff1c1c1c"
               hasStroke="1" stroke="2, mitered, butt" strokeColour="solid: ffb9b9b9"/>
    <RECT pos="223 58 213 158" fill="solid: 10f4f4f4" hasStroke="1" stroke="0.8, mitered, butt"
          strokeColour="solid: 67a0a0a0"/>
    <RECT pos="12 58 212 158" fill="solid: 10f4f4f4" hasStroke="1" stroke="0.8, mitered, butt"
          strokeColour="solid: 67a0a0a0"/>
    <RECT pos="12 58 212 33" fill="solid: 8f4f4f4" hasStroke="1" stroke="0.8, mitered, butt"
          strokeColour="solid: 67a0a0a0"/>
    <TEXT pos="66 33 149 30" fill="solid: ffffffff" hasStroke="0" text="Load Array IRs"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <RECT pos="12 335 424 105" fill="solid: 10f4f4f4" hasStroke="1" stroke="0.8, mitered, butt"
          strokeColour="solid: 67a0a0a0"/>
    <TEXT pos="21 331 331 30" fill="solid: ffffffff" hasStroke="0" text="Direct to Residual Balance Per Frequency Band"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="19 415 35 30" fill="solid: ffffffff" hasStroke="0" text="100"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="174 415 35 30" fill="solid: ffffffff" hasStroke="0" text="1k"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="328 415 35 30" fill="solid: ffffffff" hasStroke="0" text="10k"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="210 415 91 30" fill="solid: ffffffff" hasStroke="0" text="Frequency (Hz)"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="371 415 35 30" fill="solid: ffffffff" hasStroke="0" text="20k"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="413 413 27 30" fill="solid: ffffffff" hasStroke="0" text="Res"
          fontname="Default font" fontsize="11.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="413 342 17 30" fill="solid: ffffffff" hasStroke="0" text="Dir"
          fontname="Default font" fontsize="11.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <RECT pos="0 0 842 2" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="0 0 2 448" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="1298 0 2 448" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="0 446 1234 2" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <TEXT pos="22 91 138 30" fill="solid: ffffffff" hasStroke="0" text="Number of Sensors:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 114 170 30" fill="solid: ffffffff" hasStroke="0" text="Number of Directions:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 137 162 30" fill="solid: ffffffff" hasStroke="0" text="IR length: "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 159 162 30" fill="solid: ffffffff" hasStroke="0" text="IR SampleRate:  "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 184 162 30" fill="solid: ffffffff" hasStroke="0" text="DAW SampleRate:  "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="12 1 120 32" fill="solid: ffffffff" hasStroke="0" text="6DoF |"
          fontname="Default font" fontsize="18.8" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="72 1 106 32" fill="solid: ffff3636" hasStroke="0" text="Auraliser"
          fontname="Default font" fontsize="18.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="290 33 149 30" fill="solid: ffffffff" hasStroke="0" text="Load HRIRs"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <RECT pos="223 58 213 58" fill="solid: 8f4f4f4" hasStroke="1" stroke="0.8, mitered, butt"
          strokeColour="solid: 67a0a0a0"/>
    <TEXT pos="232 88 173 30" fill="solid: ffffffff" hasStroke="0" text="Use Default HRIR set:"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="230 115 170 30" fill="solid: ffffffff" hasStroke="0" text="Number of Directions:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="230 137 162 30" fill="solid: ffffffff" hasStroke="0" text="HRIR length: "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="230 161 162 30" fill="solid: ffffffff" hasStroke="0" text="HRIR SampleRate:  "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="230 184 162 30" fill="solid: ffffffff" hasStroke="0" text="DAW SampleRate:  "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <RECT pos="12 215 424 121" fill="solid: 10f4f4f4" hasStroke="1" stroke="0.8, mitered, butt"
          strokeColour="solid: 67a0a0a0"/>
    <TEXT pos="236 214 162 30" fill="solid: ffffffff" hasStroke="0" text="Analysis Avg:  "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="236 238 162 30" fill="solid: ffffffff" hasStroke="0" text="Synthesis Avg:  "
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <RECT pos="1232 0 2 448" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="840 0 2 448" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <TEXT pos="654 346 49 30" fill="solid: ffffffff" hasStroke="0" text="\ypr[0]"
          fontname="Default font" fontsize="10.0" kerning="0.0" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="695 346 45 30" fill="solid: ffffffff" hasStroke="0" text="Pitch"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="750 346 54 30" fill="solid: ffffffff" hasStroke="0" text="Roll"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="751 417 62 30" fill="solid: ffffffff" hasStroke="0" text="+/-"
          fontname="Default font" fontsize="13.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="623 417 62 30" fill="solid: ffffffff" hasStroke="0" text="+/-"
          fontname="Default font" fontsize="13.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="687 417 62 30" fill="solid: ffffffff" hasStroke="0" text="+/-"
          fontname="Default font" fontsize="13.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="618 346 64 30" fill="solid: ffffffff" hasStroke="0" text="Yaw"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="727 346 40 30" fill="solid: ffffffff" hasStroke="0" text="\ypr[1]"
          fontname="Default font" fontsize="10.0" kerning="0.0" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="784 346 40 30" fill="solid: ffffffff" hasStroke="0" text="\ypr[2]"
          fontname="Default font" fontsize="10.0" kerning="0.0" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="458 346 49 30" fill="solid: ffffffff" hasStroke="0" text="\xyz[0]"
          fontname="Default font" fontsize="10.0" kerning="0.0" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="504 346 45 30" fill="solid: ffffffff" hasStroke="0" text="Y"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="559 346 54 30" fill="solid: ffffffff" hasStroke="0" text="Z"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="427 346 64 30" fill="solid: ffffffff" hasStroke="0" text="X"
          fontname="Default font" fontsize="12.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="531 346 40 30" fill="solid: ffffffff" hasStroke="0" text="\xyz[1]"
          fontname="Default font" fontsize="10.0" kerning="0.0" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="588 346 40 30" fill="solid: ffffffff" hasStroke="0" text="\xyz[2]"
          fontname="Default font" fontsize="10.0" kerning="0.0" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="560 417 62 30" fill="solid: ffffffff" hasStroke="0" text="+/-"
          fontname="Default font" fontsize="13.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="432 417 62 30" fill="solid: ffffffff" hasStroke="0" text="+/-"
          fontname="Default font" fontsize="13.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="496 417 62 30" fill="solid: ffffffff" hasStroke="0" text="+/-"
          fontname="Default font" fontsize="13.0" kerning="0.0" bold="1"
          italic="0" justification="36" typefaceStyle="Bold"/>
    <TEXT pos="22 214 162 30" fill="solid: ffffffff" hasStroke="0" text="Rendering Mode:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 238 162 30" fill="solid: ffffffff" hasStroke="0" text="OSC Port:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="236 262 162 30" fill="solid: ffffffff" hasStroke="0" text="Max BSM (Hz):"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="236 286 162 30" fill="solid: ffffffff" hasStroke="0" text="Max Ana (Hz):"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 283 162 30" fill="solid: ffffffff" hasStroke="0" text="Source dist (m):"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="236 307 162 30" fill="solid: ffffffff" hasStroke="0" text="lin2par balance:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 307 162 30" fill="solid: ffffffff" hasStroke="0" text="Enable Source Directivity:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="22 260 162 30" fill="solid: ffffffff" hasStroke="0" text="Dist Map Option:"
          fontname="Default font" fontsize="13.5" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
  </BACKGROUND>
  <SLIDER name="new slider" id="b82f31194d53ffb4" memberName="s_diff2dir"
          virtualName="" explicitFocusOrder="0" pos="384 354 40 74" bkgcol="ff5c5d5e"
          trackcol="ff315b6d" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="0.0" max="2.0" int="0.01" style="LinearVertical" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <LABEL name="new label" id="f8b5274e0c8768f4" memberName="label_IR_fs_array"
         virtualName="" explicitFocusOrder="0" pos="164 166 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="c59fb2aab2496c4e" memberName="label_DAW_fs_array"
         virtualName="" explicitFocusOrder="0" pos="164 189 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="57ac3ad75ce7e4c2" memberName="label_N_nMics"
         virtualName="" explicitFocusOrder="0" pos="164 97 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="4358fabfd06714ba" memberName="label_N_nDirs_array"
         virtualName="" explicitFocusOrder="0" pos="164 120 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="6f16ddfe2924d418" memberName="label_IR_length_array"
         virtualName="" explicitFocusOrder="0" pos="164 143 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="f7f951a1b21e1a11" memberName="TBuseDefaultHRIRs"
                virtualName="" explicitFocusOrder="0" pos="400 91 32 24" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="new label" id="38eb15d2da8894dc" memberName="label_HRIR_fs_bin"
         virtualName="" explicitFocusOrder="0" pos="377 166 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="f48b8809471400fe" memberName="label_DAW_fs_bin"
         virtualName="" explicitFocusOrder="0" pos="377 189 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="b05e699924050def" memberName="label_N_nDirs_bin"
         virtualName="" explicitFocusOrder="0" pos="377 121 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="9590662d8dbe4192" memberName="label_IR_length_bin"
         virtualName="" explicitFocusOrder="0" pos="377 143 51 20" outlineCol="68a3a2a2"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="181508282c18d28c" memberName="SL_analysis_avg"
          virtualName="" explicitFocusOrder="0" pos="328 218 98 22" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" min="0.0" max="1.0" int="0.01" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="45"
          textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="9f8a89e1adcd1292" memberName="SL_synthesis_avg"
          virtualName="" explicitFocusOrder="0" pos="328 242 98 22" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" min="0.0" max="1.0" int="0.01" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="45"
          textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <TEXTEDITOR name="new text editor" id="1799da9e8cf495d6" memberName="te_oscport"
              virtualName="" explicitFocusOrder="0" pos="133 244 88 18" textcol="ffffffff"
              bkgcol="ffffff" outlinecol="68a3a2a2" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="0" popupmenu="1"/>
  <SLIDER name="new slider" id="ace036a85eec9703" memberName="s_yaw" virtualName=""
          explicitFocusOrder="0" pos="635 365 61 56" rotarysliderfill="ff315b6d"
          rotaryslideroutline="ff5c5d5e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-180.0" max="180.0" int="0.01" style="RotaryHorizontalVerticalDrag"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="58"
          textBoxHeight="15" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="9af7dd86cd139d85" memberName="s_pitch"
          virtualName="" explicitFocusOrder="0" pos="698 365 61 56" rotarysliderfill="ff315b6e"
          rotaryslideroutline="ff5c5d5e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-180.0" max="180.0" int="0.01" style="RotaryHorizontalVerticalDrag"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="58"
          textBoxHeight="15" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="b5d39bb257b3289a" memberName="s_roll" virtualName=""
          explicitFocusOrder="0" pos="761 365 61 56" rotarysliderfill="ff315b6d"
          rotaryslideroutline="ff5c5d5e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-180.0" max="180.0" int="0.01" style="RotaryHorizontalVerticalDrag"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="58"
          textBoxHeight="15" skewFactor="1.0" needsCallback="1"/>
  <TOGGLEBUTTON name="new toggle button" id="ac47b63592b1d4cf" memberName="t_flipYaw"
                virtualName="" explicitFocusOrder="0" pos="668 420 24 23" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="c58241ee52766d62" memberName="t_flipPitch"
                virtualName="" explicitFocusOrder="0" pos="731 420 24 23" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="717e9536768dfd8c" memberName="t_flipRoll"
                virtualName="" explicitFocusOrder="0" pos="794 420 24 23" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <SLIDER name="new slider" id="84efe9c5bf7c70af" memberName="s_x" virtualName=""
          explicitFocusOrder="0" pos="446 365 61 56" rotarysliderfill="ff315b6d"
          rotaryslideroutline="ff5c5d5e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-180.0" max="180.0" int="0.01" style="RotaryHorizontalVerticalDrag"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="58"
          textBoxHeight="15" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="79112e670c3e2858" memberName="s_y" virtualName=""
          explicitFocusOrder="0" pos="509 365 61 56" rotarysliderfill="ff315b6e"
          rotaryslideroutline="ff5c5d5e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-180.0" max="180.0" int="0.01" style="RotaryHorizontalVerticalDrag"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="58"
          textBoxHeight="15" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="2d4a3bc373dda7b4" memberName="s_z" virtualName=""
          explicitFocusOrder="0" pos="572 365 61 56" rotarysliderfill="ff315b6d"
          rotaryslideroutline="ff5c5d5e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-180.0" max="180.0" int="0.01" style="RotaryHorizontalVerticalDrag"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="58"
          textBoxHeight="15" skewFactor="1.0" needsCallback="1"/>
  <TOGGLEBUTTON name="new toggle button" id="bb85c01d568b2860" memberName="t_flipX"
                virtualName="" explicitFocusOrder="0" pos="477 420 24 23" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="8d4c14b267c5fef2" memberName="t_flipY"
                virtualName="" explicitFocusOrder="0" pos="540 420 24 23" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="bfec0d749131c59c" memberName="t_flipZ"
                virtualName="" explicitFocusOrder="0" pos="603 420 24 23" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <COMBOBOX name="new combo box" id="a7219c15eb41112a" memberName="CBdofOption"
            virtualName="" explicitFocusOrder="0" pos="128 221 96 18" editable="0"
            layout="33" items="" textWhenNonSelected="Default" textWhenNoItems="(no choices)"/>
  <SLIDER name="new slider" id="9ffde88e344388de" memberName="SL_source_distance"
          virtualName="" explicitFocusOrder="0" pos="128 288 98 22" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" min="0.5" max="3.0" int="0.01" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="45"
          textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <COMBOBOX name="new combo box" id="3c5a9dbc0789e0c" memberName="CBListenerViewOption"
            virtualName="" explicitFocusOrder="0" pos="740 38 96 14" editable="0"
            layout="33" items="" textWhenNonSelected="Default" textWhenNoItems="(no choices)"/>
  <SLIDER name="new slider" id="37b07fafe11f98a7" memberName="SL_BSM2Ambi_cutoff"
          virtualName="" explicitFocusOrder="0" pos="328 265 98 22" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" min="0.0" max="16000.0" int="1.0" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="45"
          textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="e6293f30f188555" memberName="SL_maxAnalysisFreq"
          virtualName="" explicitFocusOrder="0" pos="328 289 98 22" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" min="0.0" max="16000.0" int="1.0" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="45"
          textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <SLIDER name="new slider" id="4963c0fc0e49ac34" memberName="SL_linear2parametric"
          virtualName="" explicitFocusOrder="0" pos="328 312 98 22" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" min="0.0" max="1.0" int="0.01" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="45"
          textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <TOGGLEBUTTON name="new toggle button" id="53fa5ac6dd4c7569" memberName="TBenableSourceDirectivity"
                virtualName="" explicitFocusOrder="0" pos="194 310 32 24" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <COMBOBOX name="new combo box" id="780d93a388c911b3" memberName="CBdistMapOption"
            virtualName="" explicitFocusOrder="0" pos="128 266 96 18" editable="0"
            layout="33" items="" textWhenNonSelected="Default" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

