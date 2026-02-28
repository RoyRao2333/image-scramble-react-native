import { type Algorithm } from '@package/encryption-core';

export const ALGORITHM_LABELS: Record<Algorithm, string> = {
  tomato: '🍅小番茄图片混淆',
  block: '块混淆',
  perPixel: '逐像素混淆',
  rowPixel: '行像素混淆',
  picEncryptRow: '兼容PicEncrypt: 行模式',
  picEncryptRowColumn: '兼容PicEncrypt: 行+列模式',
};

export const isDoubleKeyAlgorithm = (algo: Algorithm): boolean =>
  algo === 'tomato' ||
  algo === 'picEncryptRow' ||
  algo === 'picEncryptRowColumn';

export const getDefaultKey = (algo: Algorithm): string =>
  algo === 'tomato' ? '1' : isDoubleKeyAlgorithm(algo) ? '0.666' : '0.666';

export const validateKey = (algo: Algorithm, key: string): string | null => {
  if (algo === 'tomato') {
    const num = Number(key);
    if (isNaN(num) || num <= 0 || num > 1.618) {
      return 'Tomato 算法的 Key 必须是 (0, 1.618] 范围内的数字';
    }
  } else if (algo === 'picEncryptRow' || algo === 'picEncryptRowColumn') {
    const num = Number(key);
    if (isNaN(num) || num <= 0 || num >= 1) {
      return 'PicEncrypt 算法的 Key 必须是 (0, 1) 范围内的数字';
    }
  }
  return null;
};
