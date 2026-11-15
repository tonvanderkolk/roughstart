cmake_minimum_required(VERSION 3.22)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" ROUGHSTART_VERSION)
string(STRIP "${ROUGHSTART_VERSION}" ROUGHSTART_VERSION)

# macOS: universal binary (Apple Silicon en Intel), macOS 12+
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "")
endif()

project(RoughStart VERSION ${ROUGHSTART_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

# Formaten: AU (alleen macOS) en VST3 (macOS en Windows).
# Na het bouwen kopiëren naar de pluginmap alleen op de Mac: op Windows
# vraagt C:\Program Files\Common Files\VST3 beheerdersrechten.
if(APPLE)
    set(ROUGHSTART_FORMATS AU VST3)
    set(ROUGHSTART_COPY TRUE)
else()
    set(ROUGHSTART_FORMATS VST3)
    set(ROUGHSTART_COPY FALSE)
endif()

juce_add_plugin(RoughStart
    COMPANY_NAME "TvdK"
    BUNDLE_ID "com.tvdk.roughstart"
    PLUGIN_MANUFACTURER_CODE TvdK
    PLUGIN_CODE RgSt
    FORMATS ${ROUGHSTART_FORMATS}
    VST3_CATEGORIES Fx Tools
    PRODUCT_NAME "RoughStart"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD ${ROUGHSTART_COPY})

juce_add_binary_data(RoughStartData SOURCES
    docs/MANUAL.md
    Resources/fonts/Inter-Regular.ttf
    Resources/fonts/Inter-Bold.ttf)

target_sources(RoughStart PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/InstanceRegistry.cpp
    Source/PresetManager.cpp
    Source/OverviewComponent.cpp)

target_compile_definitions(RoughStart PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(RoughStart
    PRIVATE
        RoughStartData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

#==============================================================================
# Tests (los van de plugin, alleen juce_core)
option(ROUGHSTART_BUILD_TESTS "Bouw de unit tests" ON)

if(ROUGHSTART_BUILD_TESTS)
    juce_add_console_app(RoughStartTests PRODUCT_NAME "RoughStartTests")
    target_sources(RoughStartTests PRIVATE Tests/RoughStartTests.cpp Source/PresetManager.cpp)
    target_compile_definitions(RoughStartTests PRIVATE JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
    target_link_libraries(RoughStartTests
        PRIVATE
            juce::juce_core
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
endif()

#==============================================================================
# Snapshot: rendert de editor naar PNG (ontwerp bekijken zonder Logic)
juce_add_console_app(RoughStartSnapshot PRODUCT_NAME "RoughStartSnapshot")
target_sources(RoughStartSnapshot PRIVATE
    Tools/Snapshot.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/InstanceRegistry.cpp
    Source/PresetManager.cpp
    Source/OverviewComponent.cpp)
target_compile_definitions(RoughStartSnapshot PRIVATE
    JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0
    JucePlugin_Name="RoughStart"
    JucePlugin_VersionString="${ROUGHSTART_VERSION}")
target_link_libraries(RoughStartSnapshot
    PRIVATE
        RoughStartData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags)
set_target_properties(RoughStartSnapshot PROPERTIES EXCLUDE_FROM_ALL TRUE)

#==============================================================================
# HostCheck: laadt de gebouwde plugin (VST3/AU) zoals een DAW en test de basis.
juce_add_console_app(RoughStartHostCheck PRODUCT_NAME "RoughStartHostCheck")
target_sources(RoughStartHostCheck PRIVATE Tools/HostCheck.cpp)
target_compile_definitions(RoughStartHostCheck PRIVATE
    JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0
    JUCE_PLUGINHOST_VST3=1 JUCE_PLUGINHOST_AU=1)
target_link_libraries(RoughStartHostCheck PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)
set_target_properties(RoughStartHostCheck PROPERTIES EXCLUDE_FROM_ALL TRUE)

#==============================================================================
# AnalyzeSim: speelt Analyze All na met sporen die de host tijdelijk overslaat.
juce_add_console_app(RoughStartAnalyzeSim PRODUCT_NAME "RoughStartAnalyzeSim")
target_sources(RoughStartAnalyzeSim PRIVATE
    Tools/AnalyzeSim.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/InstanceRegistry.cpp
    Source/PresetManager.cpp
    Source/OverviewComponent.cpp)
target_compile_definitions(RoughStartAnalyzeSim PRIVATE
    JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JucePlugin_Name="RoughStart"
    JucePlugin_VersionString="${ROUGHSTART_VERSION}")
target_link_libraries(RoughStartAnalyzeSim
    PRIVATE RoughStartData juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags)
set_target_properties(RoughStartAnalyzeSim PROPERTIES EXCLUDE_FROM_ALL TRUE)
