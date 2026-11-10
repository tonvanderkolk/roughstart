# Changelog

Alle noemenswaardige wijzigingen in RoughStart. Formaat volgt [Keep a Changelog](https://keepachangelog.com/nl/1.1.0/), versies volgen [SemVer](https://semver.org/lang/nl/).

## [0.1.0] - 2026-11-10

### Toegevoegd
- Projectopzet: JUCE 9 (submodule), CMake, AUv2-effect `aufx RgSt TvdK`, arm64, macOS 12+.
- `InstanceRegistry` via `juce::SharedResourcePointer`: instances melden zich aan en af binnen het hostproces.
- Editor toont het aantal gevonden instances, de spoornamen (via `updateTrackProperties`) en een knop "Ping all" die in alle instances een indicator laat oplichten.
- Audio wordt ongewijzigd doorgegeven, nul latency, mono en stereo.
