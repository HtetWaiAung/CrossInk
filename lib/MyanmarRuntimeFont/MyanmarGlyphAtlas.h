#pragma once

#include <cstddef>
#include <cstdint>

class MyanmarGlyphAtlas {
 public:
  struct Glyph {
    const uint8_t* bitmap = nullptr;
    uint32_t bitmapLength = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t left = 0;
    int16_t top = 0;
    int32_t advanceX26Dot6 = 0;
  };

  MyanmarGlyphAtlas(const uint8_t* bytes, size_t length)
      : bytes_(bytes), length_(length) {}

  bool valid() const;
  uint32_t glyphCount() const;
  bool find(uint16_t pixelSize, uint32_t glyphId, Glyph& result) const;

 private:
  const uint8_t* bytes_;
  size_t length_;
};
