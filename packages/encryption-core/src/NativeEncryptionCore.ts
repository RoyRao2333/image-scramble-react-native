import { TurboModule, TurboModuleRegistry } from 'react-native';

declare var global: any;

export interface Spec extends TurboModule {
  /**
   * 加密像素数据
   * @param pixelBuffer - ArrayBuffer，Uint32Array 视图，每 4 字节为一个 ARGB 像素
   * @param width - 图片宽度
   * @param height - 图片高度
   * @param algorithm - 算法名: "tomato" | "block" | "perPixel" | "rowPixel" | "picEncryptRow" | "picEncryptRowColumn"
   * @param key - 密钥（string 形式，double 类型密钥传数字字符串如 "4.2"）
   * @returns { pixels: ArrayBuffer, width: number, height: number }
   */
  readonly encrypt: (
    pixelBuffer: Object,
    width: number,
    height: number,
    algorithm: string,
    key: string,
  ) => Object;

  /**
   * 解密像素数据
   * @param pixelBuffer - ArrayBuffer，加密后的像素数据
   * @param width - 图片宽度（加密后可能与原图不同）
   * @param height - 图片高度
   * @param algorithm - 算法名（必须与加密时一致）
   * @param key - 密钥（必须与加密时一致）
   * @returns { pixels: ArrayBuffer, width: number, height: number }
   */
  readonly decrypt: (
    pixelBuffer: Object,
    width: number,
    height: number,
    algorithm: string,
    key: string,
  ) => Object;

  /**
   * 从文件路径读取图片并解码为 ARGB 像素数据
   * @param filePath - 图片文件的绝对路径
   * @returns { pixels: ArrayBuffer, width: number, height: number }
   */
  readonly readImagePixels: (filePath: string) => Object;

  /**
   * 将 ARGB 像素数据编码为 base64 PNG 字符串
   * @param pixelBuffer - ARGB 像素 ArrayBuffer
   * @param width - 图片宽度
   * @param height - 图片高度
   * @returns base64 编码的 PNG 字符串（不含 data:image/png;base64, 前缀）
   */
  readonly pixelsToBase64Png: (
    pixelBuffer: Object,
    width: number,
    height: number,
  ) => string;
}

// 1. 获取（或迫使加载）原始 Native 模块以触发 Kotlin 的 Initialize 方法
const EnforcingModule = TurboModuleRegistry.getEnforcing<Spec>(
  'NativeEncryptionCore',
);

// 2. 将代理对象的查询延后到具体的函数调用时刻
const EncryptionCoreProxy = new Proxy(EnforcingModule, {
  get(target, prop, receiver) {
    const jsiProxy = (global as any).__NativeEncryptionCoreProxy;
    // 如果 JSI C++ 代理成功安装且此方法在其内部存在，则直接截获并走 C++ 原生方法
    if (jsiProxy && prop in jsiProxy) {
      return Reflect.get(jsiProxy, prop, receiver);
    }
    // 否则退回标准的 TurboModule (iOS 依靠此路径运行)
    return Reflect.get(target, prop, receiver);
  },
});

export default EncryptionCoreProxy as Spec;
