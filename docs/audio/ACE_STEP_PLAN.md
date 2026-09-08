# Fire Escape — AI music plan (ACE-Step 1.5)

**Owner (draft):** Quill (story mood prompts) · Forge wires playback in Godot · Akitti owns hardware/VRAM  
**Status:** Research pass 2026-09-08. Not installed yet.  
**Stack pick:** [ACE-Step 1.5](https://github.com/ace-step/ACE-Step-1.5) (MIT) — Gemini’s open-source suggestion checks out.

---

## Why this fits

- Local, open-source, commercial-friendly training claims (verify license for your ship plan).
- Text → music with style control; covers / repaint later if we need loops that match a reference.
- Base line runs on modest VRAM (<4GB claimed for smaller variants); XL needs ≥12GB (offload) / ≥20GB comfortable.
- We want **balcony mood beds**, not radio singles: short loops, golden-hour / neon dusk, hopeful scrappy — matches Prism mood.

---

## Recommended start (cheap)

1. **Don’t install XL first.** Try default / turbo-ish smaller DiT for stub loops.
2. Generate **instrumental beds only** for M0–M1 (no vocal competition with UI / Ember).
3. Target lengths: 30–90s seamless-ish loops (or 2-bar beds we crossfade in Godot).
4. Quill writes **prompt cards** from story mood; Akitti/Forge run generate on a machine with GPU; assets land in `slice1/assets/audio/` (or `audio/beds/`).

### Prompt card template
```
mood: hopeful scrappy cyberpunk balcony at golden-hour into neon dusk
palette: soft pads, distant city hum, sparse clean guitar or kalimba, no horror drones
bpm: 70–90
structure: seamless loop, no big drop, no vocals
tags: ambient, cyberpunk, warm, DIY, Fire Escape
```

---

## Hardware gate

| Path | Needs | Notes |
|------|--------|--------|
| A. TrinityOrb local | NVIDIA/AMD with enough VRAM | Check GPU before clone |
| B. Quill’s computer (box) | May lack GPU | Good for docs/prompts only |
| C. Cloud GPU later | paid | Only if local too slow |

**Next check:** what GPU does TrinityOrb actually have?

---

## Godot hook (Forge later)

- `AudioStreamPlayer` for bed loop + optional one-shots (Ember gift, door, harvest).
- Mute/duck under UI.
- Do not block M0 win condition on music.

---

## Out of scope for now

- Full soundtrack album
- Partner (Rex) theme until M3 story work
- Vocal songs
- Training custom LoRA (nice later if we have 3–5 reference tracks)

---

## Next actions

1. Akitti: confirm GPU on TrinityOrb (name + VRAM).
2. Quill: write 3 prompt cards (quiet balcony / neighbor glass Ember / inventory/craft focus).
3. On greenlight: clone ACE-Step 1.5, smoke one 30s instrumental, drop WAV into repo as `prototype_bed_v0.wav`.
