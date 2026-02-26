require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name         = "EncryptionCore"
  s.version      = package["version"]
  s.summary      = "Pure C++ image encryption/scramble TurboModule for React Native"
  s.homepage     = "https://github.com/user/encryption-core"
  s.license      = "MIT"
  s.authors      = { "author" => "author@example.com" }
  s.platforms    = { :ios => "15.1" }
  s.source       = { :git => "https://github.com/user/encryption-core.git", :tag => "#{s.version}" }

  # C++ core library + TurboModule bridge
  s.source_files = "cpp/**/*.{h,hpp,cpp}", "ios/**/*.{h,mm}"
  s.exclude_files = "cpp/tests/**/*"
  s.header_mappings_dir = "cpp"

  s.pod_target_xcconfig = {
    "DEFINES_MODULE" => "YES",
    "CLANG_CXX_LANGUAGE_STANDARD" => "c++20",
    "HEADER_SEARCH_PATHS" => [
      "\"$(PODS_TARGET_SRCROOT)/cpp\"",
    ].join(" "),
  }

  install_modules_dependencies(s)
end
