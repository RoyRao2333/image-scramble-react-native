#pragma once

#include <EncryptionCoreSpecsJSI.h>
#include <memory>
#include <string>

namespace facebook::react {

class NativeEncryptionCore
    : public NativeEncryptionCoreCxxSpec<NativeEncryptionCore> {
public:
  NativeEncryptionCore(std::shared_ptr<CallInvoker> jsInvoker);

  jsi::Object encrypt(jsi::Runtime &rt, jsi::Object pixelBuffer, double width,
                      double height, jsi::String algorithm, jsi::String key);

  jsi::Object decrypt(jsi::Runtime &rt, jsi::Object pixelBuffer, double width,
                      double height, jsi::String algorithm, jsi::String key);

  jsi::Object readImagePixels(jsi::Runtime &rt, jsi::String filePath);

  jsi::String pixelsToBase64Png(jsi::Runtime &rt, jsi::Object pixelBuffer,
                                double width, double height);
};

} // namespace facebook::react
