# Myanmar glyph positioning port — CrossInk 1.6.1 / X4 Pro (experimental)

## What was ported

The working CrossPoint implementation established that HarfBuzz glyph IDs, x/y offsets and advances must be consumed by the actual draw function and measured consistently. CrossInk has a **different raster backend**: this source inherits the previous Myanmar Edition HarfBuzz + pre-rendered glyph atlas, not CrossPoint's FreeType glyph-ID rasterizer.

This focused change keeps the existing CrossInk Myanmar integration and ensures `GfxRenderer::drawText` applies HarfBuzz x/y offsets once, advances both axes in 26.6 fixed-point, and does **not collapse glyph spacing when an atlas bitmap is unavailable**. It adds `MyanmarRuntimeFont::shapedGlyph()` to read the metrics independently of atlas lookup. `getTextWidth` and `getTextAdvanceX` continue to use `MyanmarRuntimeFont::widthPixels()`. The existing EPUB parser, line breaks, justification, sleep management, and X4 Pro device profile remain unchanged.

This is an **adaptation** of the validated CrossPoint positioning principles, NOT a byte-for-byte port of its FreeType renderer. The CrossInk atlas still only supports selected pixel sizes. This patch has not been verified on hardware.

## Build

Base release: CrossInk v1.6.1. The actual CrossInk build target is `x4-pro` (different from CrossPoint's `x4pro`). Push this source to a branch named `myanmar-x4pro` in a CrossInk fork with its original `.gitmodules` and `freeink-sdk` checkout intact. The supplied GitHub Actions workflow runs `pio run -e x4-pro -j 2`.

**A downloaded source ZIP does not include the contents of the `freeink-sdk` Git submodule. Do not replace the submodule checkout with the empty ZIP directory.**

## Validation

ZIP and static patch checks only; no PlatformIO toolchain or hardware run in this workspace. Test Myanmar glyphs (`မြန်မာစာ`, `က္က`, `ကျောင်း`, `ကြိုး`, `ကျွန်တော်`) and mixed Latin/Myanmar text. If glyph shapes still differ from working CrossPoint, the next step is a native CrossInk `HalScalableFont` shaping bridge to FreeType glyph IDs, which requires the SDK implementation and device-level instrumentation.

## License

Preserve original CrossInk, third-party HarfBuzz, Myanmar font and other dependency license notices.
