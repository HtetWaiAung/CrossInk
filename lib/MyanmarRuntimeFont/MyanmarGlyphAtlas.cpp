#include "MyanmarGlyphAtlas.h"

#include <cstring>

namespace {
constexpr size_t kHeaderSize = 16;
constexpr size_t kSizeRecordSize = 8;
constexpr size_t kGlyphRecordSize = 20;

uint16_t readU16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

int16_t readI16(const uint8_t* p) {
  return static_cast<int16_t>(readU16(p));
}

uint32_t readU32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) |
         (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) |
         (static_cast<uint32_t>(p[3]) << 24);
}

int32_t readI32(const uint8_t* p) {
  return static_cast<int32_t>(readU32(p));
}

bool contains(size_t offset, size_t amount, size_t length) {
  return offset <= length && amount <= length - offset;
}
}  // namespace

bool MyanmarGlyphAtlas::valid() const {
  if (!bytes_ || !contains(0, kHeaderSize, length_) ||
      std::memcmp(bytes_, "MYGA", 4) != 0 || readU16(bytes_ + 4) != 1) {
    return false;
  }
  const uint16_t sizeCount = readU16(bytes_ + 6);
  const uint32_t count = readU32(bytes_ + 8);
  const uint32_t bitmapOffset = readU32(bytes_ + 12);
  const uint64_t expectedDirectory =
      kHeaderSize + static_cast<uint64_t>(sizeCount) * kSizeRecordSize +
      static_cast<uint64_t>(sizeCount) * count * kGlyphRecordSize;
  return sizeCount > 0 && count > 0 && expectedDirectory == bitmapOffset &&
         bitmapOffset <= length_;
}

uint32_t MyanmarGlyphAtlas::glyphCount() const {
  return valid() ? readU32(bytes_ + 8) : 0;
}

bool MyanmarGlyphAtlas::find(const uint16_t pixelSize,
                             const uint32_t glyphId,
                             Glyph& result) const {
  if (!valid() || glyphId >= glyphCount()) return false;
  const uint16_t sizeCount = readU16(bytes_ + 6);
  for (uint16_t index = 0; index < sizeCount; ++index) {
    const size_t sizeOffset = kHeaderSize + index * kSizeRecordSize;
    if (!contains(sizeOffset, kSizeRecordSize, length_)) return false;
    const uint8_t* sizeRecord = bytes_ + sizeOffset;
    if (readU16(sizeRecord) != pixelSize) continue;
    const uint32_t tableOffset = readU32(sizeRecord + 4);
    const uint64_t recordOffset =
        static_cast<uint64_t>(tableOffset) +
        static_cast<uint64_t>(glyphId) * kGlyphRecordSize;
    if (recordOffset > SIZE_MAX ||
        !contains(static_cast<size_t>(recordOffset), kGlyphRecordSize, length_)) {
      return false;
    }
    const uint8_t* record = bytes_ + static_cast<size_t>(recordOffset);
    const uint32_t bitmapOffset = readU32(record);
    const uint32_t bitmapLength = readU32(record + 4);
    if (!contains(bitmapOffset, bitmapLength, length_)) return false;
    result.bitmap = bytes_ + bitmapOffset;
    result.bitmapLength = bitmapLength;
    result.width = readU16(record + 8);
    result.height = readU16(record + 10);
    result.left = readI16(record + 12);
    result.top = readI16(record + 14);
    result.advanceX26Dot6 = readI32(record + 16);
    const uint64_t expectedLength =
        (static_cast<uint64_t>(result.width) * result.height + 7) / 8;
    return expectedLength == bitmapLength;
  }
  return false;
}
