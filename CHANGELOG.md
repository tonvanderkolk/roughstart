# Changelog

Alle noemenswaardige wijzigingen in RoughStart. Formaat volgt [Keep a Changelog](https://keepachangelog.com/nl/1.1.0/), versies volgen [SemVer](https://semver.org/lang/nl/).

## [0.10.0] - 2026-11-15

### Toegevoegd
- **VST3** naast AU op de Mac (voor Cubase, Bitwig en andere VST3-hosts), en **Windows (64-bit, VST3)**. De Windows-versie wordt in GitHub Actions gebouwd en getest, maar is nog niet in een DAW uitgeprobeerd.
- `RoughStartHostCheck` (`Tools/HostCheck.cpp`): laadt de gebouwde VST3 of AU zoals een DAW en controleert laden, twee instances, mono/stereo, audio, Trim, state opslaan/terugzetten en (met `--editor`) de editor. Draait in GitHub Actions op Mac en Windows.
- GitHub Actions: Windows-job (bouwen, unit tests, hostcontrole) en een Windows-zip bij een release, met dezelfde PDF's en licenties.
- Installatiebriefje en handleiding: Mac met AU en VST3, Windows-installatie, presetmap op Windows.

### Gewijzigd
- Presets op Windows in `%APPDATA%\RoughStart\Presets`.
- Mac-pakket heet nu `RoughStart-<versie>-macos.zip` en bevat ook `RoughStart.vst3`.
- Tooltip van Analyze track noemt "your DAW" in plaats van Logic.

### Toegevoegd
- GitHub Actions (`.github/workflows/build.yml`), naar het voorbeeld van DAW 388: bij elke push bouwen, unit tests en auval op macOS; een versielabel maakt het releasepakket en zet het in een GitHub Release.

## [0.9.1] - 2026-11-15

### Toegevoegd
- Licentie: RoughStart is open source onder AGPL-3.0 (`LICENSE`). Vermeld in handleiding, installatiebriefje en testpakket (met de licenties van RoughStart en Inter).
- Engelse `README.md` voor de openbare repo: wat het doet, schermafbeeldingen, installeren, bouwen vanaf de broncode, licenties.

## [0.9.0] - 2026-11-15

### Toegevoegd
- **Venstergrootte**: knop met percentage rechtsboven (100, 125, 150 of 175%). De keuze geldt voor alle instances en wordt onthouden (`uiScale` in `settings.xml`).
- **Lettertype Inter** (Regular en Bold, SIL Open Font License 1.1) ingebakken in de plugin, zodat RoughStart op elke Mac hetzelfde oogt. Bestanden en licentie in `Resources/fonts/`.
- Unit test: venstergrootte wordt naast de standaardpreset bewaard.

### Gewijzigd
- De editor is opgesplitst in `RoughStartView` (de interface op 100%) en `RoughStartEditor` (het plug-invenster dat de view schaalt).
- `settings.xml` wordt bij wijzigen eerst gelezen, zodat standaardpreset en venstergrootte elkaar niet overschrijven.

## [0.8.0] - 2026-11-15

### Toegevoegd
- **Trim in het overzicht**: slepen in het Trim-vakje, dubbelklik zet op 0, Alt-klik om te typen. Wijzigingen gaan als echte parameterwijziging naar de juiste instance (met begin/einde van de gebaar voor undo en automatisering). Tijdens slepen blijft de volgorde van de tabel staan.
- **Mini-meter per spoor** (kolom Level): grijs vóór, groen na Auto, wit streepje voor de piek na Auto. Sorteerbaar op het niveau na Auto.
- **Statuslabels** als gekleurde pil: OK (groen), Ceiling (blauw), No signal / Not analyzed (grijs), Waiting / Measuring (oranje).
- Hint onderin het overzicht over Trim slepen.
- **Undo/Redo voor Trim in het overzicht**: eigen gedeelde undo-geschiedenis (alle sporen), met knoppen onderin en Cmd+Z / Cmd+Shift+Z. Nodig omdat Logic's Undo per plug-invenster werkt en wijzigingen aan andere sporen niet terugdraait.

### Gewijzigd
- Overzicht breder (860 px), hogere regels, getallen zonder "dB" erachter (staat in de handleiding), kanalen als "St"/"Mono".
- `RoughStartSnapshot` rendert nu ook een overzicht met zes sporen.

## [0.7.0] - 2026-11-15

### Gewijzigd
- **Nieuwe Track-weergave**, donker en rustiger:
  - Kop met merkje (golfvorm), naam, spoornaam, tabs Track/Overview en help. Versienummer niet meer in beeld.
  - **Meter** als middelpunt: één balk op een dBFS-schaal met het niveau vóór RoughStart (grijs), na Auto (groen), het doel (gestippeld), het plafond (blauw) en de piek vóór en na. Tijdens meten een oranje live-balk.
  - **Auto** groot rechtsboven; "Peak at ceiling" onder de meter als het plafond ingrijpt.
  - **Trim als grote draaiknop** (slepen, dubbelklik = 0 dB) met Total gain ernaast.
  - **Instellingen in één kaart**: preset, Presets-menu, Apply to all; Peak/RMS-schakelaar; Target en Ceiling als compacte waardevakjes (slepen of dubbelklikken om te typen).
  - Onderbalk: **Analyze track** en **Analyze all (n)** als groene hoofdknop, met de status eronder.
- Eigen LookAndFeel (`Source/RoughStartLookAndFeel.h`) voor knoppen, keuzelijsten, draaiknop, waardevakjes, menu's en tooltips; geldt ook voor het overzicht.
- Handleiding: uitleg van de meter en de nieuwe bediening.

### Toegevoegd
- `RoughStartSnapshot` (ontwikkeltool, `Tools/Snapshot.cpp`): rendert de editor naar PNG om het ontwerp zonder Logic te bekijken. Bouwen met `cmake --build build --target RoughStartSnapshot`.

## [0.6.1] - 2026-11-10

### Toegevoegd
- Instructies voor **Ableton Live** (Mac) in het installatiebriefje (NL + EN) en de handleiding: Audio Units aanzetten, waar RoughStart in de browser staat, plaatsing, kopiëren naar andere sporen, Analyze All. Nog niet in Live getest; testers wordt gevraagd op spoornamen en Analyze All te letten.

## [0.6.0] - 2026-11-10

### Toegevoegd
- **Tooltips** op alle knoppen en instellingen, en in het overzicht op spoornaam en status. Het tooltipvenster is een kind van de editor (geen los venster, dus geen geflikker in Logic).
- **Universal binary**: werkt op Apple Silicon en Intel. Tests en auval slagen op beide architecturen.
- **Testpakket**: `scripts/package.sh` maakt `dist/RoughStart-<versie>.zip` met de plugin, een installatiebriefje en de handleiding, beide als PDF (via `scripts/md2pdf.py` en Chrome headless). Het installatiebriefje (`docs/INSTALL.md`, NL + EN) heeft vier stappen met één Terminal-opdracht, die in de PDF op één regel staat zodat hij goed kopieert.

### Gewijzigd
- Spoorkleuren bewust niet toegevoegd: Audio Units geven in Logic alleen de spoornaam door, geen kleur (alleen VST3 en AAX doen dat).
- Lange spoornamen in het overzicht worden van vóren afgekapt ("…Be With You_Drums"), zodat het onderscheidende einde zichtbaar blijft. De volledige naam staat in de tooltip.

## [0.5.1] - 2026-11-10

### Toegevoegd
- Kanaalformaat zichtbaar: naast de spoornaam staat "Mono" of "Stereo", en het overzicht heeft een kolom **Ch** (sorteerbaar). Zo zie je of Logic bij het invoegen op meerdere sporen per spoor de juiste variant koos.

### Opgelost
- Bij Analyze All bleven instances op sporen zonder audio op "Waiting" staan nadat Logic gestopt was. Logic roept plug-ins op stille sporen soms helemaal niet aan, waardoor ze het afspelen nooit zien. Zodra een ander spoor uit dezelfde Analyze All klaar is, stoppen ze nu ook (geen signaal, Auto blijft staan).
- Overzicht toont dan "No signal" (of "No signal (kept)" als er nog een oudere meting staat), in grijs.

## [0.5.0] - 2026-11-10

### Toegevoegd
- **Factory presets**, altijd aanwezig en niet te wijzigen of te verwijderen: RMS -18 (standard), RMS -24 (dynamic), Peak -12 (classic). Alle met plafond -6. In de keuzelijst gescheiden van eigen presets ("Factory" / "My presets").
- **Eigen standaard**: "Use selected preset as default for new instances" in het Presets-menu. Nieuwe instances starten met die preset; in de lijst staat "(default)" erachter. Opgeslagen in `settings.xml` in de presetmap. Wordt de eigen standaard verwijderd, dan geldt weer de factory-standaard.
- **Handleiding** (Engels): `docs/MANUAL.md`, ingebakken in de plugin en te openen met de knop "?" rechtsboven.
- Unit tests voor factory presets en de standaard.

### Gewijzigd
- Standaard voor nieuwe instances is nu RMS -18 met plafond -6 (was Peak -12). Bestaande projecten houden hun eigen instellingen.
- "Reset settings to defaults" heet nu "Reset to default preset" en gebruikt de ingestelde standaard.
- Factory presets kunnen ook geëxporteerd worden.

## [0.4.1] - 2026-11-10

### Toegevoegd
- Knop **Apply to all** naast Mode: kopieert methode, beide doelen en plafond van dit spoor naar alle andere instances, zonder eerst een preset op te slaan. (Meting en Trim blijven per spoor.)

## [0.4.0] - 2026-11-10

### Toegevoegd
- **Overview**: tweede weergave in de plugin (knoppen Track / Overview rechtsboven) met een tabel van alle RoughStart-instances: Track, Mode, Peak, RMS, Auto, Trim, Total en Status. Kolommen sorteerbaar door op de kop te klikken; niet-gemeten sporen staan bij getalkolommen altijd onderaan. Het eigen spoor is gemarkeerd. Analyze All werkt ook vanuit het overzicht.
- **Presets** voor de doelinstellingen (methode, beide doelen, plafond), als `.rspreset`-bestanden in `~/Library/Application Support/RoughStart/Presets`.
  - Keuzelijst toont de preset die bij de huidige instellingen past, anders "(custom)".
  - Menu "Presets...": opslaan, instellingen op alle sporen toepassen, importeren, exporteren, verwijderen (met bevestiging), presetmap tonen, terug naar standaard.
- Unit tests voor PresetManager (opslaan/laden, namen opschonen, import/export, ongeldige bestanden, begrenzen).
- Naam invullen bij opslaan en bevestigen bij verwijderen gebeurt in een laag binnen de plugin zelf, niet in een los venster (een los venster flikkerde in Logic).

### Gewijzigd
- Schuiven (Target, Peak ceiling, Trim) reageren niet meer op scrollen, zodat ze niet per ongeluk verschuiven.
- Als het plafond ingrijpt heet de status nu "Peak at ceiling", in een neutrale kleur in plaats van oranje: het is informatie, geen fout.

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
