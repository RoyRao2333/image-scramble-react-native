import NativeEncryptionCore from './NativeEncryptionCore';

export type Algorithm =
  | 'tomato'
  | 'block'
  | 'perPixel'
  | 'rowPixel'
  | 'picEncryptRow'
  | 'picEncryptRowColumn';

/** 所有支持的算法列表，供 UI 下拉框使用 */
export const ALGORITHMS: readonly Algorithm[] = [
  'tomato',
  'block',
  'perPixel',
  'rowPixel',
  'picEncryptRow',
  'picEncryptRowColumn',
] as const;

export interface ScrambleResult {
  pixels: ArrayBuffer;
  width: number;
  height: number;
}

/**
 * 加密图片像素
 * @param pixelBuffer - 原始像素 ArrayBuffer（Uint32Array，ARGB 格式）
 * @param width - 图片宽度
 * @param height - 图片高度
 * @param algorithm - 混淆算法名称
 * @param key - 密钥（double 类密钥传数字字符串，string 类密钥直接传字符串）
 */
export function encrypt(
  pixelBuffer: ArrayBuffer,
  width: number,
  height: number,
  algorithm: Algorithm,
  key: string,
): ScrambleResult {
  return NativeEncryptionCore.encrypt(
    pixelBuffer,
    width,
    height,
    algorithm,
    key,
  ) as ScrambleResult;
}

/**
 * 解密图片像素
 * @param pixelBuffer - 加密后的像素 ArrayBuffer
 * @param width - 图片宽度（加密后可能与原图不同，如 BlockScramble）
 * @param height - 图片高度
 * @param algorithm - 混淆算法名称（必须与加密时一致）
 * @param key - 密钥（必须与加密时一致）
 */
export function decrypt(
  pixelBuffer: ArrayBuffer,
  width: number,
  height: number,
  algorithm: Algorithm,
  key: string,
): ScrambleResult {
  return NativeEncryptionCore.decrypt(
    pixelBuffer,
    width,
    height,
    algorithm,
    key,
  ) as ScrambleResult;
}

/**
 * 从文件路径读取图片并解码为 ARGB 像素数据
 * @param filePath - 图片文件的绝对路径
 */
export function readImagePixels(filePath: string): ScrambleResult {
  return NativeEncryptionCore.readImagePixels(filePath) as ScrambleResult;
}

/**
 * 将 ARGB 像素数据编码为 base64 PNG 字符串
 * @param pixelBuffer - ARGB 像素 ArrayBuffer
 * @param width - 图片宽度
 * @param height - 图片高度
 * @returns base64 编码的 PNG 字符串（不含 data:image/png;base64, 前缀）
 */
export function pixelsToBase64Png(
  pixelBuffer: ArrayBuffer,
  width: number,
  height: number,
): string {
  return NativeEncryptionCore.pixelsToBase64Png(pixelBuffer, width, height);
}

export default NativeEncryptionCore;
