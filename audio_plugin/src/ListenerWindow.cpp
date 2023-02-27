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
//[/Headers]

#include "ListenerWindow.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
const float iconWidth = 8.0f;
const float iconRadius = iconWidth/2.0f;
const float circleWidth = 280.0f;
const float circleRadius = circleWidth / 2.0f;
//[/MiscUserDefs]

//==============================================================================
ListenerWindow::ListenerWindow (PluginProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]


    //[UserPreSize]
    //[/UserPreSize]

    setSize (391, 341);


    //[Constructor] You can add your own custom stuff here..

    /* handles */
    hVst = ownerFilter;
    hInt = hVst->getFXHandle();
    viewpoint = LISTENER_WINDOW_TOPDOWN_VIEW;
    
    grid_dirs_xyz = NULL;
    histogram = NULL;
    nDirs = 0;

    //[/Constructor]
}

ListenerWindow::~ListenerWindow()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    setLookAndFeel(nullptr);
    //[/Destructor]
}

//==============================================================================
void ListenerWindow::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..

    juce::Colour flareColour = juce::Colour (0x44f4f4f4), transparentColour = juce::Colour (0x00f4f4f4);
    juce::Colour redColour = juce::Colour (Colours::red);

    /****** DRAW TOP VIEW ******/
    /* Background and border */
    float view_x = 52.0f , view_y = 2.0f;
    float centre_x = view_x+circleRadius;
    float centre_y = view_y+circleRadius;
    g.setGradientFill (juce::ColourGradient (flareColour, view_x+circleRadius, view_y + circleRadius,
                                             transparentColour, view_x+circleRadius, view_y + circleWidth, true));
    g.fillEllipse (view_x, view_y, circleWidth, circleWidth);
    g.setGradientFill (juce::ColourGradient (transparentColour, view_x+circleRadius, view_y + circleRadius,
                                             transparentColour, view_x+circleRadius, view_y + circleWidth, true));
    g.fillEllipse (view_x, view_y, circleWidth, circleWidth);
    g.setColour (redColour);
    g.drawEllipse (view_x, view_y, circleWidth, circleWidth, 2.000f);

    /* Receiver/origin marker and grid lines (one per metre) */
    g.setColour(Colours::lightgrey);
    g.setOpacity(0.5f);
    g.fillEllipse(centre_x-iconRadius, centre_y-iconRadius, iconWidth, iconWidth);
    g.setOpacity(0.25f);
    g.drawLine(view_x+circleRadius, view_y, view_x+circleRadius, view_y+circleWidth,1.0f);
    g.drawLine(view_x, view_y+circleRadius, view_x+circleWidth, view_y+circleRadius,1.0f);
    for(int i=0; i<(int)INTERFACE_PERIMETER_DISTANCE_M; i++){
        float guideWidth = (float)i * circleWidth/INTERFACE_PERIMETER_DISTANCE_M;
        g.drawEllipse (view_x+circleRadius-guideWidth/2.0f, view_y+circleRadius-guideWidth/2.0f, guideWidth, guideWidth, 1.000f);
    }
    
    /* Plot histogram wrapped around the origin */
    float blob_x, blob_y, blob_radius;
    if(nDirs>0){
        g.setColour(Colours::lightgrey);
        for(int i=0; i<nDirs; i++){
            g.setOpacity(juce::jlimit(0.0f, 1.0f, histogram[i]));
            
            blob_radius = 1.5f*iconRadius * sqrtf(histogram[i]+0.0001f);
            
            switch(viewpoint){
                case LISTENER_WINDOW_TOPDOWN_VIEW:
                    blob_x = -circleRadius * grid_dirs_xyz[i*3+1]*(interface_getSourceDistance(hInt)/INTERFACE_PERIMETER_DISTANCE_M); // Y
                    blob_y = -circleRadius * grid_dirs_xyz[i*3]*(interface_getSourceDistance(hInt)/INTERFACE_PERIMETER_DISTANCE_M);   // X
                    break;
                case LISTENER_WINDOW_FORWARDS_VIEW:
                    blob_x = -circleRadius * grid_dirs_xyz[i*3+1]*(interface_getSourceDistance(hInt)/INTERFACE_PERIMETER_DISTANCE_M); // Y
                    blob_y = -circleRadius * grid_dirs_xyz[i*3+2]*(interface_getSourceDistance(hInt)/INTERFACE_PERIMETER_DISTANCE_M); // Z
                    break;
            }
            Rectangle<float> blob(centre_x-blob_radius+blob_x, centre_y-blob_radius+blob_y, blob_radius*2, blob_radius*2);
            g.fillEllipse(blob);
        }
    }

    /* Listener and their head orientation */
    float listener_x, listener_y, face_x, face_y;
    float* head_xyz = interface_getHeadOrientationPtr(hInt);
    switch(viewpoint){
        case LISTENER_WINDOW_TOPDOWN_VIEW:
            listener_x = -circleRadius * interface_getY(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            listener_y = -circleRadius * interface_getX(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            face_x = listener_x -circleRadius *  head_xyz[1]*0.3;
            face_y = listener_y -circleRadius *  head_xyz[0]*0.3;
            break;
        case LISTENER_WINDOW_FORWARDS_VIEW:
            listener_x = -circleRadius * interface_getY(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            listener_y = -circleRadius * interface_getZ(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            face_x = listener_x -circleRadius * head_xyz[1]*0.3;
            face_y = listener_y -circleRadius * head_xyz[2]*0.3;
            break;
    }
    g.setColour(Colours::lightgrey);
    Line<float> arrow(centre_x+listener_x, centre_y+listener_y, centre_x+face_x, centre_y+face_y);
    g.setOpacity(0.65f);
    g.drawArrow(arrow, 3.0f, 10.0f, 10.0f);
    g.setOpacity(0.85f);

    g.setColour(redColour);
    g.setOpacity(1.0f);
    Rectangle<float> listenerIcon(centre_x-iconRadius+listener_x, centre_y-iconRadius+listener_y, iconWidth, iconWidth);
    g.fillEllipse(listenerIcon);
    g.setColour(Colours::lightgrey);
    g.drawEllipse(listenerIcon.expanded(1.0f, 1.0f),1.0f);
     
    //[/UserPaint]
}

void ListenerWindow::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..


	repaint();
    //[/UserResized]
}

void ListenerWindow::mouseDown (const juce::MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...

    /* TOP VIEW */
    float view_x = 52.0f , view_y = 2.0f;
    float centre_x = view_x+circleRadius;
    float centre_y = view_y+circleRadius;
    float listener_x, listener_y;
    switch(viewpoint){
        case LISTENER_WINDOW_TOPDOWN_VIEW:
            listener_x = -circleRadius * interface_getY(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            listener_y = -circleRadius * interface_getX(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            break;
        case LISTENER_WINDOW_FORWARDS_VIEW:
            listener_x = -circleRadius * interface_getY(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            listener_y = -circleRadius * interface_getZ(hInt)/INTERFACE_PERIMETER_DISTANCE_M;
            break;
    }
    Rectangle<int> icon_int;
    icon_int.setBounds(centre_x-iconRadius+listener_x,
                       centre_y-iconRadius+listener_y,
                       iconWidth, iconWidth);
    if(icon_int.expanded(4, 4).contains(e.getMouseDownPosition())){
        iconIsClicked = true;
        return;
    }
    //[/UserCode_mouseDown]
}

void ListenerWindow::mouseDrag (const juce::MouseEvent& e)
{
    //[UserCode_mouseDrag] -- Add your code here...
    if(iconIsClicked){
        Point<float> point;
        float view_x, view_y, centre_x, centre_y;
        view_x = 52.0f; view_y = 2.0f;
        centre_x = view_x+circleRadius;
        centre_y = view_y+circleRadius;
        point.setXY((float)e.getPosition().getX()-2, (float)e.getPosition().getY()-2);
        switch(viewpoint){
            case LISTENER_WINDOW_TOPDOWN_VIEW:
                interface_setY(hInt, ((centre_x-point.getX()) * INTERFACE_PERIMETER_DISTANCE_M/circleRadius));
                interface_setX(hInt, ((centre_y-point.getY()) * INTERFACE_PERIMETER_DISTANCE_M/circleRadius));
                break;
            case LISTENER_WINDOW_FORWARDS_VIEW:
                interface_setY(hInt, ((centre_x-point.getX()) * INTERFACE_PERIMETER_DISTANCE_M/circleRadius));
                interface_setZ(hInt, ((centre_y-point.getY()) * INTERFACE_PERIMETER_DISTANCE_M/circleRadius));
                break;
        }
    }
    //[/UserCode_mouseDrag]
}

void ListenerWindow::mouseUp (const juce::MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    iconIsClicked = false;
    //[/UserCode_mouseUp]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ListenerWindow" componentName=""
                 parentClasses="public AudioProcessorEditor" constructorParams="PluginProcessor* ownerFilter"
                 variableInitialisers="AudioProcessorEditor(ownerFilter)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="391" initialHeight="341">
  <METHODS>
    <METHOD name="mouseDown (const juce::MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const juce::MouseEvent&amp; e)"/>
    <METHOD name="mouseUp (const juce::MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

