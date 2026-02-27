import { type Algorithm } from '@package/encryption-core';

export const ALGORITHM_LABELS: Record<Algorithm, string> = {
  tomato: 'Tomato',
  block: 'Block',
  perPixel: 'Per Pixel',
  rowPixel: 'Row Pixel',
  picEncryptRow: 'PicEncrypt Row',
  picEncryptRowColumn: 'PicEncrypt Row+Column',
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
