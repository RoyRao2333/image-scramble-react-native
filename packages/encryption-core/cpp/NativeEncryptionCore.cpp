#include "NativeEncryptionCore.h"
#include "ImageScramble.hpp"
#include "stb_image.h"
#include "stb_image_write.h"
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace facebook::react {

NativeEncryptionCore::NativeEncryptionCore(
    std::shared_ptr<CallInvoker> jsInvoker)
    : NativeEncryptionCoreCxxSpec(std::move(jsInvoker)) {}

// ============================================================================
// 内部辅助函数
// ============================================================================
namespace {

/// 从 jsi::Object 中提取 ArrayBuffer，并将像素数据拷贝到 std::vector<uint32_t>
std::vector<uint32_t> extractPixels(jsi::Runtime &rt, jsi::Object &bufferObj,
                                    int expectedPixelCount) {
  if (!bufferObj.isArrayBuffer(rt)) {
    throw jsi::JSError(rt, "royrao: pixelBuffer 参数必须是 ArrayBuffer 类型");
  }
  auto arrayBuffer = bufferObj.getArrayBuffer(rt);
  size_t byteLength = arrayBuffer.size(rt);
  size_t expectedBytes =
      static_cast<size_t>(expectedPixelCount) * sizeof(uint32_t);

  if (byteLength < expectedBytes) {
    throw jsi::JSError(rt, "royrao: ArrayBuffer 字节长度不足，期望 " +
                               std::to_string(expectedBytes) + " 字节，实际 " +
                               std::to_string(byteLength) + " 字节");
  }

  const uint32_t *data =
      reinterpret_cast<const uint32_t *>(arrayBuffer.data(rt));
  return std::vector<uint32_t>(data, data + expectedPixelCount);
}

/// 将 PicEncrypt::Image 结果打包为 JS Object { pixels: ArrayBuffer, width,
/// height }
jsi::Object packResult(jsi::Runtime &rt, const PicEncrypt::Image &img) {
  size_t byteSize = img.pixels.size() * sizeof(uint32_t);

  // 通过 JS 构造函数创建 ArrayBuffer
  jsi::Function abCtor = rt.global().getPropertyAsFunction(rt, "ArrayBuffer");
  jsi::Object abObj =
      abCtor.callAsConstructor(rt, static_cast<double>(byteSize)).asObject(rt);
  jsi::ArrayBuffer ab = abObj.getArrayBuffer(rt);
  std::memcpy(ab.data(rt), img.pixels.data(), byteSize);

  jsi::Object result(rt);
  result.setProperty(rt, "pixels", std::move(abObj));
  result.setProperty(rt, "width", static_cast<double>(img.width));
  result.setProperty(rt, "height", static_cast<double>(img.height));
  return result;
}

/// 统一的加密/解密处理入口
jsi::Object processScramble(jsi::Runtime &rt, jsi::Object pixelBuffer,
                            double width, double height, jsi::String algorithm,
                            jsi::String key,
                            PicEncrypt::ProcessType processType) {
  int w = static_cast<int>(width);
  int h = static_cast<int>(height);
  int pixelCount = w * h;

  if (w <= 0 || h <= 0) {
    throw jsi::JSError(rt, "royrao: width 和 height 必须为正整数");
  }

  std::string algoStr = algorithm.utf8(rt);
  std::string keyStr = key.utf8(rt);

  std::vector<uint32_t> pixels;
  try {
    pixels = extractPixels(rt, pixelBuffer, pixelCount);
  } catch (const jsi::JSError &) {
    throw; // 重新抛出 JSError
  }

  PicEncrypt::Image result({}, 0, 0);

  try {
    if (algoStr == "tomato") {
      double k = 1.0;
      try {
        k = std::stod(keyStr);
      } catch (...) {
      }
      PicEncrypt::TomatoScramble scramble(pixels, w, h, k);
      result = (processType == PicEncrypt::ProcessType::ENCRYPT)
                   ? scramble.encrypt()
                   : scramble.decrypt();

    } else if (algoStr == "block") {
      PicEncrypt::BlockScramble scramble(pixels, w, h, keyStr);
      result = (processType == PicEncrypt::ProcessType::ENCRYPT)
                   ? scramble.encrypt()
                   : scramble.decrypt();

    } else if (algoStr == "perPixel") {
      PicEncrypt::PerPixelScramble scramble(pixels, w, h, keyStr);
      result = (processType == PicEncrypt::ProcessType::ENCRYPT)
                   ? scramble.encrypt()
                   : scramble.decrypt();

    } else if (algoStr == "rowPixel") {
      PicEncrypt::RowPixelScramble scramble(pixels, w, h, keyStr);
      result = (processType == PicEncrypt::ProcessType::ENCRYPT)
                   ? scramble.encrypt()
                   : scramble.decrypt();

    } else if (algoStr == "picEncryptRow") {
      double k = 0.5;
      try {
        k = std::stod(keyStr);
      } catch (...) {
      }
      PicEncrypt::PicEncryptRowScramble scramble(pixels, w, h, k);
      result = (processType == PicEncrypt::ProcessType::ENCRYPT)
                   ? scramble.encrypt()
                   : scramble.decrypt();

    } else if (algoStr == "picEncryptRowColumn") {
      double k = 0.5;
      try {
        k = std::stod(keyStr);
      } catch (...) {
      }
      PicEncrypt::PicEncryptRowColumnScramble scramble(pixels, w, h, k);
      result = (processType == PicEncrypt::ProcessType::ENCRYPT)
                   ? scramble.encrypt()
                   : scramble.decrypt();

    } else {
      throw jsi::JSError(rt, "royrao: 不支持的混淆算法: " + algoStr +
                                 "，可选: tomato, block, perPixel, rowPixel, "
                                 "picEncryptRow, picEncryptRowColumn");
    }
  } catch (const std::out_of_range &e) {
    // ImageScramble.hpp 的 .at() 越界异常 → 转为 JS 层可捕获的错误
    throw jsi::JSError(
        rt,
        std::string("royrao: 像素处理越界异常 (已安全拦截，App 不会崩溃): ") +
            e.what());
  } catch (const jsi::JSError &) {
    throw; // 重新抛出 JSError
  } catch (const std::exception &e) {
    throw jsi::JSError(rt, std::string("royrao: 像素处理异常: ") + e.what());
  }

  return packResult(rt, result);
}

// ============================================================================
// Base64 编码表和编码函数
// ============================================================================

static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64Encode(const uint8_t *data, size_t len) {
  std::string result;
  result.reserve(((len + 2) / 3) * 4);

  for (size_t i = 0; i < len; i += 3) {
    uint32_t octet_a = data[i];
    uint32_t octet_b = (i + 1 < len) ? data[i + 1] : 0;
    uint32_t octet_c = (i + 2 < len) ? data[i + 2] : 0;
    uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

    result.push_back(BASE64_CHARS[(triple >> 18) & 0x3F]);
    result.push_back(BASE64_CHARS[(triple >> 12) & 0x3F]);
    result.push_back((i + 1 < len) ? BASE64_CHARS[(triple >> 6) & 0x3F] : '=');
    result.push_back((i + 2 < len) ? BASE64_CHARS[triple & 0x3F] : '=');
  }
  return result;
}

/// stb_image_write PNG 回调：将 PNG 数据追加到 std::vector<uint8_t>
void stbiWriteCallback(void *context, void *data, int size) {
  auto *buffer = static_cast<std::vector<uint8_t> *>(context);
  auto *bytes = static_cast<const uint8_t *>(data);
  buffer->insert(buffer->end(), bytes, bytes + size);
}

} // anonymous namespace

// ============================================================================
// TurboModule 接口实现
// ============================================================================

jsi::Object NativeEncryptionCore::encrypt(jsi::Runtime &rt,
                                          jsi::Object pixelBuffer, double width,
                                          double height, jsi::String algorithm,
                                          jsi::String key) {
  return processScramble(rt, std::move(pixelBuffer), width, height,
                         std::move(algorithm), std::move(key),
                         PicEncrypt::ProcessType::ENCRYPT);
}

jsi::Object NativeEncryptionCore::decrypt(jsi::Runtime &rt,
                                          jsi::Object pixelBuffer, double width,
                                          double height, jsi::String algorithm,
                                          jsi::String key) {
  return processScramble(rt, std::move(pixelBuffer), width, height,
                         std::move(algorithm), std::move(key),
                         PicEncrypt::ProcessType::DECRYPT);
}

jsi::Object NativeEncryptionCore::readImagePixels(jsi::Runtime &rt,
                                                  jsi::String filePath) {
  std::string path = filePath.utf8(rt);

  // 使用 fstream 读取文件到内存（STBI_NO_STDIO 模式）
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw jsi::JSError(rt,
                       "royrao: 无法打开图片文件: " + path);
  }

  auto fileSize = file.tellg();
  if (fileSize <= 0) {
    throw jsi::JSError(rt, "royrao: 图片文件为空或无法读取: " + path);
  }

  file.seekg(0, std::ios::beg);
  std::vector<uint8_t> fileData(static_cast<size_t>(fileSize));
  if (!file.read(reinterpret_cast<char *>(fileData.data()),
                 static_cast<std::streamsize>(fileSize))) {
    throw jsi::JSError(rt, "royrao: 读取图片文件失败: " + path);
  }
  file.close();

  // 使用 stb_image 从内存解码图片（强制 4 通道 RGBA）
  int w = 0, h = 0, channels = 0;
  stbi_uc *imgData = stbi_load_from_memory(
      fileData.data(), static_cast<int>(fileData.size()), &w, &h, &channels, 4);

  if (!imgData) {
    throw jsi::JSError(
        rt, std::string("royrao: stb_image 解码失败: ") + stbi_failure_reason());
  }

  if (w <= 0 || h <= 0) {
    stbi_image_free(imgData);
    throw jsi::JSError(rt, "royrao: 图片尺寸无效");
  }

  int pixelCount = w * h;
  size_t byteSize = static_cast<size_t>(pixelCount) * sizeof(uint32_t);

  // RGBA → ARGB 转换（就地操作）
  // stb_image 返回 R,G,B,A 字节序；我们需要 ARGB（Android Bitmap 格式）
  // 内存布局：[R, G, B, A] → 打包为 uint32_t: (A << 24) | (R << 16) | (G <<
  // 8) | B
  auto *rgba = imgData;
  std::vector<uint32_t> argbPixels(pixelCount);
  for (int i = 0; i < pixelCount; ++i) {
    int idx = i * 4;
    uint8_t r = rgba[idx + 0];
    uint8_t g = rgba[idx + 1];
    uint8_t b = rgba[idx + 2];
    uint8_t a = rgba[idx + 3];
    argbPixels[i] =
        (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) |
        (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
  }
  stbi_image_free(imgData);

  // 创建 JS ArrayBuffer 并拷贝像素数据
  jsi::Function abCtor = rt.global().getPropertyAsFunction(rt, "ArrayBuffer");
  jsi::Object abObj =
      abCtor.callAsConstructor(rt, static_cast<double>(byteSize)).asObject(rt);
  jsi::ArrayBuffer ab = abObj.getArrayBuffer(rt);
  std::memcpy(ab.data(rt), argbPixels.data(), byteSize);

  jsi::Object result(rt);
  result.setProperty(rt, "pixels", std::move(abObj));
  result.setProperty(rt, "width", static_cast<double>(w));
  result.setProperty(rt, "height", static_cast<double>(h));
  return result;
}

jsi::String NativeEncryptionCore::pixelsToBase64Png(jsi::Runtime &rt,
                                                    jsi::Object pixelBuffer,
                                                    double width,
                                                    double height) {
  int w = static_cast<int>(width);
  int h = static_cast<int>(height);
  int pixelCount = w * h;

  if (w <= 0 || h <= 0) {
    throw jsi::JSError(rt, "royrao: width 和 height 必须为正整数");
  }

  // 提取 ARGB 像素
  std::vector<uint32_t> argbPixels = extractPixels(rt, pixelBuffer, pixelCount);

  // ARGB → RGBA 转换（stb_image_write 需要 RGBA 字节序）
  std::vector<uint8_t> rgbaData(static_cast<size_t>(pixelCount) * 4);
  for (int i = 0; i < pixelCount; ++i) {
    uint32_t px = argbPixels[i];
    rgbaData[i * 4 + 0] = static_cast<uint8_t>((px >> 16) & 0xFF); // R
    rgbaData[i * 4 + 1] = static_cast<uint8_t>((px >> 8) & 0xFF);  // G
    rgbaData[i * 4 + 2] = static_cast<uint8_t>(px & 0xFF);         // B
    rgbaData[i * 4 + 3] = static_cast<uint8_t>((px >> 24) & 0xFF); // A
  }

  // 使用 stb_image_write 将 RGBA 编码为 PNG（写入内存缓冲区）
  std::vector<uint8_t> pngBuffer;
  pngBuffer.reserve(static_cast<size_t>(pixelCount)); // 预估大小

  int stride = w * 4;
  int success = stbi_write_png_to_func(stbiWriteCallback, &pngBuffer, w, h, 4,
                                       rgbaData.data(), stride);

  if (!success || pngBuffer.empty()) {
    throw jsi::JSError(rt, "royrao: PNG 编码失败");
  }

  // Base64 编码
  std::string base64 = base64Encode(pngBuffer.data(), pngBuffer.size());

  return jsi::String::createFromUtf8(rt, base64);
}

} // namespace facebook::react


