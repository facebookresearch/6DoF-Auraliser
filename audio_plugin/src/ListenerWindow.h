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
#include "JuceHeader.h"
#include "PluginProcessor.h"

typedef enum _LISTENER_WINDOW_VIEWPOINTS{
    LISTENER_WINDOW_TOPDOWN_VIEW = 1,
    LISTENER_WINDOW_FORWARDS_VIEW
}LISTENER_WINDOW_VIEWPOINTS;

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ListenerWindow  : public AudioProcessorEditor
{
public:
    //==============================================================================
    ListenerWindow (PluginProcessor* ownerFilter);
    ~ListenerWindow() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void refresh(){
        /* Redraw visual depiction of what is going on... */
        repaint();
    }
    void setViewPoint(LISTENER_WINDOW_VIEWPOINTS newViewPoint){ viewpoint = newViewPoint; }
    LISTENER_WINDOW_VIEWPOINTS getViewPoint(){ return viewpoint; }

    void setDataHandles(float* _grid_dirs_xyz, float* _histogram, int _nDirs){
        grid_dirs_xyz = _grid_dirs_xyz;
        histogram = _histogram;
        nDirs = _nDirs;
    }
    
    
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    PluginProcessor* hVst;
    void* hInt;
    bool iconIsClicked;
    LISTENER_WINDOW_VIEWPOINTS viewpoint;

    /* tooltips */
    SharedResourcePointer<TooltipWindow> tipWindow;
    
    int nDirs;
    float* grid_dirs_xyz;
    float* histogram;
    
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListenerWindow)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

