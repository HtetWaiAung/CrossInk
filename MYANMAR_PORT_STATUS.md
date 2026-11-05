# CrossInk 1.6.1 + Myanmar source port (EXPERIMENTAL)

This archive is an **uncompiled source integration**, NOT flash-ready firmware.

## Included
- Original upstream CrossInk 1.6.1 source retained as the base.
- HarfBuzzTiny shaping dependency, Myanmar runtime glyph atlas / font and shaped-cache libraries from Myanmar Edition RC2.
- Myanmar font embedding declarations and basic UI shaping hooks added to the 1.6.1 renderer.
- Myanmar translation YAML retained for the latest i18n build generator.

## Outstanding engineering/verification
- Not compiled: PlatformIO toolchain / dependencies unavailable in current environment.
- Verify `MyanmarShapedCacheFile::begin()` lifecycle and duplicate setup on light network boots.
- EPUB font IDs are now registered for Myanmar shaping and getTextAdvanceX() measures shaped runs for line layout. Full EPUB integration is still UNVERIFIED: fallback glyph coverage, mixed Latin/Myanmar runs, word-break boundaries, raster baselines and performance require testing.
- Check mixed Burmese/Latin text runs, line wrapping, truncation and shaping metrics; this basic renderer integration may not satisfy those cases.
- Verify font embedding linker symbol names and flash/RAM budgets.
- X4 sleep-refresh fix reviewed: upstream 1.6.1 already uses HALF_REFRESH for X4 sleep screens; older patch not copied. See MYANMAR_X4_SLEEP_AUDIT.md; hardware regression testing still required.
- No hardware testing and **do not flash until a successful build and device validation**.

Build locally using PlatformIO: `pio run -e default` after satisfying SDK and git submodule dependencies.

## Experimental update: EPUB syllable boundaries
`lib/Epub/Epub/ParsedText.cpp` now supplies conservative Myanmar syllable line-end candidates and suppresses generic codepoint splitting for Myanmar-containing tokens. See `MYANMAR_LINEBREAK_PORT_NOTES.md`. Not compiled or hardware-tested.

## Experimental update: mixed Myanmar / Latin runs
`lib/GfxRenderer/GfxRenderer.cpp` segments mixed-script text, rendering and measuring Myanmar runs separately from other scripts. See `MYANMAR_MIXED_SCRIPT_PORT_NOTES.md`. Source-only; not compiled or tested. The X4 sleep refresh behavior is already in upstream 1.6.1; see MYANMAR_X4_SLEEP_AUDIT.md.
