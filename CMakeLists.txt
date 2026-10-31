cmake_minimum_required(VERSION 3.22)
# Set before project() so Rosetta-hosted CMake does not silently produce Intel-only plugins.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS target architectures")
endif()
project(Acid303 VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Acid303
    COMPANY_NAME "Seth Duncan"
    BUNDLE_ID com.sethduncan.sd303
    PLUGIN_MANUFACTURER_CODE SeDu
    PLUGIN_CODE SD03
    FORMATS VST3 Standalone
    PRODUCT_NAME "SD-303"
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

option(SD303_BUILD_TESTS "Build offline processor regression tests" OFF)
if(SD303_BUILD_TESTS)
    enable_testing()
    juce_add_console_app(SD303Tests PRODUCT_NAME "SD303Tests")
    target_sources(SD303Tests PRIVATE Tests/ProcessorTests.cpp)
    target_include_directories(SD303Tests PRIVATE Source "${CMAKE_CURRENT_BINARY_DIR}/Acid303_artefacts/JuceLibraryCode")
    target_compile_features(SD303Tests PRIVATE cxx_std_17)
    target_link_libraries(SD303Tests PRIVATE Acid303 juce::juce_audio_utils juce::juce_dsp)
    add_test(NAME processor COMMAND SD303Tests)
endif()
