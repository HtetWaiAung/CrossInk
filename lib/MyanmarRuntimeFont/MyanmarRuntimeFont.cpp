#include "MyanmarRuntimeFont.h"

#include "../HarfBuzzTiny/src/hb-ot.h"

MyanmarRuntimeFont::MyanmarRuntimeFont(const uint8_t* openTypeBytes,
                                       const size_t openTypeLength,
                                       const uint8_t* atlasBytes,
                                       const size_t atlasLength)
    : openTypeBytes_(openTypeBytes),
      openTypeLength_(openTypeLength),
      atlas_(atlasBytes, atlasLength) {
  if (!openTypeBytes_ || openTypeLength_ == 0 || !atlas_.valid()) return;
  blob_ = hb_blob_create(reinterpret_cast<const char*>(openTypeBytes_),
                         static_cast<unsigned int>(openTypeLength_),
                         HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  face_ = hb_face_create(blob_, 0);
  font_ = hb_font_create(face_);
  hb_ot_font_set_funcs(font_);
  buffer_ = hb_buffer_create();
}

MyanmarRuntimeFont::~MyanmarRuntimeFont() {
  if (buffer_) hb_buffer_destroy(buffer_);
  if (font_) hb_font_destroy(font_);
  if (face_) hb_face_destroy(face_);
  if (blob_) hb_blob_destroy(blob_);
}

bool MyanmarRuntimeFont::ready() const {
  return blob_ && face_ && font_ && buffer_ && atlas_.valid() &&
         hb_face_get_glyph_count(face_) == atlas_.glyphCount();
}

bool MyanmarRuntimeFont::shape(const char* utf8, const uint16_t pixelSize) {
  if (!ready() || !utf8 || *utf8 == '\0') return false;

  uint64_t textHash = 0xCBF29CE484222325ULL;
  size_t textLength = 0;
  for (const auto* p = reinterpret_cast<const uint8_t*>(utf8); *p;
       ++p, ++textLength) {
    textHash ^= *p;
    textHash *= 0x100000001B3ULL;
  }
  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    if (it->hash == textHash && it->pixelSize == pixelSize &&
        it->text.size() == textLength && it->text == utf8) {
      cache_.splice(cache_.begin(), cache_, it);
      activeGlyphs_ = &cache_.front().glyphs;
      activeWidthPixels_ = cache_.front().widthPixels;
      pixelSize_ = pixelSize;
      return !activeGlyphs_->empty();
    }
  }

  activeGlyphs_ = nullptr;
  activeWidthPixels_ = 0;
  infos_ = nullptr;
  positions_ = nullptr;
  shapedCount_ = 0;
  pixelSize_ = 0;
  hb_buffer_reset(buffer_);
  hb_buffer_add_utf8(buffer_, utf8, -1, 0, -1);
  hb_buffer_set_direction(buffer_, HB_DIRECTION_LTR);
  hb_buffer_set_script(buffer_, HB_SCRIPT_MYANMAR);
  hb_buffer_set_language(buffer_, hb_language_from_string("my", -1));
  const int scale = static_cast<int>(pixelSize) * 64;
  hb_font_set_scale(font_, scale, scale);
  hb_shape(font_, buffer_, nullptr, 0);
  infos_ = hb_buffer_get_glyph_infos(buffer_, &shapedCount_);
  positions_ = hb_buffer_get_glyph_positions(buffer_, &shapedCount_);
  pixelSize_ = pixelSize;
  if (!infos_ || !positions_ || shapedCount_ == 0) return false;

  CacheEntry entry;
  entry.hash = textHash;
  entry.pixelSize = pixelSize;
  entry.text.assign(utf8, textLength);
  entry.glyphs.reserve(shapedCount_);
  int64_t width26Dot6 = 0;
  for (unsigned int index = 0; index < shapedCount_; ++index) {
    const hb_glyph_info_t& info = infos_[index];
    const hb_glyph_position_t& position = positions_[index];
    entry.glyphs.push_back(
        ShapedGlyph{info.codepoint, position.x_advance, position.y_advance,
                    position.x_offset, position.y_offset});
    width26Dot6 += position.x_advance;
  }
  entry.widthPixels = static_cast<int>((width26Dot6 + 32) / 64);
  cachedGlyphs_ += entry.glyphs.size();
  cache_.push_front(std::move(entry));

  while (cache_.size() > kMaxCachedEntries ||
         (cachedGlyphs_ > kMaxCachedGlyphs && cache_.size() > 1)) {
    cachedGlyphs_ -= cache_.back().glyphs.size();
    cache_.pop_back();
  }
  activeGlyphs_ = &cache_.front().glyphs;
  activeWidthPixels_ = cache_.front().widthPixels;
  return true;
}

bool MyanmarRuntimeFont::glyph(const size_t index, ShapedGlyph& shaped,
                               MyanmarGlyphAtlas::Glyph& bitmap) const {
  if (!activeGlyphs_ || index >= activeGlyphs_->size()) return false;
  shaped = (*activeGlyphs_)[index];
  return atlas_.find(pixelSize_, shaped.glyphId, bitmap);
}

int MyanmarRuntimeFont::widthPixels() const { return activeWidthPixels_; }

bool MyanmarRuntimeFont::containsMyanmar(const char* utf8) {
  if (!utf8) return false;
  const auto* p = reinterpret_cast<const uint8_t*>(utf8);
  while (*p) {
    uint32_t cp = 0;
    if (*p < 0x80) {
      cp = *p++;
    } else if ((*p & 0xE0) == 0xC0 && p[1]) {
      cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
      p += 2;
    } else if ((*p & 0xF0) == 0xE0 && p[1] && p[2]) {
      cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) |
           (p[2] & 0x3F);
      p += 3;
    } else if ((*p & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
      cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
           ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
      p += 4;
    } else {
      ++p;
      continue;
    }
    if ((cp >= 0x1000 && cp <= 0x109F) ||
        (cp >= 0xA9E0 && cp <= 0xA9FF) ||
        (cp >= 0xAA60 && cp <= 0xAA7F)) {
      return true;
    }
  }
  return false;
}
