# RoughStart manual

RoughStart sets the level of every track before you start mixing. Put one instance on each track, press Analyze All, play the song, and every track lands at the same starting level. Your faders stay at 0 dB, so what you hear is already a rough mix. Then you balance by ear with Trim.

## Where to put it

- Audio tracks: the first insert slot.
- Software instrument tracks: the first effect slot, directly after the instrument.
- RoughStart works in mono and in stereo. Logic picks the right version for each track, also when you insert it on several tracks at once. The track name at the top shows which one is in use.
- Logic only loads plug-ins on tracks that contain regions. Empty tracks do not show up until you add something to them.

## Quick start

1. Open RoughStart on any track and choose a preset, or set Mode, Target and Peak ceiling yourself.
2. Click Apply to all. Every RoughStart instance now uses the same settings.
3. Click Analyze All, play the whole song (or a cycle area) and stop.
4. Every track now has its own Auto gain. Open Overview to see all tracks at once.
5. Balance by ear with Trim. Faders stay at 0 dB.

## Peak or RMS

- Peak looks at the loudest single moment of a track and puts that at the target (classic: -12 dBFS). Safe and predictable, but it says little about how loud a track sounds.
- RMS looks at the average level of a track, which is closer to what you hear. Tracks end up sounding roughly equally loud, so you need less Trim afterwards.
- Drums and other punchy material have peaks far above their average. In RMS mode the Peak ceiling stops those peaks from being pushed too high.

## Settings

- Mode: Peak or RMS.
- Target: the level RoughStart aims for, for the chosen mode.
- Peak ceiling: Auto never raises a track's peak above this level (default -6 dBFS). When this happens the status shows Peak at ceiling. That is information, not an error: the track's peak simply ends up at the ceiling.
- Trim: your own adjustment on top of Auto, -24 to +24 dB. Trim can be automated. Double-click to reset it to 0.
- Auto is calculated from the stored measurement and the current Mode, Target and Peak ceiling. Changing those updates Auto right away; you do not need to analyze again.
- Auto is limited to -24 to +24 dB.

## Analyzing

- Analyze measures only this track. Analyze All measures every RoughStart instance in the project at the same time.
- Measuring only happens while Logic is playing. If you click Analyze while Logic is stopped, RoughStart waits for playback.
- When playback stops, the analysis stops and the measurement is stored. You can also click Stop (or Stop All) yourself to measure only part of the song.
- Silence is ignored: passages below -60 dBFS do not count. In RMS mode, short passages more than 20 dB below the track's average (such as decaying drum tails) do not count either.
- RoughStart measures the incoming signal, before its own gain, so analyzing again gives the same result whatever Auto and Trim are set to.
- If a track has no signal during the analysis, its previous measurement and Auto stay as they were. The Overview then shows No signal. Tracks that stay silent during Analyze All finish as soon as the other tracks are done.

## Overview

- Click Overview (top right) to see every RoughStart instance in the project: Track, Ch (Mono or Stereo), Mode, Peak, RMS, Auto, Trim, Total and Status.
- Click a column header to sort; click again to reverse. Tracks that have not been analyzed always sort to the bottom.
- The track you opened RoughStart on is highlighted.

## Presets

- A preset stores Mode, both targets and Peak ceiling. It does not store measurements or Trim.
- Factory presets are always available: RMS -18 (standard), RMS -24 (dynamic) and Peak -12 (classic).
- Presets... menu: save your own presets, apply settings to all tracks, choose the default for new instances, import, export and delete presets.
- Use selected preset as default for new instances: every RoughStart you add from then on starts with that preset.
- Your own presets are files ending in .rspreset in ~/Library/Application Support/RoughStart/Presets. Share them with Export and Import.

## Good to know

- All settings and measurements are saved in the Logic project.
- RoughStart adds no latency and almost no CPU load, even with 80 instances.
- Gain changes are smoothed over 50 ms, so there are no clicks.
- If you send a project to someone who does not have RoughStart, the gain is gone. Bounce or export the tracks first so the gain is part of the audio.
