# EPUB Myanmar shaping extension (experimental)

This revision registers the original X4 reader font IDs (Lexend Deca and Bitter, 10/12/14/16) with Myanmar runtime shaping, retaining the CrossInk 1.6.1 EPUB parser. It also routes `getTextAdvanceX` measurements through the same HarfBuzz-powered Myanmar renderer used by `drawText`.

## Known limitations
- **Not compiled**; PlatformIO and its Espressif framework/toolchain are not installed in this environment.
- Not verified whether atlas has glyphs at pixel sizes 21/25/29/33; unavailable glyphs currently skip drawing.
- Myanmar syllable segmentation in long whitespace-free paragraphs remains unimplemented; current EPUB splitting does not use a Myanmar dictionary or grapheme cluster algorithm.
- Mixed Latin/Myanmar passages may need script-run segmentation and per-run font fallback to preserve Latin glyphs.
- Shaping cached run widths excludes possible followingCp kerning; the actual need must be checked during device tests.
- Unsupported font sizes / SD card reader fonts may not be mapped.
- X4 sleep fix is still unported.

Build check locally with `pio run -e default` after installing PlatformIO and necessary dependencies. Do not flash without a successful build and hardware checks.
