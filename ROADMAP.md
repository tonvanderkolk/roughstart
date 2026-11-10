# Roadmap RoughStart

- [ ] **v0.1 Proof of concept:** twee of meer instances in Logic 12 vinden elkaar; een knop in één instance laat alle instances reageren. *(gebouwd, test in Logic nog te doen)*
- [ ] **v0.2 Eén instance:** meten (piek/RMS, stiltefilter), Auto-gain, Trim, opslaan in project.
- [ ] **v0.3 Analyze All:** gezamenlijke analyse over alle instances, gekoppeld aan transport.
- [ ] **v0.4 Overzicht:** overzichtsvenster met sorteerbare kolommen, CSV-export, presets import/export.
- [ ] **v1.0 Afronding:** UI polijsten, `auval` schoon, signing.

## Open vragen
- JUCE-licentie bij publiek delen (AGPLv3 of commerciële licentie).
- Apple Developer ID voor signing en notarisatie.
- Alleen arm64 of Universal binary.
- Maximaal correctiebereik Auto (voorstel ±24 dB) en drempel stiltefilter (voorstel -60 dBFS).
- Gedrag bij spoor zonder signaal tijdens Analyze All (voorstel: Auto ongewijzigd, markeren in overzicht).
