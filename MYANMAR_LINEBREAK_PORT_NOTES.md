# Myanmar EPUB line-break extension (experimental, uncompiled)

Added conservative Myanmar orthographic-syllable split points in `lib/Epub/Epub/ParsedText.cpp` for words without whitespace. A break is permitted immediately before a Myanmar consonant when the preceding codepoint is Myanmar and not virama U+1039. Breaks do not insert a Latin hyphen. The generic fallback codepoint splitter and Latin hyphenation are disabled for tokens containing Myanmar because they can detach vowel signs or split stacked glyph clusters.

## Limitations

- **This is not Myanmar dictionary segmentation or full Unicode grapheme segmentation.** Some boundaries may still be linguistically inappropriate, and other boundaries may be missed.
- The parser's separate CJK word-token split phase remains unchanged.
- Mixed Latin/Myanmar text still needs per-script font run fallback in the renderer; current HarfBuzz path may not preserve Latin glyphs optimally.
- The font atlas has not been verified for all four EPUB sizes.
- PlatformIO toolchain is not installed; this revision has **not been compiled or tested on the X4**, and it is **not safe to flash**.
- The prior Myanmar X4 sleep fix is not ported.

Next: test source build (`pio run -e default`), verify atlas glyph coverage and mixed-script runs, compare sleep changes against upstream.
