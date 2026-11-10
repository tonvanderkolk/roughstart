# RoughStart

Audio Unit-plugin (AUv2) voor Logic Pro 12 die het gainstagen van alle sporen automatiseert voordat het mixen begint. Op elk spoor staat een instance; via één "Analyze All" meten alle instances hun signaal en brengen het naar een doelniveau (Auto). Daarna balanceert de gebruiker op het gehoor met Trim, terwijl de faders op 0 blijven.

Versie: zie `VERSION`. Achtergrond, besluiten en mijlpalen: zie `HANDOVER.md` en `ROADMAP.md`.

## Codes
- Type `aufx`, subtype `RgSt`, fabrikant `TvdK`
- Bundle ID `com.tvdk.roughstart`

## Stack
- C++20, JUCE 9 (git-submodule in `JUCE/`), CMake + Ninja
- macOS 12+, arm64
- Bouwen kan met alleen de Command Line Tools; volledige Xcode is optioneel (debuggen)

## Bouwen
```bash
git submodule update --init --depth 1
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
De `.component` wordt na het bouwen automatisch gekopieerd naar `~/Library/Audio/Plug-Ins/Components/`.

## Testen
```bash
./build/RoughStartTests_artefacts/Release/RoughStartTests
```

## Valideren
```bash
auval -v aufx RgSt TvdK
```

## Structuur
- `Source/InstanceRegistry.*`: gedeelde lijst van alle instances in het hostproces (via `juce::SharedResourcePointer`), plus Analyze All / Stop All en de batchstatus. Werkt omdat Logic AUv2 in-process laadt.
- `Source/LevelMeter.h`: meting (piek/RMS, absolute + relatieve stiltedrempel), Auto-berekening met plafond en dB-rekenwerk, zonder JUCE; getest in `Tests/`.
- `Source/PluginProcessor.*`: parameters, analyse, Auto + Trim, state, spoornaam, ping-teller.
- `Source/PluginEditor.*`: UI in het Engels (Mode, Target, Peak ceiling, Analyze, Auto, Trim, instances, Analyze All).

## Afspraken
- `VERSION` en `CHANGELOG.md` bij elke wijziging bijwerken.
- Geen certificaten, wachtwoorden of keys in de repo.
