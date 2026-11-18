# RoughStart installeren

Bedankt dat je RoughStart wilt gebruiken! Vanaf versie 0.11.1 is RoughStart op de Mac door Apple ondertekend en gecontroleerd (genotariseerd). Uitpakken en in de pluginmap zetten is genoeg.

**Werkt op:**

- **Mac:** macOS 12 of nieuwer, Apple Silicon en Intel. Als Audio Unit (Logic Pro, GarageBand, Ableton Live en andere) en als VST3 (bijvoorbeeld Cubase, Bitwig, Reaper).
- **Windows:** 64-bit, als VST3 (bijvoorbeeld Ableton Live, Cubase, Studio One, Reaper). De Windows-versie is nieuw en nog niet in een DAW getest.

Getest in Logic Pro en Cubase Pro 14 op de Mac. Ervaringen met andere programma's zijn welkom.

## Mac

### Stap 1: uitpakken
Dubbelklik op het zip-bestand. Er verschijnt een map met daarin `RoughStart.component` (Audio Unit) en `RoughStart.vst3` (VST3).

### Stap 2: naar de pluginmap slepen
Kies in de Finder het menu **Ga > Ga naar map...**, plak `~/Library/Audio/Plug-Ins/Components` en druk op Enter. Sleep `RoughStart.component` naar die map.

Gebruik je een DAW die VST3 laadt, zoals Cubase of Bitwig? Ga dan ook naar `~/Library/Audio/Plug-Ins/VST3` en sleep `RoughStart.vst3` daarheen. Voor Logic is dat niet nodig.

### Oudere versie of melding van Apple?
Krijg je toch "Apple kon niet verifiëren dat RoughStart geen malware bevat", dan heb je waarschijnlijk een versie van vóór 0.11.1. Download de nieuwste versie, of hef de blokkade op in Terminal (met `sudo` en je Mac-wachtwoord als de plugin in `/Library/Audio/Plug-Ins` staat):

```
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/RoughStart.component
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/RoughStart.vst3
```

### Stap 3: je DAW starten
Start Logic Pro (opnieuw). RoughStart staat bij **Audio Units > TvdK > RoughStart**.

**Ziet Logic de plugin niet, of meldt het dat hij niet werkt?** Ga in Logic naar Logic Pro > Instellingen > Plug-in Manager, zoek RoughStart, selecteer hem en klik op "Reset & Rescan Selection".

### Cubase (Mac)
Cubase onthoudt plugins die bij het zoeken werden geblokkeerd en slaat ze daarna over, ook als de blokkade weg is. Zie je RoughStart niet, bijvoorbeeld omdat een oudere versie eerder werd geblokkeerd:

- Start Cubase opnieuw en open **Studio > VST Plug-in Manager**.
- Staat RoughStart onderaan bij de geblokkeerde plugins (Blocklist)? Selecteer hem en klik op **Reactivate**. Lukt dat niet, klik dan op **Rescan All**.
- RoughStart staat daarna bij de effecten onder **TvdK > RoughStart**.

### Ableton Live (Mac)
Doe eerst stap 1 en 2 hierboven. Daarna:

- Open in Live **Settings > Plug-Ins** (in oudere versies: Preferences) en zet **Use Audio Units** aan. Klik op **Rescan**.
- RoughStart staat nu in de browser onder **Plug-Ins > Audio Units > TvdK > RoughStart**.
- Sleep RoughStart als **eerste device** op elk spoor. Bij een instrumentspoor direct ná het instrument.
- Snel op meer sporen: klik op de titelbalk van RoughStart, kopieer met Cmd+C, selecteer een ander spoor en plak met Cmd+V.
- Het venster van de plugin open je met het kleine icoontje in de titelbalk van het device.
- Analyze All werkt hetzelfde als in Logic: klik erop, speel af (Arrangement of Session) en stop.

## Windows

### Stap 1: uitpakken
Klik met de rechtermuisknop op het zip-bestand en kies **Alles uitpakken**. Er verschijnt een map met daarin de map `RoughStart.vst3`.

### Stap 2: naar de VST3-map kopiëren
Open in de Verkenner `C:\Program Files\Common Files\VST3` en sleep de hele map `RoughStart.vst3` daarheen. Windows vraagt om toestemming als beheerder; klik op **Doorgaan**.

### Stap 3: je DAW starten
Start je DAW (opnieuw) en laat hem zo nodig opnieuw naar plugins zoeken. RoughStart staat bij de VST3-effecten onder **TvdK > RoughStart**.

- **Ableton Live:** Settings > Plug-Ins, zet **Use VST3 Plug-In System Folders** aan en klik op **Rescan**.
- **Cubase, Studio One, Reaper:** die vinden de standaard VST3-map vanzelf.

Let er in het begin op of de spoornamen in het overzicht kloppen (anders staat er "Instance 1", "Instance 2"...) en of Analyze All alle sporen meeneemt.

## Hoe het werkt

Klik in de plugin rechtsboven op **?** voor de handleiding. In het kort:

1. Zet RoughStart op elk spoor: audiosporen op het eerste insert-slot, instrumentsporen direct na het instrument.
2. Kies op één spoor een preset (bijvoorbeeld RMS -18) en klik op **Apply to all**.
   Wil je drums anders instellen dan keys of zang? Kies per spoor een **Group** (Drums, Keys, Vocals...), stel per groep een preset in en klik op **Apply to group**.
3. Klik op **Analyze all**, speel het nummer af en stop.
4. Klaar: alle sporen staan op een gelijk startniveau, je faders blijven op 0. Balanceer daarna op het gehoor met Trim.

Zet RoughStart óf op de sporen óf op de bussen, niet op allebei. Wil je toch allebei, doe Analyze all dan twee keer; de handleiding legt uit waarom.

## Licentie

RoughStart is gratis en open source (AGPL-3.0). De broncode staat op github.com/tonvanderkolk/roughstart.

## Verwijderen

- **Mac:** gooi `RoughStart.component` (in `~/Library/Audio/Plug-Ins/Components`) en eventueel `RoughStart.vst3` (in `~/Library/Audio/Plug-Ins/VST3`) in de prullenmand. Staan ze in `/Library/Audio/Plug-Ins` (zonder `~`), kijk dan daar. Je presets staan in `~/Library/Application Support/RoughStart`.
- **Windows:** verwijder de map `RoughStart.vst3` uit `C:\Program Files\Common Files\VST3`. Je presets staan in `%APPDATA%\RoughStart`.

## Feedback

Ik ben benieuwd naar je ervaringen! Vooral:

- Wat was onduidelijk, of werkte anders dan je verwachtte?
- Klopte de balans na Analyze All ongeveer? Welke sporen moest je flink trimmen?
- Gebruikte je Peak of RMS, en welk doel?
- Liep je ergens tegenaan (foutmeldingen, vreemd gedrag, klikken in het geluid)?
- Wat mis je?

Vermeld je computer (Mac met Apple Silicon of Intel, of Windows), de versie van het besturingssysteem en je DAW met versie. Reacties zijn welkom via github.com/tonvanderkolk/roughstart/discussions.

---

# Installing RoughStart

From version 0.11.1 RoughStart is signed and notarized by Apple on the Mac. Unzip it and move it to the plug-in folder; that is all.

**Works on:** macOS 12 or later (Apple Silicon and Intel) as Audio Unit and VST3, and Windows 64-bit as VST3. Tested in Logic Pro and Cubase Pro 14 on the Mac; the Windows version is new and untested in a DAW.

## Mac

### Step 1: unzip
Double-click the zip file. The folder contains `RoughStart.component` (Audio Unit) and `RoughStart.vst3` (VST3).

### Step 2: move the plug-in
In the Finder choose **Go > Go to Folder...**, paste `~/Library/Audio/Plug-Ins/Components` and press Enter. Drag `RoughStart.component` into that folder. For DAWs that load VST3 (Cubase, Bitwig and others), also drag `RoughStart.vst3` to `~/Library/Audio/Plug-Ins/VST3`.

If macOS still says it cannot verify RoughStart, you have a version older than 0.11.1. Download the latest version, or run `xattr -dr com.apple.quarantine` on the plug-in in Terminal (with `sudo` if it is in `/Library/Audio/Plug-Ins`).

### Step 3: start your DAW
(Re)start your DAW. In Logic, RoughStart is under Audio Units > TvdK > RoughStart. If Logic does not show it, use Plug-in Manager > Reset & Rescan Selection. In Ableton Live, turn on **Use Audio Units** in Settings > Plug-Ins and click **Rescan**. In Cubase, open **Studio > VST Plug-in Manager**; if an older, blocked version of RoughStart is in the blocklist, select it and click **Reactivate** (or **Rescan All**).

## Windows

1. Right-click the zip file and choose **Extract All**.
2. Drag the whole `RoughStart.vst3` folder to `C:\Program Files\Common Files\VST3` and confirm the administrator prompt.
3. (Re)start your DAW and rescan plug-ins if needed. RoughStart is among the VST3 effects under TvdK. In Ableton Live, turn on **Use VST3 Plug-In System Folders** in Settings > Plug-Ins and click **Rescan**.

Click **?** in the plug-in for the manual. Feedback is very welcome at github.com/tonvanderkolk/roughstart/discussions: what was unclear, did the balance after Analyze all make sense, what did you miss? Please mention your computer, operating system and DAW with versions.
