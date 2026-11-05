# Myanmar EPUB font routing fix (experimental)

Problem: Myanmar works in UI but is invisible in EPUB. EPUB can resolve a selected reader font to another point-size ID or an SD-card font; the previous mapping only covered a fixed list of font IDs.

Changes:
- Added `GfxRenderer::myanmarPixelSizeForFont()` which uses explicitly registered sizes first, then 21/25 px atlas fallbacks for known built-in or SD-card font IDs.
- Used the same lookup for drawing, width calculation, and glyph advance calculations.
- The bundled atlas supports 17, 21, and 25 px; this is an interim fallback, not full-size scaling.
- Sleep behavior and partitions are untouched.

Testing: Not compiled here. Rebuild with your existing GitHub Actions X4 `default` workflow. Confirm Myanmar text in EPUB using Bitter 12pt, Lexend Deca 12pt, and the selected SD-card font. Check mixed Myanmar/Latin, line breaks and other EPUB text. If still invisible, inspect parser text and glyph-lookup logs. This patch does not prove the EPUB parser passes original Unicode intact.

Installation: Overwrite only `lib/GfxRenderer/GfxRenderer.cpp` and `lib/GfxRenderer/GfxRenderer.h` in your repository's `myanmar-x4` branch, then commit. The full archive also includes these changes.
