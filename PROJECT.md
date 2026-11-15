# RoughStart

Audio Unit-plugin (AUv2) voor Logic Pro 12 die het gainstagen van alle sporen automatiseert voordat het mixen begint. Op elk spoor staat een instance; via één "Analyze All" meten alle instances hun signaal en brengen het naar een doelniveau (Auto). Daarna balanceert de gebruiker op het gehoor met Trim, terwijl de faders op 0 blijven.

Versie: zie `VERSION`. Besluiten en mijlpalen: zie `ROADMAP.md`.

## Codes
- Type `aufx`, subtype `RgSt`, fabrikant `TvdK`
- Bundle ID `com.tvdk.roughstart`

## Stack
- C++20, JUCE 9 (git-submodule in `JUCE/`), CMake + Ninja
- macOS 12+, Universal binary (arm64 + x86_64)
- Bouwen kan met alleen de Command Line Tools; volledige Xcode is optioneel (debuggen)

## Bouwen
```bash
git submodule update --init --depth 1
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
De `.component` wordt na het bouwen automatisch gekopieerd naar `~/Library/Audio/Plug-Ins/Components/`.

## Testpakket voor anderen
```bash
./scripts/package.sh
```
Maakt `dist/RoughStart-<versie>.zip` met de plugin en `docs/INSTALL.md` en `docs/MANUAL.md` als PDF (vereist Google Chrome en Python-module `markdown`). Niet ondertekend: testers halen de quarantainemarkering eraf (staat in INSTALL.md).

## Automatisch bouwen (GitHub Actions)
`.github/workflows/build.yml`: bij elke push naar main en elke pull request bouwen, unit tests en auval op macOS. Een versielabel (`git tag v1.0.0 && git push --tags`) maakt het releasepakket en zet het in een GitHub Release; via Actions > build > Run workflow kun je het pakket ook zonder release laten bouwen.

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
- `Source/PluginEditor.*`: `RoughStartView` (interface op 100%) en `RoughStartEditor` (plug-invenster, schaalt de view); UI in het Engels, weergaven Track en Overview, presetmenu, Analyze All.
- `Source/OverviewComponent.*`: sorteerbare tabel met alle instances.
- `Source/PresetManager.*`: presets als `.rspreset` (XML) in `~/Library/Application Support/RoughStart/Presets`, factory presets en de standaard voor nieuwe instances (`settings.xml`); zonder GUI, getest in `Tests/`.
- `Source/UiStyle.h`: gedeelde kleuren, dB-opmaak en het logo.
- `Source/RoughStartLookAndFeel.h`: eigen donkere stijl voor alle bediening.
- `Source/MeterView.h`: de meter in de Track-weergave.
- `Tools/Snapshot.cpp`: rendert de editor naar PNG (`--target RoughStartSnapshot`).
- `Source/InlinePrompt.h`: invoer/bevestiging als laag in de editor (losse vensters flikkeren in Logic).
- `Source/HelpOverlay.h`: toont de handleiding in de plugin.
- `Resources/fonts/`: lettertype Inter (SIL OFL 1.1, licentie erbij), ingebakken via BinaryData.
- `docs/MANUAL.md`: handleiding (Engels), via `juce_add_binary_data` in de plugin ingebakken.

## Afspraken
- `VERSION` en `CHANGELOG.md` bij elke wijziging bijwerken.
- Geen certificaten, wachtwoorden of keys in de repo.
