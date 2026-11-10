# Changelog

Alle noemenswaardige wijzigingen in RoughStart. Formaat volgt [Keep a Changelog](https://keepachangelog.com/nl/1.1.0/), versies volgen [SemVer](https://semver.org/lang/nl/).

## [0.3.0] - 2026-11-10

### Toegevoegd
- **Analyze All**: knop onderin elke instance. Zet alle RoughStart-instances in het project tegelijk in analysestand (onder één batchnummer). Ze meten tijdens het afspelen en zetten bij stop elk hun eigen meting vast.
- **Stop All**: dezelfde knop stopt een lopende Analyze All voor alle instances.
- Status van de batch in elke instance: wachten op afspelen, aantal sporen dat meet, en na afloop hoeveel sporen zijn opgeslagen en hoeveel geen signaal hadden (Auto ongewijzigd).

### Verwijderd
- Knop "Ping all" en de indicator uit de proof of concept (v0.1).

## [0.2.2] - 2026-11-10

### Gewijzigd
- Analyse volgt het transport: er wordt alleen gemeten zolang Logic afspeelt. Klik je op Analyze terwijl Logic stilstaat, dan wacht de plugin tot je afspeelt ("Waiting for playback...", knop "Cancel").
- Stopt Logic na het afspelen, dan stopt de analyse vanzelf en wordt de meting vastgezet. Handmatig Stop blijft werken.
- Vangnet: komen er na het afspelen 1,5 seconde geen audioblokken meer binnen, dan geldt dat ook als gestopt.
- Status (wachten, meten, opgeslagen, geen signaal) komt uit de processor, dus klopt ook als het venster dicht was.

## [0.2.1] - 2026-11-10

### Toegevoegd
- Piekplafond (Peak Ceiling, standaard -6 dBFS, instelbaar -24 tot 0): Auto tilt de gemeten piek nooit boven het plafond. In de UI staat "(limited by ceiling)" als het plafond ingrijpt.
- Relatieve stiltedrempel voor RMS: vensters van 50 ms die meer dan 20 dB onder het gemiddelde liggen tellen niet mee (zoals de relatieve gate in EBU R128). Uitklinkende staarten tussen drumklappen trekken de RMS niet meer omlaag.

### Gewijzigd
- UI en parameternamen in het Engels (Mode, Peak Target, RMS Target, Trim, Peak Ceiling). Parameter-ID's zijn gelijk gebleven, dus bestaande projecten laden gewoon.

### Opgelost
- RMS-modus kon op dynamisch materiaal (drums) het kanaal laten clippen.

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
