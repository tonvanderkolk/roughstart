# RoughStart manual

RoughStart sets the level of every track before you start mixing. Put one instance on each track, press Analyze All, play the song, and every track lands at the same starting level. Your faders stay at 0 dB, so what you hear is already a rough mix. Then you balance by ear with Trim.

## Where to put it

- Audio tracks: the first insert slot.
- Software instrument tracks: the first effect slot, directly after the instrument.
- RoughStart works in mono and in stereo. Logic picks the right version for each track, also when you insert it on several tracks at once. The track name at the top shows which one is in use.
- Logic only loads plug-ins on tracks that contain regions. Empty tracks do not show up until you add something to them.

## Formats and platforms

- macOS: Audio Unit (Logic Pro, GarageBand, Ableton Live and others) and VST3 (Cubase, Bitwig and others).
- Windows: VST3. On Windows, use Ctrl where this manual says Cmd.

## Ableton Live (Mac)

- Turn on Use Audio Units in Settings > Plug-Ins (Preferences in older versions) and click Rescan. RoughStart is in the browser under Plug-Ins > Audio Units > TvdK.
- Put RoughStart as the first device on each track; on instrument tracks directly after the instrument.
- To add it to more tracks quickly, copy the device (Cmd+C) and paste it (Cmd+V) on other tracks.
- Open the plug-in window with the small icon in the device's title bar.
- Analyze All works while Live plays, in Arrangement or Session view, and stops when you stop playback.
- If the Overview shows Instance 1, Instance 2 and so on instead of track names, Live does not pass track names to Audio Units. Everything else still works.

## Tracks or buses

- RoughStart works on tracks and on buses (aux channels, summing stacks).
- Use it on the tracks or on the buses, not on both. A bus measures the sum of its tracks after their own RoughStart. During Analyze all the tracks still have their old Auto, so the bus would measure a level that is gone once the analysis is done.
- On the tracks: once the tracks are set, the buses usually end up at a sensible level by themselves.
- On the buses: handy when you work with stems or summing stacks and want to set whole groups at once.
- Want both after all? Run Analyze all twice. The first pass sets the tracks; in the second pass the tracks measure the same again (they measure before their own gain) and the buses measure the correct sum.

## Quick start

1. Open RoughStart on any track and choose a preset, or set Mode, Target and Peak ceiling yourself.
2. Click Apply to all. Every RoughStart instance now uses the same settings.
3. Click Analyze All, play the whole song (or a cycle area) and stop.
4. Every track now has its own Auto gain. Open Overview to see all tracks at once.
5. Balance by ear with Trim. Faders stay at 0 dB.

## Reading the meter

- The Track view shows one bar on a dBFS scale.
- Grey: the track's level before RoughStart (RMS or peak, following the Mode).
- Green: the level after Auto.
- Dashed green line: your target. Blue line: the Peak ceiling.
- Thin grey tick: the track's peak before RoughStart. White tick: the peak after Auto.
- When the white tick sits on the blue line, the ceiling is limiting Auto and the meter says Peak at ceiling.

## Peak or RMS

- Peak looks at the loudest single moment of a track and puts that at the target (classic: -12 dBFS). Safe and predictable, but it says little about how loud a track sounds.
- RMS looks at the average level of a track, which is closer to what you hear. Tracks end up sounding roughly equally loud, so you need less Trim afterwards.
- Drums and other punchy material have peaks far above their average. In RMS mode the Peak ceiling stops those peaks from being pushed too high.

## Settings

- Mode: Peak or RMS.
- Target: the level RoughStart aims for, for the chosen mode. Drag the Target and Ceiling boxes left or right, or double-click to type a value.
- Peak ceiling: Auto never raises a track's peak above this level (default -6 dBFS). When this happens the status shows Peak at ceiling. That is information, not an error: the track's peak simply ends up at the ceiling.
- Trim: the big knob. Your own adjustment on top of Auto, -24 to +24 dB. Drag up or down; double-click to reset it to 0. Trim can be automated.
- Auto is calculated from the stored measurement and the current Mode, Target and Peak ceiling. Changing those updates Auto right away; you do not need to analyze again.
- Auto is limited to -24 to +24 dB.

## Analyzing

- Analyze track measures only this track. Analyze all measures every RoughStart instance in the project at the same time; the button shows how many tracks it will measure.
- Measuring only happens while Logic is playing. If you click Analyze while Logic is stopped, RoughStart waits for playback.
- When playback stops, the analysis stops and the measurement is stored. You can also click Stop (or Stop All) yourself to measure only part of the song.
- Silence is ignored: passages below -60 dBFS do not count. In RMS mode, short passages more than 20 dB below the track's average (such as decaying drum tails) do not count either.
- RoughStart measures the incoming signal, before its own gain, so analyzing again gives the same result whatever Auto and Trim are set to.
- If a track has no signal during the analysis, its previous measurement and Auto stay as they were. The Overview then shows No signal. Tracks that stay silent during Analyze All finish as soon as the other tracks are done.

## Overview

- Click Overview (top right) to see every RoughStart instance in the project: Track, Ch (Mono or Stereo), Mode, Level, Peak, RMS, Auto, Trim, Total and Status. Numbers are in dB.
- Click a column header to sort; click again to reverse. Tracks that have not been analyzed always sort to the bottom.
- The track you opened RoughStart on is highlighted.
- Level shows a small meter per track: grey before RoughStart, green after Auto, white tick for the peak after Auto. While analyzing, Level and the Peak and RMS columns show the live measurement in orange.
- Trim can be changed right in the table: drag the value left or right, double-click to reset it to 0, or Alt-click to type a value. You can balance the whole song from this one window. Undo and Redo next to Analyze all (or Cmd+Z / Cmd+Shift+Z while the RoughStart window is active) step back through Trim changes made in the table, across all tracks. Logic's own Undo in the plug-in window header only covers that one track.
- Status: OK, Ceiling (peak at the Peak ceiling), No signal, Not analyzed, or Waiting/Measuring during an analysis.

## Presets

- A preset stores Mode, both targets and Peak ceiling. It does not store measurements or Trim.
- Factory presets are always available: RMS -18 (standard), RMS -24 (dynamic) and Peak -12 (classic).
- Presets... menu: save your own presets, apply settings to all tracks, choose the default for new instances, import, export and delete presets.
- Use selected preset as default for new instances: every RoughStart you add from then on starts with that preset.
- Your own presets are files ending in .rspreset in ~/Library/Application Support/RoughStart/Presets. Share them with Export and Import.

## Good to know

- Window size: click the percentage at the top right to choose 100, 125, 150 or 175%. The choice applies to every RoughStart and is remembered.

- All settings and measurements are saved in the Logic project.
- RoughStart adds no latency and almost no CPU load, even with 80 instances.
- Gain changes are smoothed over 50 ms, so there are no clicks.
- If you send a project to someone who does not have RoughStart, the gain is gone. Bounce or export the tracks first so the gain is part of the audio.

## License

- RoughStart is free and open-source software, licensed under the GNU Affero General Public License v3.0 (AGPL-3.0). You can use it for free, also commercially, and share it. The source code is on GitHub: github.com/tonvanderkolk/roughstart.
- RoughStart is built with the JUCE framework (AGPL-3.0) and uses the Inter typeface (SIL Open Font License 1.1).
- RoughStart comes without any warranty.
