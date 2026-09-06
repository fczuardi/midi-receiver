# Slice 017: Pitch Bend Event Output

The receiver already parsed BLE MIDI pitch bend messages for diagnostics. This
slice makes pitch bend part of the reusable instrument output boundary.

`BleMidiInput` now creates a shared `PitchBendEvent` when the BLE-MIDI callback
reports pitch bend:

```cpp
PitchBendEvent{channel, bendValue}
```

The existing serial and display diagnostics still receive the raw diagnostic
callback. Instrument consumers receive the typed event through
`InstrumentEventSink::onPitchBendEvent()`.

This does not decide how pitch bend changes sound. The receiver only normalizes
transport data into a shared event. The buzzer instrument or a later synth
backend owns the musical mapping from `-8192..8191` to frequency or timbre.
