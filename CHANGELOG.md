# Changelog

Alle noemenswaardige wijzigingen in RoughStart. Formaat volgt [Keep a Changelog](https://keepachangelog.com/nl/1.1.0/), versies volgen [SemVer](https://semver.org/lang/nl/).

## [0.2.0] - 2026-11-10

### Toegevoegd
- Meting per instance: piek en RMS tegelijk, over vensters van 50 ms; vensters onder -60 dBFS (stilte) tellen niet mee. Gemeten wordt het ingangssignaal, vóór de gain.
- Knop Analyze/Stop: Analyze & Hold. Bij Stop wordt de meting vastgezet; zonder signaal blijft de vorige meting (en Auto) staan.
- Meetmethode kiesbaar (Piek of RMS) met eigen doel: piek -12 dBFS, RMS -18 dBFS (standaard).
- Auto-gain = doel min meting, begrensd op ±24 dB. Doel of methode wijzigen rekent Auto direct opnieuw uit zonder nieuwe analyse.
- Trim (±24 dB, automatiseerbaar, dubbelklik zet op 0).
- Gain (Auto + Trim) vloeiend toegepast met een ramp van 50 ms, nul latency.
- Instellingen en vastgezette meting worden opgeslagen in de plugin-state (Logic-project).
- Unit tests voor de meting (`RoughStartTests`).

## [0.1.0] - 2026-11-10

### Toegevoegd
- Projectopzet: JUCE 9 (submodule), CMake, AUv2-effect `aufx RgSt TvdK`, arm64, macOS 12+.
- `InstanceRegistry` via `juce::SharedResourcePointer`: instances melden zich aan en af binnen het hostproces.
- Editor toont het aantal gevonden instances, de spoornamen (via `updateTrackProperties`) en een knop "Ping all" die in alle instances een indicator laat oplichten.
- Audio wordt ongewijzigd doorgegeven, nul latency, mono en stereo.
