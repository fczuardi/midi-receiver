# Slice 004: Raw MIDI Packet Activity

This slice answers one question: after a BLE MIDI client connects, does it write
any data to our MIDI characteristic?

We are still not parsing MIDI commands. The goal is smaller than that. Every
write to the BLE MIDI data characteristic is treated as one raw packet. For each
packet, the firmware records:

- total number of received packets;
- byte length of the most recent packet;
- `millis()` timestamp when the most recent packet arrived.
- a bounded hex snapshot of the most recent packet for serial debugging.

This gives us the next hard fact in the experiment. Slice 002 proved that a
client can discover and connect to the device. This slice proves whether MIDI
activity is actually crossing the BLE link.

The C++ design follows the same callback pattern as connection handling. The BLE
write callback does a bounded amount of work: it records packet activity and
copies a small packet snapshot. A mutex protects that shared snapshot while
`BleMidiPeripheral::update()` transfers the activity into `AppState` and prints
a compact serial log.

The display also stays simple. It shows:

- `MIDI packets`
- `Last bytes`

Those fields update in-place, using small rectangle redraws like the uptime
field from the previous slice. The label and value positions use explicit row
and column constants so the fields line up predictably on the small screen.

During hardware testing, we also observed small 3-byte packets even without
pressing notes. That may be normal MIDI real-time traffic wrapped in BLE MIDI
timestamp bytes, such as Active Sensing or Timing Clock. Logging the raw bytes in
hex lets us confirm what is arriving before we add MIDI parsing.

Acceptance test:

```bash
pio run -t upload --upload-port /dev/ttyACM0
pio device monitor --port /dev/ttyACM0
```

Then connect from a BLE MIDI app and send any note or control message. Expected
result: serial logs show `MIDI RX` with hex bytes, and the packet counter on the
screen increases.
