cmake_minimum_required(VERSION 3.22)
project(Acid303 VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Acid303
    COMPANY_NAME "Open Circuit"
    BUNDLE_ID com.opencircuit.acid303
    PLUGIN_MANUFACTURER_CODE OpCi
    PLUGIN_CODE Ac03
    FORMATS VST3 Standalone
    PRODUCT_NAME "Acid 303"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
)
target_sources(Acid303 PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
juce_generate_juce_header(Acid303)
target_compile_features(Acid303 PRIVATE cxx_std_17)
target_compile_definitions(Acid303 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(Acid303 PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
