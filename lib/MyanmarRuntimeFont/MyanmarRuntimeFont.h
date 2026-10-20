#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <string>
#include <vector>

#include "MyanmarGlyphAtlas.h"

struct hb_blob_t;
struct hb_face_t;
struct hb_font_t;
struct hb_buffer_t;
struct hb_glyph_info_t;
struct hb_glyph_position_t;

class MyanmarRuntimeFont {
 public:
  struct ShapedGlyph {
    uint32_t glyphId = 0;
    int32_t xAdvance26Dot6 = 0;
    int32_t yAdvance26Dot6 = 0;
    int32_t xOffset26Dot6 = 0;
    int32_t yOffset26Dot6 = 0;
  };

  MyanmarRuntimeFont(const uint8_t* openTypeBytes, size_t openTypeLength,
                     const uint8_t* atlasBytes, size_t atlasLength);
  ~MyanmarRuntimeFont();

  MyanmarRuntimeFont(const MyanmarRuntimeFont&) = delete;
  MyanmarRuntimeFont& operator=(const MyanmarRuntimeFont&) = delete;

  bool ready() const;
  bool shape(const char* utf8, uint16_t pixelSize);
  size_t glyphCount() const {
    return activeGlyphs_ ? activeGlyphs_->size() : 0;
  }
  bool glyph(size_t index, ShapedGlyph& shaped,
             MyanmarGlyphAtlas::Glyph& bitmap) const;
  int widthPixels() const;

  static bool containsMyanmar(const char* utf8);

 private:
  const uint8_t* openTypeBytes_;
  size_t openTypeLength_;
  MyanmarGlyphAtlas atlas_;
  hb_blob_t* blob_ = nullptr;
  hb_face_t* face_ = nullptr;
  hb_font_t* font_ = nullptr;
  hb_buffer_t* buffer_ = nullptr;
  const hb_glyph_info_t* infos_ = nullptr;
  const hb_glyph_position_t* positions_ = nullptr;
  unsigned int shapedCount_ = 0;
  uint16_t pixelSize_ = 0;

  struct CacheEntry {
    uint64_t hash = 0;
    uint16_t pixelSize = 0;
    std::string text;
    std::vector<ShapedGlyph> glyphs;
    int widthPixels = 0;
  };

  static constexpr size_t kMaxCachedEntries = 24;
  static constexpr size_t kMaxCachedGlyphs = 512;
  std::list<CacheEntry> cache_;
  size_t cachedGlyphs_ = 0;
  const std::vector<ShapedGlyph>* activeGlyphs_ = nullptr;
  int activeWidthPixels_ = 0;
};
