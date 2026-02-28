#!/bin/bash
set -e

# ============================================================
# Build unsigned IPA for GitHub distribution
# Usage: ./scripts/build-ios-unsigned.sh [version]
# Example: ./scripts/build-ios-unsigned.sh 1.0.0
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IOS_DIR="$SCRIPT_DIR/../ios"
BUILD_DIR="$IOS_DIR/build"
WORKSPACE="$IOS_DIR/ImageScramble.xcworkspace"
SCHEME="ImageScramble"
ARCHIVE_PATH="$BUILD_DIR/ImageScramble.xcarchive"
PAYLOAD_DIR="$BUILD_DIR/Payload"

# 版本号（可选参数，默认读取 Info.plist 中的 CFBundleShortVersionString）
VERSION="${1:-}"

echo "royrao: 🚀 Starting unsigned IPA build..."

# ── 清理旧产物 ──────────────────────────────────────────────
echo "royrao: 🗑  Cleaning previous build artifacts..."
rm -rf "$ARCHIVE_PATH" "$PAYLOAD_DIR" "$BUILD_DIR/ImageScramble-unsigned.ipa"

# ── Archive（禁用代码签名）──────────────────────────────────
echo "royrao: 🔨 Archiving with code signing disabled..."

XCODEBUILD_CMD=(xcodebuild archive \
  -workspace "$WORKSPACE" \
  -scheme "$SCHEME" \
  -configuration Release \
  -destination 'generic/platform=iOS' \
  -archivePath "$ARCHIVE_PATH" \
  CODE_SIGNING_ALLOWED=NO \
  CODE_SIGN_IDENTITY="" \
  CODE_SIGNING_REQUIRED=NO \
  AD_HOC_CODE_SIGNING_ALLOWED=YES)

if command -v xcpretty &>/dev/null; then
  "${XCODEBUILD_CMD[@]}" | xcpretty
  BUILD_EXIT=${PIPESTATUS[0]}
else
  "${XCODEBUILD_CMD[@]}"
  BUILD_EXIT=$?
fi

if [ "$BUILD_EXIT" -ne 0 ]; then
  echo "royrao: ❌ xcodebuild archive failed."
  exit 1
fi

APP_PATH="$ARCHIVE_PATH/Products/Applications/ImageScramble.app"
if [ ! -d "$APP_PATH" ]; then
  echo "royrao: ❌ .app bundle not found at: $APP_PATH"
  exit 1
fi

# ── 打包成 IPA ──────────────────────────────────────────────
echo "royrao: 📦 Packaging .app into unsigned IPA..."
mkdir -p "$PAYLOAD_DIR"
cp -r "$APP_PATH" "$PAYLOAD_DIR/"

# 确定输出文件名（含版本号）
if [ -z "$VERSION" ]; then
  INFO_PLIST="$APP_PATH/Info.plist"
  if [ -f "$INFO_PLIST" ]; then
    VERSION=$(/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" "$INFO_PLIST" 2>/dev/null || echo "1.0.0")
  else
    VERSION="1.0.0"
  fi
fi

IPA_NAME="ImageScramble-v${VERSION}-unsigned.ipa"
IPA_PATH="$BUILD_DIR/$IPA_NAME"

(cd "$BUILD_DIR" && zip -r "$IPA_NAME" Payload -x "*.DS_Store")

echo ""
echo "royrao: ✅ Done! Unsigned IPA generated:"
echo "  → $IPA_PATH"
echo ""
echo "royrao: ⚠️  Users must re-sign this IPA before installation (AltStore / Sideloadly / TrollStore)."
