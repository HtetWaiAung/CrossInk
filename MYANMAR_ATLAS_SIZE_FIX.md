# Myanmar EPUB atlas-size compatibility fix (experimental)

Verified the bundled `assets/myanmar_runtime.atlas` MYGA size records: **17, 21, 25 px**. Previous EPUB registration requested 29 and 33 px for 14/16pt reader fonts; `MyanmarGlyphAtlas::find()` returns false when the requested size does not exist, so glyph drawing skips glyphs.

This patch maps 14/16pt reader font IDs to the largest currently available 25 px Myanmar atlas strike. This is a temporary workaround; Myanmar letters may look smaller than Latin letters at larger reader settings.

Not addressed: dynamic SD-card font IDs; Myanmar font shaping init/ready validation; glyph coverage; missing/unsupported font-specific variants; exact baselines.

Compile with PlatformIO `pio run -e default`, test with a **Unicode** (not Zawgyi) Myanmar EPUB, using built-in Bitter/Lexend font at 12, 14, and 16pt. Do not assume build success means correct rendering.
