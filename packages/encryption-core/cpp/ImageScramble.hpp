#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace PicEncrypt {

// ============================================================================
// Internal MD5 Implementation (RFC 1321 compliant)
// ============================================================================
class MD5 {
public:
  static constexpr size_t BLOCK_SIZE = 64;

  MD5() { init(); }
  MD5(const std::string &text) {
    init();
    update(text.c_str(), text.length());
    finalize();
  }

  void update(const void *input, size_t length) {
    const uint8_t *buf = static_cast<const uint8_t *>(input);
    uint32_t current_bytes = count_[0];
    if ((count_[0] += (static_cast<uint32_t>(length) << 3)) < current_bytes) {
      count_[1]++;
    }
    count_[1] += static_cast<uint32_t>(length >> 29);

    current_bytes = (current_bytes >> 3) & 0x3F;
    uint32_t left = 0;

    if (current_bytes) {
      uint32_t available = BLOCK_SIZE - current_bytes;
      if (length < available) {
        std::memcpy(buffer_ + current_bytes, buf, length);
        return;
      }
      std::memcpy(buffer_ + current_bytes, buf, available);
      transform(buffer_);
      left += available;
    }

    while (left + BLOCK_SIZE <= length) {
      transform(buf + left);
      left += BLOCK_SIZE;
    }

    std::memcpy(buffer_, buf + left, length - left);
  }

  void finalize() {
    if (finalized_)
      return;

    uint8_t bits[8];
    encode(count_, bits, 8);

    uint32_t index = (count_[0] >> 3) & 0x3f;
    uint32_t padLen = (index < 56) ? (56 - index) : (120 - index);

    static const uint8_t PADDING[64] = {0x80}; // first bit is 1, rest 0
    update(PADDING, padLen);
    update(bits, 8);

    encode(state_, digest_, 16);
    finalized_ = true;
  }

  const uint8_t *digest() const { return digest_; }

private:
  uint32_t state_[4];
  uint32_t count_[2];
  uint8_t buffer_[BLOCK_SIZE];
  uint8_t digest_[16];
  bool finalized_{false};

  void init() {
    state_[0] = 0x67452301;
    state_[1] = 0xefcdab89;
    state_[2] = 0x98badcfe;
    state_[3] = 0x10325476;
    count_[0] = 0;
    count_[1] = 0;
    finalized_ = false;
    std::memset(buffer_, 0, sizeof(buffer_));
    std::memset(digest_, 0, sizeof(digest_));
  }

  static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) | (~x & z);
  }
  static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) {
    return (x & z) | (y & ~z);
  }
  static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) {
    return x ^ y ^ z;
  }
  static inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) {
    return y ^ (x | ~z);
  }
  static inline uint32_t ROTATE_LEFT(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
  }

  static inline void FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
                        uint32_t x, uint32_t s, uint32_t ac) {
    a = ROTATE_LEFT(a + F(b, c, d) + x + ac, s) + b;
  }
  static inline void GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
                        uint32_t x, uint32_t s, uint32_t ac) {
    a = ROTATE_LEFT(a + G(b, c, d) + x + ac, s) + b;
  }
  static inline void HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
                        uint32_t x, uint32_t s, uint32_t ac) {
    a = ROTATE_LEFT(a + H(b, c, d) + x + ac, s) + b;
  }
  static inline void II(uint32_t &a, uint32_t b, uint32_t c, uint32_t d,
                        uint32_t x, uint32_t s, uint32_t ac) {
    a = ROTATE_LEFT(a + I(b, c, d) + x + ac, s) + b;
  }

  static void decode(const uint8_t *input, uint32_t *output, size_t len) {
    for (size_t i = 0, j = 0; j < len; i++, j += 4) {
      output[i] = (static_cast<uint32_t>(input[j])) |
                  (static_cast<uint32_t>(input[j + 1]) << 8) |
                  (static_cast<uint32_t>(input[j + 2]) << 16) |
                  (static_cast<uint32_t>(input[j + 3]) << 24);
    }
  }

  static void encode(const uint32_t *input, uint8_t *output, size_t len) {
    for (size_t i = 0, j = 0; j < len; i++, j += 4) {
      output[j] = static_cast<uint8_t>(input[i] & 0xff);
      output[j + 1] = static_cast<uint8_t>((input[i] >> 8) & 0xff);
      output[j + 2] = static_cast<uint8_t>((input[i] >> 16) & 0xff);
      output[j + 3] = static_cast<uint8_t>((input[i] >> 24) & 0xff);
    }
  }

  void transform(const uint8_t block[BLOCK_SIZE]) {
    uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
    uint32_t x[16];
    decode(block, x, BLOCK_SIZE);

    // Round 1
    FF(a, b, c, d, x[0], 7, 0xd76aa478);
    FF(d, a, b, c, x[1], 12, 0xe8c7b756);
    FF(c, d, a, b, x[2], 17, 0x242070db);
    FF(b, c, d, a, x[3], 22, 0xc1bdceee);
    FF(a, b, c, d, x[4], 7, 0xf57c0faf);
    FF(d, a, b, c, x[5], 12, 0x4787c62a);
    FF(c, d, a, b, x[6], 17, 0xa8304613);
    FF(b, c, d, a, x[7], 22, 0xfd469501);
    FF(a, b, c, d, x[8], 7, 0x698098d8);
    FF(d, a, b, c, x[9], 12, 0x8b44f7af);
    FF(c, d, a, b, x[10], 17, 0xffff5bb1);
    FF(b, c, d, a, x[11], 22, 0x895cd7be);
    FF(a, b, c, d, x[12], 7, 0x6b901122);
    FF(d, a, b, c, x[13], 12, 0xfd987193);
    FF(c, d, a, b, x[14], 17, 0xa679438e);
    FF(b, c, d, a, x[15], 22, 0x49b40821);

    // Round 2
    GG(a, b, c, d, x[1], 5, 0xf61e2562);
    GG(d, a, b, c, x[6], 9, 0xc040b340);
    GG(c, d, a, b, x[11], 14, 0x265e5a51);
    GG(b, c, d, a, x[0], 20, 0xe9b6c7aa);
    GG(a, b, c, d, x[5], 5, 0xd62f105d);
    GG(d, a, b, c, x[10], 9, 0x02441453);
    GG(c, d, a, b, x[15], 14, 0xd8a1e681);
    GG(b, c, d, a, x[4], 20, 0xe7d3fbc8);
    GG(a, b, c, d, x[9], 5, 0x21e1cde6);
    GG(d, a, b, c, x[14], 9, 0xc33707d6);
    GG(c, d, a, b, x[3], 14, 0xf4d50d87);
    GG(b, c, d, a, x[8], 20, 0x455a14ed);
    GG(a, b, c, d, x[13], 5, 0xa9e3e905);
    GG(d, a, b, c, x[2], 9, 0xfcefa3f8);
    GG(c, d, a, b, x[7], 14, 0x676f02d9);
    GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    // Round 3
    HH(a, b, c, d, x[5], 4, 0xfffa3942);
    HH(d, a, b, c, x[8], 11, 0x8771f681);
    HH(c, d, a, b, x[11], 16, 0x6d9d6122);
    HH(b, c, d, a, x[14], 23, 0xfde5380c);
    HH(a, b, c, d, x[1], 4, 0xa4beea44);
    HH(d, a, b, c, x[4], 11, 0x4bdecfa9);
    HH(c, d, a, b, x[7], 16, 0xf6bb4b60);
    HH(b, c, d, a, x[10], 23, 0xbebfbc70);
    HH(a, b, c, d, x[13], 4, 0x289b7ec6);
    HH(d, a, b, c, x[0], 11, 0xeaa127fa);
    HH(c, d, a, b, x[3], 16, 0xd4ef3085);
    HH(b, c, d, a, x[6], 23, 0x04881d05);
    HH(a, b, c, d, x[9], 4, 0xd9d4d039);
    HH(d, a, b, c, x[12], 11, 0xe6db99e5);
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8);
    HH(b, c, d, a, x[2], 23, 0xc4ac5665);

    // Round 4
    II(a, b, c, d, x[0], 6, 0xf4292244);
    II(d, a, b, c, x[7], 10, 0x432aff97);
    II(c, d, a, b, x[14], 15, 0xab9423a7);
    II(b, c, d, a, x[5], 21, 0xfc93a039);
    II(a, b, c, d, x[12], 6, 0x655b59c3);
    II(d, a, b, c, x[3], 10, 0x8f0ccc92);
    II(c, d, a, b, x[10], 15, 0xffeff47d);
    II(b, c, d, a, x[1], 21, 0x85845dd1);
    II(a, b, c, d, x[8], 6, 0x6fa87e4f);
    II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    II(c, d, a, b, x[6], 15, 0xa3014314);
    II(b, c, d, a, x[13], 21, 0x4e0811a1);
    II(a, b, c, d, x[4], 6, 0xf7537e82);
    II(d, a, b, c, x[11], 10, 0xbd3af235);
    II(c, d, a, b, x[2], 15, 0x2ad7d2bb);
    II(b, c, d, a, x[9], 21, 0xeb86d391);

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
  }
};

// ============================================================================
// Core Image Scramble Data Structures
// ============================================================================

enum class ProcessType { ENCRYPT, DECRYPT };

struct Image {
  std::vector<uint32_t> pixels;
  int width;
  int height;

  Image(std::vector<uint32_t> px, int w, int h)
      : pixels(std::move(px)), width(w), height(h) {}
};

class ImageScramble {
protected:
  std::vector<uint32_t> pixels;
  int width;
  int height;
  int pixelCount;

public:
  ImageScramble(const Image &image)
      : pixels(image.pixels), width(image.width), height(image.height),
        pixelCount(image.width * image.height) {}

  ImageScramble(std::vector<uint32_t> px, int w, int h)
      : pixels(std::move(px)), width(w), height(h), pixelCount(w * h) {}

  virtual ~ImageScramble() = default;

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  virtual Image process(ProcessType processType) = 0;

  virtual Image encrypt() { return process(ProcessType::ENCRYPT); }
  virtual Image decrypt() { return process(ProcessType::DECRYPT); }

protected:
  size_t getCoreCount() const {
    size_t cores = std::thread::hardware_concurrency();
    return cores > 0 ? cores : 4;
  }

  // Helper to safely invoke async tasks
  void invokeAll(std::vector<std::future<void>> &tasks) {
    for (auto &t : tasks) {
      t.get();
    }
  }
};

// ============================================================================
// Base Classes
// ============================================================================

class BasePicEncryptScramble : public ImageScramble {
protected:
  double key;

public:
  BasePicEncryptScramble(const Image &image, double k)
      : ImageScramble(image), key(k) {}

  BasePicEncryptScramble(std::vector<uint32_t> px, int w, int h, double k)
      : ImageScramble(std::move(px), w, h), key(k) {}

protected:
  std::vector<std::pair<double, int>> generateLogistic(double x1, int n) {
    std::vector<std::pair<double, int>> arr(n);
    double x = x1;
    arr.at(0) = {x, 0};
    for (int i = 1; i < n; ++i) {
      x = 3.9999999 * x * (1.0 - x);
      arr.at(i) = {x, i};
    }
    return arr;
  }

  std::vector<int>
  getSortedPositions(std::vector<std::pair<double, int>> &logisticMap,
                     int positionSize) {
    std::stable_sort(
        logisticMap.begin(), logisticMap.end(),
        [](const auto &a, const auto &b) { return a.first < b.first; });
    std::vector<int> positions(positionSize);
    for (int i = 0; i < positionSize; ++i) {
      positions.at(i) = logisticMap.at(i).second;
    }
    return positions;
  }
};

class BasePixelScramble : public ImageScramble {
protected:
  std::string key;

public:
  BasePixelScramble(const Image &image, std::string k)
      : ImageScramble(image), key(std::move(k)) {}

  BasePixelScramble(std::vector<uint32_t> px, int w, int h, std::string k)
      : ImageScramble(std::move(px), w, h), key(std::move(k)) {}

protected:
  std::vector<int> shuffle(int length) {
    std::vector<int> arr(length);
    for (int i = 0; i < length; ++i) {
      arr.at(i) = i;
    }
    for (int i = length - 1; i > 0; --i) {
      std::string text = key + std::to_string(i);
      MD5 md5(text);
      const uint8_t *hash = md5.digest();

      // Java BigInteger.toString(16) behavior with 0 padding to 32 chars:
      // Substring(0,7) corresponds to the highest 28 bits of the big-endian
      // mapped output
      uint32_t b0 = hash[0];
      uint32_t b1 = hash[1];
      uint32_t b2 = hash[2];
      uint32_t b3 = hash[3];
      uint32_t val = (b0 << 20) | (b1 << 12) | (b2 << 4) | (b3 >> 4);

      int rand_val = static_cast<int>(val % (i + 1));
      std::swap(arr.at(rand_val), arr.at(i));
    }
    return arr;
  }

public:
  Image encrypt() override { return process(ProcessType::ENCRYPT); }

  Image decrypt() override { return process(ProcessType::DECRYPT); }
};

// ============================================================================
// Concrete Algorithm Classes
// ============================================================================

class BlockScramble : public BasePixelScramble {
private:
  int xBlockCount;
  int yBlockCount;

public:
  BlockScramble(const Image &image, const std::string &key)
      : BlockScramble(image.pixels, image.width, image.height, key, 32, 32) {}

  BlockScramble(std::vector<uint32_t> pixels, int width, int height,
                const std::string &key)
      : BlockScramble(std::move(pixels), width, height, key, 32, 32) {}

  BlockScramble(std::vector<uint32_t> pixels, int width, int height,
                const std::string &key, int xBC, int yBC)
      : BasePixelScramble(std::move(pixels), width, height, key),
        xBlockCount(xBC), yBlockCount(yBC) {}

  Image process(ProcessType processType) override {
    const std::vector<int> xArray = shuffle(xBlockCount);
    const std::vector<int> yArray = shuffle(yBlockCount);

    const int newWidth = (width % xBlockCount > 0)
                             ? (width + xBlockCount - width % xBlockCount)
                             : width;
    const int newHeight = (height % yBlockCount > 0)
                              ? (height + yBlockCount - height % yBlockCount)
                              : height;

    const int blockWidth = newWidth / xBlockCount;
    const int blockHeight = newHeight / yBlockCount;
    std::vector<uint32_t> newPixels(newWidth * newHeight, 0);

    const int coreCount = static_cast<int>(getCoreCount());
    const int taskCount = std::min(newWidth, coreCount);
    const int step =
        static_cast<int>(std::ceil(static_cast<double>(newWidth) / taskCount));

    std::vector<std::future<void>> tasks;

    for (int k = 0; k < taskCount; ++k) {
      const int begin = k * step;
      const int end = std::min(begin + step, newWidth);

      tasks.push_back(std::async(std::launch::async, [this, processType, begin,
                                                      end, newWidth, newHeight,
                                                      blockWidth, blockHeight,
                                                      &xArray, &yArray,
                                                      &newPixels]() {
        if (processType == ProcessType::ENCRYPT) {
          for (int i = begin; i < end; ++i) {
            for (int j = 0; j < newHeight; ++j) {
              int n = j;
              int m = (xArray.at((n / blockHeight) % xBlockCount) * blockWidth +
                       i) %
                      newWidth;
              m = xArray.at(m / blockWidth) * blockWidth + m % blockWidth;
              n = (yArray.at(m / blockWidth % yBlockCount) * blockHeight + n) %
                  newHeight;
              n = yArray.at(n / blockHeight) * blockHeight + n % blockHeight;
              newPixels.at(i + j * newWidth) =
                  pixels.at(m % width + n % height * width);
            }
          }
        } else {
          for (int i = begin; i < end; ++i) {
            for (int j = 0; j < newHeight; ++j) {
              int n = j;
              int m = (xArray.at((n / blockHeight) % xBlockCount) * blockWidth +
                       i) %
                      newWidth;
              m = xArray.at(m / blockWidth) * blockWidth + m % blockWidth;
              n = (yArray.at(m / blockWidth % yBlockCount) * blockHeight + n) %
                  newHeight;
              n = yArray.at(n / blockHeight) * blockHeight + n % blockHeight;
              newPixels.at(m + n * newWidth) =
                  pixels.at(i % width + j % height * width);
            }
          }
        }
      }));
    }

    invokeAll(tasks);
    return Image(std::move(newPixels), newWidth, newHeight);
  }
};

class PicEncryptRowColumnScramble : public BasePicEncryptScramble {
private:
  static constexpr int maxTaskCount = 50;

public:
  PicEncryptRowColumnScramble(const Image &image, double key)
      : BasePicEncryptScramble(image, key) {}

  PicEncryptRowColumnScramble(std::vector<uint32_t> pixels, int width,
                              int height, double key)
      : BasePicEncryptScramble(std::move(pixels), width, height, key) {}

  Image process(ProcessType processType) override {
    return processType == ProcessType::ENCRYPT ? encrypt() : decrypt();
  }

  Image encrypt() override {
    std::vector<uint32_t> newPixels = pixels;
    double x = key;

    std::vector<std::future<void>> tasks;
    for (int j = 0, offset = 0; j < height; ++j, offset += width) {
      auto logisticMap = generateLogistic(x, width);
      x = logisticMap.at(width - 1).first;

      const int offset2 = offset;
      tasks.push_back(std::async(
          std::launch::async, [this, logisticMap = std::move(logisticMap),
                               offset2, &newPixels]() mutable {
            const auto positions = getSortedPositions(logisticMap, width);
            for (int i = 0; i < width; ++i) {
              pixels.at(i + offset2) = newPixels.at(positions.at(i) + offset2);
            }
          }));
      if (tasks.size() >= maxTaskCount) {
        invokeAll(tasks);
        tasks.clear();
      }
    }
    invokeAll(tasks);
    tasks.clear();

    x = key;
    for (int i = 0; i < width; ++i) {
      auto logisticMap = generateLogistic(x, height);
      x = logisticMap.at(height - 1).first;

      const int i2 = i;
      tasks.push_back(std::async(
          std::launch::async, [this, logisticMap = std::move(logisticMap), i2,
                               &newPixels]() mutable {
            const auto positions = getSortedPositions(logisticMap, height);
            for (int j = 0; j < height; ++j) {
              newPixels.at(i2 + j * width) =
                  pixels.at(i2 + positions.at(j) * width);
            }
          }));
      if (tasks.size() >= maxTaskCount) {
        invokeAll(tasks);
        tasks.clear();
      }
    }
    invokeAll(tasks);
    return Image(std::move(newPixels), width, height);
  }

  Image decrypt() override {
    std::vector<uint32_t> newPixels = pixels;
    double x = key;

    std::vector<std::future<void>> tasks;
    for (int i = 0; i < width; ++i) {
      auto logisticMap = generateLogistic(x, height);
      x = logisticMap.at(height - 1).first;

      const int i2 = i;
      tasks.push_back(std::async(
          std::launch::async, [this, logisticMap = std::move(logisticMap), i2,
                               &newPixels]() mutable {
            const auto positions = getSortedPositions(logisticMap, height);
            for (int j = 0, offset = 0; j < height; ++j, offset += width) {
              pixels.at(i2 + positions.at(j) * width) =
                  newPixels.at(i2 + offset);
            }
          }));
      if (tasks.size() >= maxTaskCount) {
        invokeAll(tasks);
        tasks.clear();
      }
    }
    invokeAll(tasks);
    tasks.clear();

    x = key;
    for (int j = 0, offset = 0; j < height; ++j, offset += width) {
      auto logisticMap = generateLogistic(x, width);
      x = logisticMap.at(width - 1).first;

      const int offset2 = offset;
      tasks.push_back(std::async(
          std::launch::async, [this, logisticMap = std::move(logisticMap),
                               offset2, &newPixels]() mutable {
            const auto positions = getSortedPositions(logisticMap, width);
            for (int i = 0; i < width; ++i) {
              newPixels.at(positions.at(i) + offset2) = pixels.at(i + offset2);
            }
          }));
      if (tasks.size() >= maxTaskCount) {
        invokeAll(tasks);
        tasks.clear();
      }
    }
    invokeAll(tasks);
    return Image(std::move(newPixels), width, height);
  }
};

class PicEncryptRowScramble : public BasePicEncryptScramble {
public:
  PicEncryptRowScramble(const Image &image, double key)
      : BasePicEncryptScramble(image, key) {}

  PicEncryptRowScramble(std::vector<uint32_t> pixels, int width, int height,
                        double key)
      : BasePicEncryptScramble(std::move(pixels), width, height, key) {}

  Image process(ProcessType processType) override {
    auto logisticMap = generateLogistic(key, width);
    std::vector<int> positions = getSortedPositions(logisticMap, width);

    std::vector<uint32_t> newPixels(pixelCount, 0);

    const int taskCount = static_cast<int>(getCoreCount());
    const int step =
        static_cast<int>(std::ceil(static_cast<double>(width) / taskCount));
    const int offset = (height - 1) * width;

    std::vector<std::future<void>> tasks;
    for (int k = 0; k < taskCount; ++k) {
      const int begin = step * k;
      const int end = std::min(begin + step, width);
      if (begin >= end)
        break;

      tasks.push_back(
          std::async(std::launch::async, [this, processType, begin, end, offset,
                                          &positions, &newPixels]() {
            if (processType == ProcessType::ENCRYPT) {
              for (int i = begin; i < end; ++i) {
                const int m = positions.at(i);
                for (int j = offset; j >= 0; j -= width) {
                  newPixels.at(i + j) = pixels.at(m + j);
                }
              }
            } else {
              for (int i = begin; i < end; ++i) {
                const int m = positions.at(i);
                for (int j = offset; j >= 0; j -= width) {
                  newPixels.at(m + j) = pixels.at(i + j);
                }
              }
            }
          }));
    }

    invokeAll(tasks);
    return Image(std::move(newPixels), width, height);
  }
};

class RowPixelScramble : public BasePixelScramble {
public:
  RowPixelScramble(const Image &image, const std::string &key)
      : BasePixelScramble(image, key) {}

  RowPixelScramble(std::vector<uint32_t> pixels, int width, int height,
                   const std::string &key)
      : BasePixelScramble(std::move(pixels), width, height, key) {}

  Image process(ProcessType processType) override {
    const std::vector<int> xArray = shuffle(width);
    std::vector<uint32_t> newPixels(pixelCount, 0);

    const int coreCount = static_cast<int>(getCoreCount());
    const int taskCount = std::min(width, coreCount);
    const int step =
        static_cast<int>(std::ceil(static_cast<double>(width) / taskCount));

    std::vector<std::future<void>> tasks;
    for (int k = 0; k < taskCount; ++k) {
      const int begin = k * step;
      const int end = std::min(begin + step, width);

      tasks.push_back(
          std::async(std::launch::async, [this, processType, begin, end,
                                          &xArray, &newPixels]() {
            if (processType == ProcessType::ENCRYPT) {
              for (int i = begin; i < end; ++i) {
                for (int j = 0; j < height; ++j) {
                  const int m = xArray.at((xArray.at(j % width) + i) % width);
                  newPixels.at(i + j * width) = pixels.at(m + j * width);
                }
              }
            } else {
              for (int i = begin; i < end; ++i) {
                for (int j = 0; j < height; ++j) {
                  const int m = xArray.at((xArray.at(j % width) + i) % width);
                  newPixels.at(m + j * width) = pixels.at(i + j * width);
                }
              }
            }
          }));
    }

    invokeAll(tasks);
    return Image(std::move(newPixels), width, height);
  }
};

class PerPixelScramble : public BasePixelScramble {
public:
  PerPixelScramble(const Image &image, const std::string &key)
      : BasePixelScramble(image, key) {}

  PerPixelScramble(std::vector<uint32_t> pixels, int width, int height,
                   const std::string &key)
      : BasePixelScramble(std::move(pixels), width, height, key) {}

  Image process(ProcessType processType) override {
    const std::vector<int> xArray = shuffle(width);
    const std::vector<int> yArray = shuffle(height);
    std::vector<uint32_t> newPixels(pixelCount, 0);

    const int coreCount = static_cast<int>(getCoreCount());
    const int taskCount = std::min(width, coreCount);
    const int step =
        static_cast<int>(std::ceil(static_cast<double>(width) / taskCount));

    std::vector<std::future<void>> tasks;
    for (int k = 0; k < taskCount; ++k) {
      const int begin = k * step;
      const int end = std::min(begin + step, width);

      tasks.push_back(
          std::async(std::launch::async, [this, processType, begin, end,
                                          &xArray, &yArray, &newPixels]() {
            if (processType == ProcessType::ENCRYPT) {
              for (int i = begin; i < end; ++i) {
                for (int j = 0; j < height; ++j) {
                  const int m = xArray.at((xArray.at(j % width) + i) % width);
                  const int n = yArray.at((yArray.at(m % height) + j) % height);
                  newPixels.at(i + j * width) = pixels.at(m + n * width);
                }
              }
            } else {
              for (int i = begin; i < end; ++i) {
                for (int j = 0; j < height; ++j) {
                  const int m = xArray.at((xArray.at(j % width) + i) % width);
                  const int n = yArray.at((yArray.at(m % height) + j) % height);
                  newPixels.at(m + n * width) = pixels.at(i + j * width);
                }
              }
            }
          }));
    }

    invokeAll(tasks);
    return Image(std::move(newPixels), width, height);
  }
};

class TomatoScramble : public ImageScramble {
private:
  int offset;
  std::vector<int> positions;
  int pos;

  void gilbert2d() {
    positions.resize(pixelCount, 0);
    pos = 0;
    if (width >= height) {
      generate2d(0, 0, width, 0, 0, height);
    } else {
      generate2d(0, 0, 0, height, width, 0);
    }
  }

  int myFloorDiv(int x, int y) {
    int r = x / y;
    if ((x ^ y) < 0 && (r * y != x)) {
      r--;
    }
    return r;
  }

  int mySignum(int x) { return (x > 0) - (x < 0); }

  void generate2d(int x, int y, int ax, int ay, int bx, int by) {
    const int w = std::abs(ax + ay);
    const int h = std::abs(bx + by);
    const int dax = mySignum(ax);
    const int day = mySignum(ay);
    const int dbx = mySignum(bx);
    const int dby = mySignum(by);

    if (h == 1) {
      for (int i = 0; i < w; ++i) {
        positions.at(pos) = x + y * width;
        pos++;
        x += dax;
        y += day;
      }
      return;
    }

    if (w == 1) {
      for (int i = 0; i < h; ++i) {
        positions.at(pos) = x + y * width;
        pos++;
        x += dbx;
        y += dby;
      }
      return;
    }

    int ax2 = myFloorDiv(ax, 2);
    int ay2 = myFloorDiv(ay, 2);
    int bx2 = myFloorDiv(bx, 2);
    int by2 = myFloorDiv(by, 2);
    const int w2 = std::abs(ax2 + ay2);
    const int h2 = std::abs(bx2 + by2);

    if (2 * w > 3 * h) {
      if ((w2 & 1) == 1 && w > 2) {
        ax2 += dax;
        ay2 += day;
      }
      generate2d(x, y, ax2, ay2, bx, by);
      generate2d(x + ax2, y + ay2, ax - ax2, ay - ay2, bx, by);
    } else {
      if ((h2 & 1) == 1 && h > 2) {
        bx2 += dbx;
        by2 += dby;
      }
      generate2d(x, y, bx2, by2, ax2, ay2);
      generate2d(x + bx2, y + by2, ax, ay, bx - bx2, by - by2);
      generate2d(x + (ax - dax) + (bx2 - dbx), y + (ay - day) + (by2 - dby),
                 -bx2, -by2, -(ax - ax2), -(ay - ay2));
    }
  }

public:
  TomatoScramble(const Image &image) : TomatoScramble(image, 1.0) {}

  TomatoScramble(std::vector<uint32_t> pixels, int width, int height)
      : TomatoScramble(std::move(pixels), width, height, 1.0) {}

  TomatoScramble(const Image &image, double key) : ImageScramble(image) {
    offset = static_cast<int>(
        std::round((std::sqrt(5.0) - 1.0) / 2.0 * pixelCount * key));
  }

  TomatoScramble(std::vector<uint32_t> pixels, int width, int height,
                 double key)
      : ImageScramble(std::move(pixels), width, height) {
    offset = static_cast<int>(
        std::round((std::sqrt(5.0) - 1.0) / 2.0 * pixelCount * key));
  }

  Image process(ProcessType processType) override {
    gilbert2d();
    const int loopPosition = pixelCount - offset;
    std::vector<uint32_t> newPixels(pixelCount, 0);

    if (pixelCount > 10000) {
      const int taskCount = static_cast<int>(getCoreCount());
      std::vector<std::future<void>> tasks;
      const int step = static_cast<int>(
          std::ceil(static_cast<double>(pixelCount) / taskCount));

      for (int i = 0; i < taskCount; ++i) {
        const int begin = step * i;
        const int end = std::min(begin + step, pixelCount);

        tasks.push_back(
            std::async(std::launch::async, [this, processType, begin, end,
                                            loopPosition, &newPixels]() {
              if (processType == ProcessType::ENCRYPT) {
                if (begin >= loopPosition) {
                  for (int j = begin; j < end; ++j) {
                    newPixels.at(positions.at(j - loopPosition)) =
                        pixels.at(positions.at(j));
                  }
                } else if (end <= loopPosition) {
                  for (int j = begin; j < end; ++j) {
                    newPixels.at(positions.at(j + offset)) =
                        pixels.at(positions.at(j));
                  }
                } else {
                  for (int j = begin; j < loopPosition; ++j) {
                    newPixels.at(positions.at(j + offset)) =
                        pixels.at(positions.at(j));
                  }
                  for (int j = loopPosition; j < end; ++j) {
                    newPixels.at(positions.at(j - loopPosition)) =
                        pixels.at(positions.at(j));
                  }
                }
              } else {
                if (begin >= loopPosition) {
                  for (int j = begin; j < end; ++j) {
                    newPixels.at(positions.at(j)) =
                        pixels.at(positions.at(j - loopPosition));
                  }
                } else if (end <= loopPosition) {
                  for (int j = begin; j < end; ++j) {
                    newPixels.at(positions.at(j)) =
                        pixels.at(positions.at(j + offset));
                  }
                } else {
                  for (int j = begin; j < loopPosition; ++j) {
                    newPixels.at(positions.at(j)) =
                        pixels.at(positions.at(j + offset));
                  }
                  for (int j = loopPosition; j < end; ++j) {
                    newPixels.at(positions.at(j)) =
                        pixels.at(positions.at(j - loopPosition));
                  }
                }
              }
            }));
      }

      invokeAll(tasks);
    } else {
      if (processType == ProcessType::ENCRYPT) {
        for (int i = 0; i < loopPosition; ++i) {
          newPixels.at(positions.at(i + offset)) = pixels.at(positions.at(i));
        }
        for (int i = loopPosition; i < pixelCount; ++i) {
          newPixels.at(positions.at(i - loopPosition)) =
              pixels.at(positions.at(i));
        }
      } else {
        for (int i = 0; i < loopPosition; ++i) {
          newPixels.at(positions.at(i)) = pixels.at(positions.at(i + offset));
        }
        for (int i = loopPosition; i < pixelCount; ++i) {
          newPixels.at(positions.at(i)) =
              pixels.at(positions.at(i - loopPosition));
        }
      }
    }

    // Clean up to save memory
    positions.clear();
    return Image(std::move(newPixels), width, height);
  }
};

} // namespace PicEncrypt
