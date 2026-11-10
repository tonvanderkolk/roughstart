# RoughStart installeren (testversie)

Bedankt dat je RoughStart wilt testen! Dit is een testversie die nog niet door Apple is ondertekend. Daarom is er één extra stap nodig.

**Werkt op:** macOS 12 of nieuwer, Apple Silicon en Intel, Logic Pro.

## Installeren

1. Pak het zip-bestand uit (dubbelklik).
2. Open Terminal (Programma's > Hulpprogramma's > Terminal).
3. Plak deze regel en druk op Enter. Hij kopieert de plugin naar de juiste map. Sleep anders `RoughStart.component` zelf naar de map `~/Library/Audio/Plug-Ins/Components` (in de Finder: Ga > Ga naar map...).

   ```
   cp -R ~/Downloads/RoughStart-*/RoughStart.component ~/Library/Audio/Plug-Ins/Components/
   ```

   Staat de uitgepakte map niet in Downloads? Pas het pad dan aan, of sleep de plugin met de hand.

4. Plak deze regel en druk op Enter. Hij haalt de downloadblokkade van macOS van de plugin af:

   ```
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/RoughStart.component
   ```

5. Start Logic Pro (opnieuw). RoughStart staat bij Audio Units > TvdK > RoughStart.

**Ziet Logic de plugin niet, of meldt het dat hij niet werkt?** Ga in Logic naar Logic Pro > Instellingen > Plug-in Manager, zoek RoughStart, selecteer hem en klik op "Reset & Rescan Selection".

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

Vermeld je Mac (Apple Silicon of Intel), macOS-versie en Logic-versie.

---

# Installing RoughStart (test version)

This test version is not yet signed by Apple, so one extra step is needed. Works on macOS 12 or later, Apple Silicon and Intel, Logic Pro.

1. Unzip the file.
2. Copy `RoughStart.component` to `~/Library/Audio/Plug-Ins/Components` (Finder: Go > Go to Folder...).
3. Open Terminal and run:

   ```
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/RoughStart.component
   ```

4. (Re)start Logic Pro. RoughStart is under Audio Units > TvdK > RoughStart. If Logic does not show it, use Plug-in Manager > Reset & Rescan Selection.

Click **?** in the plug-in for the manual. Feedback is very welcome: what was unclear, did the balance after Analyze All make sense, what did you miss? Please mention your Mac (Apple Silicon or Intel), macOS version and Logic version.
