cmake_minimum_required(VERSION 3.22)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" ROUGHSTART_VERSION)
string(STRIP "${ROUGHSTART_VERSION}" ROUGHSTART_VERSION)

# Alleen Apple Silicon voorlopig (Universal binary is een open vraag)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "")

project(RoughStart VERSION ${ROUGHSTART_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(RoughStart
    COMPANY_NAME "TvdK"
    BUNDLE_ID "com.tvdk.roughstart"
    PLUGIN_MANUFACTURER_CODE TvdK
    PLUGIN_CODE RgSt
    FORMATS AU
    PRODUCT_NAME "RoughStart"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(RoughStart PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/InstanceRegistry.cpp)

target_compile_definitions(RoughStart PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(RoughStart
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
