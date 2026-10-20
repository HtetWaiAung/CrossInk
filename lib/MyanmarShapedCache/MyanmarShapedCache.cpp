#include "MyanmarShapedCache.h"

#include <cstring>
#include <limits>

namespace {
constexpr size_t HEADER_SIZE = 16;
constexpr size_t ENTRY_SIZE = 28;

uint16_t readU16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8)); }

int16_t readI16(const uint8_t* p) { return static_cast<int16_t>(readU16(p)); }

uint32_t readU32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t readU64(const uint8_t* p) {
  return static_cast<uint64_t>(readU32(p)) | (static_cast<uint64_t>(readU32(p + 4)) << 32);
}

bool canAdvance(size_t cursor, size_t amount, size_t length) {
  return amount <= length && cursor <= length - amount;
}
}  // namespace

bool MyanmarShapedCache::valid() const {
  if (!bytes_ || length_ < HEADER_SIZE || std::memcmp(bytes_, "MYSC", 4) != 0 || readU16(bytes_ + 4) != 2) {
    return false;
  }
  const uint64_t directoryEnd = HEADER_SIZE + static_cast<uint64_t>(readU32(bytes_ + 8)) * ENTRY_SIZE;
  return directoryEnd == readU32(bytes_ + 12) && directoryEnd <= length_;
}

uint32_t MyanmarShapedCache::entryCount() const { return valid() ? readU32(bytes_ + 8) : 0; }

uint64_t MyanmarShapedCache::keyHash(const std::string_view utf8, const uint16_t pixelSize) {
  uint64_t value = 0xCBF29CE484222325ULL;
  const uint8_t sizeBytes[] = {static_cast<uint8_t>(pixelSize & 0xFF), static_cast<uint8_t>(pixelSize >> 8)};
  for (const uint8_t byte : sizeBytes) {
    value ^= byte;
    value *= 0x100000001B3ULL;
  }
  for (const unsigned char byte : utf8) {
    value ^= byte;
    value *= 0x100000001B3ULL;
  }
  return value;
}

bool MyanmarShapedCache::find(const std::string_view utf8, const uint16_t pixelSize, Bitmap& result) const {
  if (!valid() || utf8.size() > std::numeric_limits<uint16_t>::max()) return false;

  const uint64_t wantedHash = keyHash(utf8, pixelSize);
  const uint32_t count = entryCount();
  uint32_t lo = 0;
  uint32_t hi = count;
  while (lo < hi) {
    const uint32_t mid = lo + (hi - lo) / 2;
    const size_t offset = HEADER_SIZE + static_cast<size_t>(mid) * ENTRY_SIZE;
    if (!canAdvance(offset, ENTRY_SIZE, length_)) return false;
    if (readU64(bytes_ + offset) < wantedHash) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }

  for (uint32_t index = lo; index < count; ++index) {
    const size_t directoryOffset = HEADER_SIZE + static_cast<size_t>(index) * ENTRY_SIZE;
    if (!canAdvance(directoryOffset, ENTRY_SIZE, length_)) return false;
    const uint8_t* entry = bytes_ + directoryOffset;
    const uint64_t storedHash = readU64(entry);
    if (storedHash != wantedHash) break;
    const uint32_t payloadOffset = readU32(entry + 8);
    const uint16_t storedSize = readU16(entry + 12);
    const uint16_t width = readU16(entry + 14);
    const uint16_t height = readU16(entry + 16);
    const int16_t baseline = readI16(entry + 18);
    const uint16_t textLength = readU16(entry + 20);
    const uint32_t bitmapLength = readU32(entry + 24);
    const size_t payloadLength = static_cast<size_t>(textLength) + bitmapLength;
    const uint64_t expectedBitmapLength = (static_cast<uint64_t>(width) * height + 7) / 8;
    if (bitmapLength != expectedBitmapLength || !canAdvance(payloadOffset, payloadLength, length_)) return false;
    const uint8_t* text = bytes_ + payloadOffset;
    const uint8_t* bitmap = text + textLength;

    if (storedHash == wantedHash && storedSize == pixelSize && textLength == utf8.size() &&
        std::memcmp(text, utf8.data(), textLength) == 0) {
      result = Bitmap{storedSize, width, height, baseline, bitmap, bitmapLength};
      return true;
    }
  }
  return false;
}
