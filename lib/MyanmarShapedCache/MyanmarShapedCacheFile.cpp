#include "MyanmarShapedCacheFile.h"

#include <Logging.h>

#include <cstring>
#include <limits>

namespace {
uint16_t readU16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8)); }

int16_t readI16(const uint8_t* p) { return static_cast<int16_t>(readU16(p)); }

uint32_t readU32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t readU64(const uint8_t* p) {
  return static_cast<uint64_t>(readU32(p)) | (static_cast<uint64_t>(readU32(p + 4)) << 32);
}
}  // namespace

bool MyanmarShapedCacheFile::begin(const char* path) {
  close();
  if (!Storage.openFileForRead("MYSC", path, file_)) return false;

  fileSize_ = static_cast<uint32_t>(file_.fileSize());
  uint8_t header[HEADER_SIZE];
  if (fileSize_ < HEADER_SIZE || file_.read(header, sizeof(header)) != sizeof(header) ||
      std::memcmp(header, "MYSC", 4) != 0 || readU16(header + 4) != FORMAT_VERSION) {
    LOG_ERR("MYSC", "Invalid shaped-cache header");
    close();
    return false;
  }

  entryCount_ = readU32(header + 8);
  payloadOffset_ = readU32(header + 12);
  const uint64_t expectedDirectoryEnd =
      static_cast<uint64_t>(HEADER_SIZE) + static_cast<uint64_t>(entryCount_) * ENTRY_SIZE;
  if (entryCount_ == 0 || expectedDirectoryEnd != payloadOffset_ || payloadOffset_ > fileSize_) {
    LOG_ERR("MYSC", "Invalid shaped-cache directory (%lu entries)", static_cast<unsigned long>(entryCount_));
    close();
    return false;
  }

  compareBuffer_.reserve(128);
  ready_ = true;
  LOG_INF("MYSC", "Opened %s (%lu entries)", path, static_cast<unsigned long>(entryCount_));
  return true;
}

void MyanmarShapedCacheFile::close() {
  if (file_) file_.close();
  file_ = HalFile();
  fileSize_ = 0;
  entryCount_ = 0;
  payloadOffset_ = 0;
  ready_ = false;
  compareBuffer_.clear();
}

uint64_t MyanmarShapedCacheFile::keyHash(const std::string_view utf8, const uint16_t pixelSize) {
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

bool MyanmarShapedCacheFile::readEntry(const uint32_t index, Entry& result) {
  if (!ready_ || index >= entryCount_) return false;
  const uint32_t offset = HEADER_SIZE + index * ENTRY_SIZE;
  uint8_t bytes[ENTRY_SIZE];
  if (!file_.seekSet(offset) || file_.read(bytes, sizeof(bytes)) != sizeof(bytes)) return false;

  result.hash = readU64(bytes);
  result.payloadOffset = readU32(bytes + 8);
  result.pixelSize = readU16(bytes + 12);
  result.width = readU16(bytes + 14);
  result.height = readU16(bytes + 16);
  result.baseline = readI16(bytes + 18);
  result.textLength = readU16(bytes + 20);
  result.bitmapLength = readU32(bytes + 24);

  const uint64_t payloadEnd = static_cast<uint64_t>(result.payloadOffset) + result.textLength + result.bitmapLength;
  const uint64_t expectedBitmapLength =
      (static_cast<uint64_t>(result.width) * static_cast<uint64_t>(result.height) + 7) / 8;
  return result.payloadOffset >= payloadOffset_ && result.textLength <= MAX_TEXT_BYTES &&
         result.bitmapLength == expectedBitmapLength && payloadEnd <= fileSize_;
}

bool MyanmarShapedCacheFile::textMatches(const Entry& entry, const std::string_view utf8) {
  if (entry.textLength != utf8.size()) return false;
  compareBuffer_.resize(entry.textLength);
  return file_.seekSet(entry.payloadOffset) &&
         file_.read(compareBuffer_.data(), compareBuffer_.size()) == static_cast<int>(compareBuffer_.size()) &&
         std::memcmp(compareBuffer_.data(), utf8.data(), utf8.size()) == 0;
}

bool MyanmarShapedCacheFile::findMetrics(const std::string_view utf8, const uint16_t pixelSize, Entry& result) {
  if (!ready_ || utf8.empty() || utf8.size() > MAX_TEXT_BYTES) return false;
  const uint64_t wanted = keyHash(utf8, pixelSize);

  uint32_t lo = 0;
  uint32_t hi = entryCount_;
  while (lo < hi) {
    const uint32_t mid = lo + (hi - lo) / 2;
    Entry candidate;
    if (!readEntry(mid, candidate)) return false;
    if (candidate.hash < wanted) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }

  for (uint32_t index = lo; index < entryCount_; ++index) {
    Entry candidate;
    if (!readEntry(index, candidate)) return false;
    if (candidate.hash != wanted) break;
    if (candidate.pixelSize == pixelSize && textMatches(candidate, utf8)) {
      result = candidate;
      return true;
    }
  }
  return false;
}

bool MyanmarShapedCacheFile::loadBitmap(const Entry& entry, std::vector<uint8_t>& bitmap) {
  if (!ready_ || entry.bitmapLength == 0) return false;
  const uint64_t bitmapOffset = static_cast<uint64_t>(entry.payloadOffset) + entry.textLength;
  if (bitmapOffset + entry.bitmapLength > fileSize_ || bitmapOffset > std::numeric_limits<uint32_t>::max()) return false;
  bitmap.resize(entry.bitmapLength);
  return file_.seekSet(static_cast<uint32_t>(bitmapOffset)) &&
         file_.read(bitmap.data(), bitmap.size()) == static_cast<int>(bitmap.size());
}
