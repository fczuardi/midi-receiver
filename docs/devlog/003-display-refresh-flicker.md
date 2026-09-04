# Slice 003: Reducing Display Flicker

While testing the BLE advertising firmware, we noticed that the screen blinked
once per second. The cause was the uptime counter: each uptime change marked the
whole display as dirty, and the display layer cleared the entire screen with
`fillScreen()` before redrawing all text.

That was acceptable for the first BLE slice, but it is a poor habit for the rest
of this project. Display updates are much slower than ordinary state changes.
If we later redraw the whole screen for every MIDI event, the firmware will feel
noisy and may waste time on work that is not musically important.

The improvement in this slice is to make display refresh reasons more explicit:

- BLE state changes request a full display refresh.
- Uptime changes request only an uptime field refresh.

`DisplayView` now draws the static layout separately from the changing uptime
value. Updating uptime clears only a small rectangle where the number appears,
then writes the new value in that same region.

This keeps the code understandable while introducing an important embedded UI
pattern: do not redraw more pixels than necessary. The screen is still simple,
but the structure is ready for future sections such as last MIDI event, active
note count, and connection status.

Verification goal: after flashing, the uptime number should update once per
second without the whole screen visibly blinking.

