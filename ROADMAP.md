# Roadmap RoughStart

- [x] **v0.1 Proof of concept:** twee of meer instances in Logic 12 vinden elkaar; een knop in één instance laat alle instances reageren. *(getest in Logic Pro 12 op 2026-11-10: instances vinden elkaar, Ping all werkt, spoornamen komen door)*
- [x] **v0.2 Eén instance:** meten (piek/RMS, stiltefilter), Auto-gain, Trim, opslaan in project. *(getest in Logic Pro 12 op 2026-11-10: meten, Auto, Trim en opslaan werken; RMS-modus kan clippen → piekplafond in v0.2.1)*
- [ ] **v0.2.1:** piekplafond voor Auto, meeschuivende stiltedrempel, UI in het Engels.
- [ ] **v0.3 Analyze All:** gezamenlijke analyse over alle instances, gekoppeld aan transport.
- [ ] **v0.4 Overzicht:** overzichtsvenster met sorteerbare kolommen, CSV-export, presets import/export.
- [ ] **v1.0 Afronding:** UI polijsten, `auval` schoon, signing.

## Besluiten
- Auto-bereik ±24 dB, stiltedrempel -60 dBFS (vensters van 50 ms).
- Meting wordt vastgezet; Auto wordt afgeleid van meting + doel, dus doel wijzigen vraagt geen nieuwe analyse.

## Aandachtspunten
- Logic laadt plug-ins op sporen zonder regio's pas als er iets op komt. Die instances bestaan dan nog niet en staan niet in de registry (relevant voor het overzicht in v0.4).

## Open vragen
- JUCE-licentie bij publiek delen (AGPLv3 of commerciële licentie).
- Apple Developer ID voor signing en notarisatie.
- Alleen arm64 of Universal binary.
- Gedrag bij spoor zonder signaal tijdens Analyze All (voorstel: Auto ongewijzigd, markeren in overzicht).
