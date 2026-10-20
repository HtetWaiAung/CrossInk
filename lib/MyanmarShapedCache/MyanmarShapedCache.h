#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

class MyanmarShapedCache {
 public:
  struct Bitmap {
    uint16_t pixelSize = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t baseline = 0;
    const uint8_t* data = nullptr;
    uint32_t dataLength = 0;
  };

  MyanmarShapedCache(const uint8_t* bytes, size_t length) : bytes_(bytes), length_(length) {}

  bool valid() const;
  uint32_t entryCount() const;
  bool find(std::string_view utf8, uint16_t pixelSize, Bitmap& result) const;

  static uint64_t keyHash(std::string_view utf8, uint16_t pixelSize);

 private:
  const uint8_t* bytes_ = nullptr;
  size_t length_ = 0;
};
