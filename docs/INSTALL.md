# RoughStart installeren (testversie)

Bedankt dat je RoughStart wilt testen! Dit is een testversie die nog niet door Apple is ondertekend. Daarom is er één extra stap nodig.

**Werkt op:** macOS 12 of nieuwer, Apple Silicon en Intel. Logic Pro, en ook Ableton Live op de Mac (zie "Ableton Live" verderop).

## Installeren

### Stap 1: uitpakken
Dubbelklik op het zip-bestand. Er verschijnt een map met daarin `RoughStart.component`.

### Stap 2: naar de pluginmap slepen
Kies in de Finder het menu **Ga > Ga naar map...**, plak `~/Library/Audio/Plug-Ins/Components` en druk op Enter. Sleep `RoughStart.component` naar die map.

### Stap 3: downloadblokkade opheffen
Open Terminal (Programma's > Hulpprogramma's > Terminal). Kopieer de regel hieronder in zijn geheel, plak hem in Terminal en druk op Enter:

```
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/RoughStart.component
```

Je krijgt geen melding terug; dat is goed.

### Stap 4: Logic starten
Start Logic Pro (opnieuw). RoughStart staat bij **Audio Units > TvdK > RoughStart**.

**Ziet Logic de plugin niet, of meldt het dat hij niet werkt?** Ga in Logic naar Logic Pro > Instellingen > Plug-in Manager, zoek RoughStart, selecteer hem en klik op "Reset & Rescan Selection".

### Ableton Live
Doe eerst stap 1 tot en met 3 hierboven. Daarna:

- Open in Live **Settings > Plug-Ins** (in oudere versies: Preferences) en zet **Use Audio Units** aan. Klik op **Rescan**.
- RoughStart staat nu in de browser onder **Plug-Ins > Audio Units > TvdK > RoughStart**.
- Sleep RoughStart als **eerste device** op elk spoor. Bij een instrumentspoor direct ná het instrument.
- Snel op meer sporen: klik op de titelbalk van RoughStart, kopieer met Cmd+C, selecteer een ander spoor en plak met Cmd+V.
- Het venster van de plugin open je met het kleine icoontje (gereedschap of pijltje) in de titelbalk van het device.
- Analyze All werkt hetzelfde als in Logic: klik erop, speel af (Arrangement of Session) en stop.

Dit is in Ableton nog niet getest. Let vooral op of de spoornamen in het overzicht kloppen (anders staat er "Instance 1", "Instance 2"...) en of Analyze All alle sporen meeneemt. Laat het me weten!

## Hoe het werkt

Klik in de plugin rechtsboven op **?** voor de handleiding. In het kort:

1. Zet RoughStart op elk spoor: audiosporen op het eerste insert-slot, instrumentsporen direct na het instrument.
2. Kies op één spoor een preset (bijvoorbeeld RMS -18) en klik op **Apply to all**.
3. Klik op **Analyze All**, speel het nummer af en stop.
4. Klaar: alle sporen staan op een gelijk startniveau, je faders blijven op 0. Balanceer daarna op het gehoor met Trim.

## Verwijderen

Gooi `~/Library/Audio/Plug-Ins/Components/RoughStart.component` in de prullenmand. Je presets staan in `~/Library/Application Support/RoughStart`.

## Feedback

Ik ben benieuwd naar je ervaringen! Vooral:

- Wat was onduidelijk, of werkte anders dan je verwachtte?
- Klopte de balans na Analyze All ongeveer? Welke sporen moest je flink trimmen?
- Gebruikte je Peak of RMS, en welk doel?
- Liep je ergens tegenaan (foutmeldingen, vreemd gedrag, klikken in het geluid)?
- Wat mis je?

Vermeld je Mac (Apple Silicon of Intel), macOS-versie en je DAW met versie (Logic of Ableton).

---

# Installing RoughStart (test version)

This test version is not yet signed by Apple, so one extra step is needed. Works on macOS 12 or later, Apple Silicon and Intel, in Logic Pro and Ableton Live (Mac).

### Step 1: unzip
Double-click the zip file.

### Step 2: move the plug-in
In the Finder choose **Go > Go to Folder...**, paste `~/Library/Audio/Plug-Ins/Components` and press Enter. Drag `RoughStart.component` into that folder.

### Step 3: remove the download block
Open Terminal, paste this line in full and press Enter:

```
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/RoughStart.component
```

### Step 4: start Logic
(Re)start Logic Pro. RoughStart is under Audio Units > TvdK > RoughStart. If Logic does not show it, use Plug-in Manager > Reset & Rescan Selection.

### Ableton Live
After steps 1 to 3: open **Settings > Plug-Ins** (Preferences in older versions), turn on **Use Audio Units** and click **Rescan**. RoughStart is in the browser under Plug-Ins > Audio Units > TvdK. Drag it onto each track as the first device (after the instrument on instrument tracks); copy it to more tracks with Cmd+C / Cmd+V. Not yet tested in Live: please check whether track names show up in the Overview and whether Analyze All includes every track.

Click **?** in the plug-in for the manual. Feedback is very welcome: what was unclear, did the balance after Analyze All make sense, what did you miss? Please mention your Mac (Apple Silicon or Intel), macOS version and your DAW and its version.
