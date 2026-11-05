#pragma once

#include <HalStorage.h>

#include <cstdint>
#include <string_view>
#include <vector>

class MyanmarShapedCacheFile {
 public:
  struct Entry {
    uint64_t hash = 0;
    uint32_t payloadOffset = 0;
    uint16_t pixelSize = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t baseline = 0;
    uint16_t textLength = 0;
    uint32_t bitmapLength = 0;
  };

  bool begin(const char* path = "/.crosspoint/myanmar-shaped-cache.bin");
  void close();
  bool ready() const { return ready_; }
  uint32_t entryCount() const { return entryCount_; }

  bool findMetrics(std::string_view utf8, uint16_t pixelSize, Entry& result);
  bool loadBitmap(const Entry& entry, std::vector<uint8_t>& bitmap);

  static uint64_t keyHash(std::string_view utf8, uint16_t pixelSize);

 private:
  static constexpr uint32_t HEADER_SIZE = 16;
  static constexpr uint32_t ENTRY_SIZE = 28;
  static constexpr uint16_t FORMAT_VERSION = 2;
  static constexpr uint16_t MAX_TEXT_BYTES = 1024;

  bool readEntry(uint32_t index, Entry& result);
  bool textMatches(const Entry& entry, std::string_view utf8);

  HalFile file_;
  uint32_t fileSize_ = 0;
  uint32_t entryCount_ = 0;
  uint32_t payloadOffset_ = 0;
  bool ready_ = false;
  std::vector<char> compareBuffer_;
};
