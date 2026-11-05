# X4 sleep-refresh audit — CrossInk 1.6.1 Myanmar port

## Finding
The RC2 Myanmar fork introduced a `sleepRefreshMode()` helper choosing `HalDisplay::HALF_REFRESH` for X4 and `FULL_REFRESH` for X3 to avoid repeated black/white flashing on the X4 sleep screen. Upstream CrossInk 1.6.1 has *already* explicitly incorporated the key X4 behavior: the sleep-screen paths call `HALF_REFRESH`, and the accompanying comment identifies the OEM single-pass waveform (`0xD7`) versus the multi-flash full waveform (`0xF7`). This means **the X4 sleep fix does not need another functional source patch**.

We deliberately did not copy the entire older SleepActivity.cpp, which would regress new 1.6.1 functionality (including changes to statistics, PNG alpha/grayscale compositing and sleep-folder selection).

## Important qualification
The old fork's helper chose FULL for X3; 1.6.1 uses HALF across these sleep drawing paths. This port targets X4. X3-specific waveform behavior must be assessed separately; do not reintroduce X3 changes without hardware tests.

## Verification performed
- Static source check of 1.6.1 sleep-screen calls to `renderer.displayBuffer(...)` and `renderer.displayGrayscaleBase(...)`.
- Compared original RC2 sleep-refresh implementation with newer 1.6.1 code.
- No firmware compilation and no physical X4 sleep/wake cycling. Screen refresh and deep-sleep current must be validated on-device.

## Target hardware verification
1. Boot X4 and open an English EPUB then Myanmar EPUB; sleep from each.
2. Confirm one sleep-image refresh without multiple high-contrast inversion cycles.
3. Wake from deep sleep, confirm correct restored page, no residual ghosting or stuck panel.
4. Repeat with light/dark sleep screen, BMP cover, grayscale cover and rotated orientation.
5. Test >=20 sleep/wake cycles and inspect consumption, reset reasons and free heap.

## Next release gate
Build `pio run -e default` with the project toolchain and dependencies; resolve compiler/linker failures and test Myanmar text and sleep behavior on the physical X4 before distributing flashable `.bin` files.
