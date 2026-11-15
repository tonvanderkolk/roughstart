# Roadmap RoughStart

- [x] **v0.1 Proof of concept:** twee of meer instances in Logic 12 vinden elkaar; een knop in één instance laat alle instances reageren. *(getest in Logic Pro 12 op 2026-11-10: instances vinden elkaar, Ping all werkt, spoornamen komen door)*
- [x] **v0.2 Eén instance:** meten (piek/RMS, stiltefilter), Auto-gain, Trim, opslaan in project. *(getest in Logic Pro 12 op 2026-11-10: meten, Auto, Trim en opslaan werken; RMS-modus kan clippen → piekplafond in v0.2.1)*
- [x] **v0.2.1:** piekplafond voor Auto, meeschuivende stiltedrempel, UI in het Engels. *(getest in Logic Pro 12 op 2026-11-10)*
- [x] **v0.2.2:** analyse gekoppeld aan transport (stopt vanzelf als Logic stopt). *(getest in Logic Pro 12 op 2026-11-10)*
- [x] **v0.3 Analyze All:** gezamenlijke analyse over alle instances, gekoppeld aan transport. *(getest in Logic Pro 12 op 2026-11-10; Stop All stopt de analyse, afspelen loopt door)*
- [x] **v0.4 Overzicht:** overzichtsvenster met sorteerbare kolommen, presets import/export. *(getest in Logic Pro 12 op 2026-11-10; CSV-export geschrapt)*
- [x] **v0.4.1:** knop Apply to all. *(getest in Logic Pro 12 op 2026-11-10)*
- [x] **v0.5:** factory presets, eigen standaard voor nieuwe instances, handleiding in de plugin; v0.5.1 toont Mono/Stereo en rondt stille sporen af bij Analyze All. *(getest in Logic Pro 12 op 2026-11-10)*
- [x] **v0.6 Testronde:** spoornamen van vóren afkappen, tooltips, Universal binary, testpakket met installatiebriefje. *(getest in Logic Pro 12 op 2026-11-10)*
- [x] **v0.7 Track-weergave:** nieuwe donkere indeling met meter, Trim-knop, instellingenkaart. *(getest in Logic Pro op 2026-11-15)*
- [x] **v0.8 Overzicht:** Trim in de tabel, mini-meters, statuslabels. Plus eigen Undo/Redo voor Trim. *(getest in Logic Pro op 2026-11-15)*
- [x] **v0.9 Afwerking:** eigen lettertype (Inter), venstergrootte schalen. *(getest in Logic Pro op 2026-11-15)*
- [ ] **v1.0 Afronding:** UI polijsten, `auval` schoon, signing.

## Besluiten
- Auto-bereik ±24 dB, stiltedrempel -60 dBFS absoluut en -20 dB relatief voor RMS (vensters van 50 ms).
- Piekplafond standaard -6 dBFS; geldt alleen voor Auto, Trim mag er bewust overheen.
- UI en parameternamen in het Engels (plugin is bedoeld om later wereldwijd te delen); documentatie blijft Nederlands.
- Meting wordt vastgezet; Auto wordt afgeleid van meting + doel, dus doel wijzigen vraagt geen nieuwe analyse.

## Misschien later
- CSV-export van het overzicht. Alleen nuttig om een mix te delen met iemand zonder RoughStart; stems exporteren of een screenshot van het overzicht dekt dat meestal al.

## Aandachtspunten
- Logic laadt plug-ins op sporen zonder regio's pas als er iets op komt. Die instances bestaan dan nog niet en staan niet in de registry (relevant voor het overzicht in v0.4).

- Spoorkleur is in AU niet beschikbaar: Logic geeft aan Audio Units alleen de naam door.

- Ableton Live (Mac) zou als AU moeten werken (laadt plug-ins in één proces), maar is nog niet getest. Onzeker: of Live spoornamen aan AU doorgeeft.
- Windows/VST3: nodig voor Ableton-gebruikers op Windows. Bouwen via een Windows-runner in GitHub Actions.

## Open vragen
- JUCE-licentie bij publiek delen (AGPLv3 of commerciële licentie).
- Apple Developer ID voor signing en notarisatie.
- Gedrag bij spoor zonder signaal tijdens Analyze All (voorstel: Auto ongewijzigd, markeren in overzicht).
