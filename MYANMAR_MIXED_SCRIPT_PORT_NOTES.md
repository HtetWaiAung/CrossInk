# Mixed Myanmar / Latin rendering — experimental

In `lib/GfxRenderer/GfxRenderer.cpp`, `splitMixedMyanmarRuns()` partitions a UTF-8 string into Myanmar-script and non-Myanmar runs. Mixed-script `drawText()`, `getTextWidth()`, and `getTextAdvanceX()` now process matching runs consistently. English letters and punctuation use normal CrossInk font handling; Myanmar script runs use the Myanmar runtime shaping path.

## Important limitations
- This is source-only and HAS NOT BEEN COMPILED or hardware-tested.
- The script-run segmentation is simple and does not implement full Unicode UAX #29 grapheme segmentation, UAX #14 line breaking, complex bidirectional text or punctuation attachment. Script change splitting can affect kerning at boundaries; baseline matching requires visual verification.
- Font fallback at unsupported glyphs in Myanmar runs and Myanmar-runtime glyph coverage must be validated.
- The Myanmar width cache, glyph atlas rendering, pixel performance, clipping and alignment need testing on real X4.
- The X4 sleep-refresh fix from the RC2 fork has not yet been ported.
- DO NOT flash until firmware compiles and passes X4 device tests.
