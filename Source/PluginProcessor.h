#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "InstanceRegistry.h"
#include "LevelMeter.h"

class RoughStartProcessor final : public juce::AudioProcessor,
                                  private juce::Timer
{
public:
    RoughStartProcessor();
    ~RoughStartProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void updateTrackProperties (const TrackProperties& properties) override;

    //==============================================================================
    enum class Mode { peak = 0, rms = 1 };

    struct Measurement
    {
        bool valid = false;
        float peakDb = roughstart::minusInfinityDb;
        float rmsDb  = roughstart::minusInfinityDb;
    };

    juce::AudioProcessorValueTreeState parameters;

    Mode getMode() const noexcept;
    float getTargetDb() const noexcept;          // doel van de actieve meetmethode
    float getTrimDb() const noexcept;

    /** Laatst vastgezette meting (Analyze & Hold). */
    Measurement getStoredMeasurement() const noexcept;

    /** Lopende meting tijdens analyse (valid = er is al niet-stil signaal gezien). */
    Measurement getLiveMeasurement() const noexcept;

    float getCeilingDb() const noexcept;

    /** Auto-gain uit de vastgezette meting, het doel en het plafond; 0 zonder meting. */
    roughstart::AutoGainResult getAutoGain() const noexcept;
    float getAutoGainDb() const noexcept { return getAutoGain().gainDb; }
    float getTotalGainDb() const noexcept { return getAutoGainDb() + getTrimDb(); }

    enum class AnalysisState  { idle, waitingForPlayback, measuring };
    enum class AnalysisResult { none, stored, noSignal };

    /**
        Zet de analyse klaar. Er wordt alleen gemeten zolang de host afspeelt;
        stopt de host (na afspelen), dan stopt de analyse vanzelf en wordt de
        meting vastgezet. Zonder transportinfo wordt altijd gemeten.
    */
    void startAnalysis (int batch = 0);

    /** Batchnummer van de laatste analyse: 0 = losse Analyze, >0 = Analyze All. */
    int getAnalysisBatch() const noexcept { return analysisBatch.load(); }
    /** Stopt de analyse. Geeft false als er geen signaal was; Auto blijft dan staan. */
    bool stopAnalysis();
    bool isAnalyzing() const noexcept { return analyzing.load(); }
    AnalysisState getAnalysisState() const noexcept;
    AnalysisResult getLastAnalysisResult() const noexcept { return (AnalysisResult) lastResult.load(); }

    //==============================================================================
    InstanceRegistry& getRegistry() noexcept { return *registry; }
    int getInstanceNumber() const noexcept { return instanceNumber; }

    /** Spoornaam van de host, of "Instance N" als de host die niet doorgeeft. */
    juce::String getDisplayName() const;


private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::SharedResourcePointer<InstanceRegistry> registry;
    int instanceNumber = 0;

    mutable juce::CriticalSection nameLock;
    juce::String trackName;

    // Parameters (raw pointers van de APVTS, thread-safe te lezen)
    std::atomic<float>* modeParam = nullptr;
    std::atomic<float>* targetPeakParam = nullptr;
    std::atomic<float>* targetRmsParam = nullptr;
    std::atomic<float>* trimParam = nullptr;
    std::atomic<float>* ceilingParam = nullptr;

    // Vastgezette meting
    std::atomic<bool>  storedValid { false };
    std::atomic<float> storedPeakDb { roughstart::minusInfinityDb };
    std::atomic<float> storedRmsDb  { roughstart::minusInfinityDb };

    // Lopende analyse: audiothread schrijft, berichtthread leest
    roughstart::LevelMeter meter;
    std::atomic<bool>  analyzing { false };
    std::atomic<bool>  resetMeterPending { false };
    std::atomic<bool>  liveValid { false };
    std::atomic<float> livePeakDb { roughstart::minusInfinityDb };
    std::atomic<float> liveRmsDb  { roughstart::minusInfinityDb };

    // Transport: audiothread schrijft, timer (berichtthread) leest
    std::atomic<bool>   hostPlaying { false };
    std::atomic<bool>   sawPlayback { false };
    std::atomic<double> lastBlockMs { 0.0 };
    std::atomic<int>    lastResult { (int) AnalysisResult::none };
    std::atomic<int>    analysisBatch { 0 };

    void timerCallback() override;

    juce::SmoothedValue<float> smoothedGain { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoughStartProcessor)
};
