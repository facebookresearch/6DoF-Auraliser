
cmake_minimum_required(VERSION 3.15)

project(proposed VERSION 1.0.0)
message(STATUS "  ${PROJECT_NAME}")

# juce helper function to create a library, which is configured as a JUCE plugin
juce_add_plugin(${PROJECT_NAME}
    VERSION                     "${PROJECT_VERSION}"
    COMPANY_NAME                "Meta Reality Labs"                         
    IS_SYNTH                    FALSE                               
    NEEDS_MIDI_INPUT            FALSE                      
    NEEDS_MIDI_OUTPUT           FALSE                  
    IS_MIDI_EFFECT              FALSE                       
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE           
    COPY_PLUGIN_AFTER_BUILD     TRUE    
    PLUGIN_MANUFACTURER_CODE    META                
    PLUGIN_CODE                 PROP                     
    FORMATS                     VST     # ( VST AAX Unity VST3 AU AUv3 Standalone )
    PRODUCT_NAME                "${PROJECT_NAME}"
)   

# Generates "JuceHeader.h", and adds its path to the include paths of the target
juce_generate_juce_header(${PROJECT_NAME})  

# Add plugin source files
target_sources(${PROJECT_NAME} 
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ListenerWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ListenerWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/log2dSlider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/log2dSlider.cpp
)

# Add any extra JUCE-specific pre-processor definitions
target_compile_definitions(${PROJECT_NAME}
PUBLIC 
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
 
# Link with shades, and the required juce modules
target_link_libraries(${PROJECT_NAME} 
PRIVATE 
    interface
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
    juce::juce_osc
    juce::juce_opengl
)

