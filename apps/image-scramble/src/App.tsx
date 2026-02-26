import { useCallback, useMemo, useRef, useState } from 'react';
import {
  ActivityIndicator,
  Alert,
  Image,
  Modal,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  useColorScheme,
  View,
} from 'react-native';
import { SafeAreaProvider, SafeAreaView } from 'react-native-safe-area-context';
import ImageCropPicker from 'react-native-image-crop-picker';
import {
  type Algorithm,
  ALGORITHMS,
  type ScrambleResult,
  encrypt,
  decrypt,
  readImagePixels,
  pixelsToBase64Png,
} from '@package/encryption-core';

import './styles/global.css';

// ============================================================================
// 常量
// ============================================================================

/** 算法展示名称映射 */
const ALGORITHM_LABELS: Record<Algorithm, string> = {
  tomato: 'Tomato',
  block: 'Block',
  perPixel: 'Per Pixel',
  rowPixel: 'Row Pixel',
  picEncryptRow: 'PicEncrypt Row',
  picEncryptRowColumn: 'PicEncrypt Row+Column',
};

/** 判断是否为需要 Double 类型 key 的算法 */
const isDoubleKeyAlgorithm = (algo: Algorithm): boolean =>
  algo === 'tomato' ||
  algo === 'picEncryptRow' ||
  algo === 'picEncryptRowColumn';

/** 获取算法默认 key */
const getDefaultKey = (algo: Algorithm): string =>
  algo === 'tomato' ? '1' : isDoubleKeyAlgorithm(algo) ? '0.666' : '0.666';

/** 校验 key 合法性 */
const validateKey = (algo: Algorithm, key: string): string | null => {
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
  // block / perPixel / rowPixel 接受任意字符串
  return null;
};

// ============================================================================
// 组件
// ============================================================================

export const App = () => {
  const isDarkMode = useColorScheme() === 'dark';
  const colors = useMemo(
    () => ({
      bg: isDarkMode ? '#0F0F14' : '#F5F5FA',
      card: isDarkMode ? '#1A1A24' : '#FFFFFF',
      text: isDarkMode ? '#E8E8F0' : '#1A1A2E',
      textSecondary: isDarkMode ? '#8888A0' : '#6B6B80',
      accent: '#6C5CE7',
      accentLight: isDarkMode ? '#6C5CE730' : '#6C5CE715',
      danger: '#E74C3C',
      dangerLight: isDarkMode ? '#E74C3C30' : '#E74C3C15',
      success: '#27AE60',
      successLight: isDarkMode ? '#27AE6030' : '#27AE6015',
      border: isDarkMode ? '#2A2A3A' : '#E0E0E8',
      inputBg: isDarkMode ? '#12121A' : '#F0F0F5',
      overlay: 'rgba(0,0,0,0.5)',
      placeholder: isDarkMode ? '#555570' : '#A0A0B0',
    }),
    [isDarkMode],
  );

  // State
  const [selectedAlgorithm, setSelectedAlgorithm] =
    useState<Algorithm>('tomato');
  const [keyString, setKeyString] = useState('1');
  const [imageUri, setImageUri] = useState<string | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [loadingText, setLoadingText] = useState('');
  const [showAlgorithmPicker, setShowAlgorithmPicker] = useState(false);

  // 保存原始像素数据（用于多次操作）
  const originalPixelsRef = useRef<ScrambleResult | null>(null);
  // 保存当前像素数据（加密后供解密用）
  const currentPixelsRef = useRef<ScrambleResult | null>(null);

  // ------ 选择算法 ------
  const handleAlgorithmSelect = useCallback(
    (algo: Algorithm) => {
      const previousAlgo = selectedAlgorithm;
      setSelectedAlgorithm(algo);
      setShowAlgorithmPicker(false);

      // 切换算法类型时自动切换默认 key
      if (isDoubleKeyAlgorithm(previousAlgo) !== isDoubleKeyAlgorithm(algo)) {
        setKeyString(getDefaultKey(algo));
      } else if (previousAlgo === 'tomato' && algo !== 'tomato') {
        setKeyString(getDefaultKey(algo));
      } else if (previousAlgo !== 'tomato' && algo === 'tomato') {
        setKeyString(getDefaultKey(algo));
      }
    },
    [selectedAlgorithm],
  );

  // ------ 选择图片 ------
  const handlePickImage = useCallback(async () => {
    try {
      const result = await ImageCropPicker.openPicker({
        mediaType: 'photo',
        // 不裁剪，保持原始图片
        cropping: false,
      });

      if (!result.path) {
        return;
      }

      setIsLoading(true);
      setLoadingText('读取图片...');

      // 读取像素数据（同步 JSI 调用，但在 setState 后执行以展示 loading）
      setTimeout(() => {
        try {
          const pixelData = readImagePixels(result.path);
          originalPixelsRef.current = pixelData;
          currentPixelsRef.current = pixelData;

          // 将像素转为 base64 展示
          const base64 = pixelsToBase64Png(
            pixelData.pixels,
            pixelData.width,
            pixelData.height,
          );
          setImageUri(`data:image/png;base64,${base64}`);
        } catch (error) {
          console.error('royrao: 读取图片像素失败', error);
          Alert.alert('错误', `读取图片失败: ${error}`);
        } finally {
          setIsLoading(false);
          setLoadingText('');
        }
      }, 50);
    } catch (error: any) {
      // 用户取消选择
      if (error?.code === 'E_PICKER_CANCELLED') {
        return;
      }
      console.error('royrao: 选择图片失败', error);
      Alert.alert('错误', `选择图片失败: ${error}`);
    }
  }, []);

  // ------ 混淆 ------
  const handleEncrypt = useCallback(() => {
    if (!originalPixelsRef.current) {
      Alert.alert('提示', '请先选择一张图片');
      return;
    }

    const validationError = validateKey(selectedAlgorithm, keyString);
    if (validationError) {
      Alert.alert('Key 无效', validationError);
      return;
    }

    setIsLoading(true);
    setLoadingText('混淆中...');

    setTimeout(() => {
      try {
        const source = originalPixelsRef.current!;
        const result = encrypt(
          source.pixels,
          source.width,
          source.height,
          selectedAlgorithm,
          keyString,
        );

        currentPixelsRef.current = result;

        const base64 = pixelsToBase64Png(
          result.pixels,
          result.width,
          result.height,
        );
        setImageUri(`data:image/png;base64,${base64}`);
      } catch (error) {
        console.error('royrao: 混淆失败', error);
        Alert.alert('错误', `混淆失败: ${error}`);
      } finally {
        setIsLoading(false);
        setLoadingText('');
      }
    }, 50);
  }, [selectedAlgorithm, keyString]);

  // ------ 解除混淆 ------
  const handleDecrypt = useCallback(() => {
    if (!currentPixelsRef.current) {
      Alert.alert('提示', '请先选择一张图片');
      return;
    }

    const validationError = validateKey(selectedAlgorithm, keyString);
    if (validationError) {
      Alert.alert('Key 无效', validationError);
      return;
    }

    setIsLoading(true);
    setLoadingText('解除混淆中...');

    setTimeout(() => {
      try {
        const source = currentPixelsRef.current!;
        const result = decrypt(
          source.pixels,
          source.width,
          source.height,
          selectedAlgorithm,
          keyString,
        );

        currentPixelsRef.current = result;

        const base64 = pixelsToBase64Png(
          result.pixels,
          result.width,
          result.height,
        );
        setImageUri(`data:image/png;base64,${base64}`);
      } catch (error) {
        console.error('royrao: 解除混淆失败', error);
        Alert.alert('错误', `解除混淆失败: ${error}`);
      } finally {
        setIsLoading(false);
        setLoadingText('');
      }
    }, 50);
  }, [selectedAlgorithm, keyString]);

  // ------ 计算动态样式 ------
  const dynamicStyles = useMemo(
    () =>
      StyleSheet.create({
        container: {
          flex: 1,
          backgroundColor: colors.bg,
        },
        header: {
          paddingHorizontal: 20,
          paddingTop: 8,
          paddingBottom: 16,
        },
        headerTitle: {
          fontSize: 28,
          fontWeight: '800',
          color: colors.text,
          letterSpacing: -0.5,
        },
        headerSubtitle: {
          fontSize: 14,
          color: colors.textSecondary,
          marginTop: 4,
        },
        // 图片预览区域
        imageContainer: {
          marginHorizontal: 20,
          borderRadius: 16,
          backgroundColor: colors.card,
          borderWidth: 1,
          borderColor: colors.border,
          overflow: 'hidden',
          minHeight: 200,
          justifyContent: 'center',
          alignItems: 'center',
        },
        imagePlaceholder: {
          padding: 40,
          alignItems: 'center',
        },
        imagePlaceholderIcon: {
          fontSize: 48,
          marginBottom: 12,
        },
        imagePlaceholderText: {
          fontSize: 15,
          color: colors.textSecondary,
        },
        image: {
          width: '100%',
          aspectRatio: 1,
        },
        // 控制区域
        controlsContainer: {
          marginTop: 20,
          marginHorizontal: 20,
          gap: 12,
        },
        // 算法选择器
        selectorButton: {
          flexDirection: 'row',
          alignItems: 'center',
          justifyContent: 'space-between',
          backgroundColor: colors.card,
          borderRadius: 12,
          borderWidth: 1,
          borderColor: colors.border,
          paddingHorizontal: 16,
          paddingVertical: 14,
        },
        selectorLabel: {
          fontSize: 13,
          color: colors.textSecondary,
          marginBottom: 2,
        },
        selectorValue: {
          fontSize: 16,
          fontWeight: '600',
          color: colors.text,
        },
        selectorArrow: {
          fontSize: 12,
          color: colors.textSecondary,
        },
        // Key 输入
        keyInputContainer: {
          backgroundColor: colors.card,
          borderRadius: 12,
          borderWidth: 1,
          borderColor: colors.border,
          paddingHorizontal: 16,
          paddingTop: 10,
          paddingBottom: 4,
        },
        keyInputLabel: {
          fontSize: 13,
          color: colors.textSecondary,
        },
        keyInput: {
          fontSize: 16,
          fontWeight: '600',
          color: colors.text,
          paddingVertical: 8,
          paddingHorizontal: 0,
        },
        // 按钮行
        buttonRow: {
          flexDirection: 'row',
          gap: 10,
        },
        // 选择图片按钮
        pickButton: {
          flex: 1,
          backgroundColor: colors.accentLight,
          borderRadius: 12,
          paddingVertical: 14,
          alignItems: 'center',
          justifyContent: 'center',
        },
        pickButtonText: {
          fontSize: 15,
          fontWeight: '700',
          color: colors.accent,
        },
        // 操作按钮
        actionButtonRow: {
          flexDirection: 'row',
          gap: 10,
        },
        encryptButton: {
          flex: 1,
          backgroundColor: colors.accent,
          borderRadius: 12,
          paddingVertical: 14,
          alignItems: 'center',
          justifyContent: 'center',
        },
        encryptButtonText: {
          fontSize: 15,
          fontWeight: '700',
          color: '#FFFFFF',
        },
        decryptButton: {
          flex: 1,
          backgroundColor: colors.successLight,
          borderRadius: 12,
          paddingVertical: 14,
          alignItems: 'center',
          justifyContent: 'center',
        },
        decryptButtonText: {
          fontSize: 15,
          fontWeight: '700',
          color: colors.success,
        },
        disabledButton: {
          opacity: 0.4,
        },
        // Loading overlay
        loadingOverlay: {
          ...StyleSheet.absoluteFillObject,
          backgroundColor: colors.overlay,
          justifyContent: 'center',
          alignItems: 'center',
          zIndex: 999,
        },
        loadingCard: {
          backgroundColor: colors.card,
          borderRadius: 16,
          paddingHorizontal: 32,
          paddingVertical: 24,
          alignItems: 'center',
          gap: 12,
        },
        loadingText: {
          fontSize: 15,
          fontWeight: '600',
          color: colors.text,
        },
        // Modal 下拉列表
        modalOverlay: {
          flex: 1,
          backgroundColor: colors.overlay,
          justifyContent: 'flex-end',
        },
        modalContent: {
          backgroundColor: colors.card,
          borderTopLeftRadius: 20,
          borderTopRightRadius: 20,
          paddingTop: 12,
          paddingBottom: 40,
        },
        modalHandle: {
          width: 36,
          height: 4,
          borderRadius: 2,
          backgroundColor: colors.border,
          alignSelf: 'center',
          marginBottom: 16,
        },
        modalTitle: {
          fontSize: 18,
          fontWeight: '700',
          color: colors.text,
          paddingHorizontal: 20,
          marginBottom: 8,
        },
        modalItem: {
          paddingHorizontal: 20,
          paddingVertical: 14,
          flexDirection: 'row',
          alignItems: 'center',
          justifyContent: 'space-between',
        },
        modalItemText: {
          fontSize: 16,
          color: colors.text,
        },
        modalItemSelected: {
          backgroundColor: colors.accentLight,
        },
        modalItemSelectedText: {
          color: colors.accent,
          fontWeight: '700',
        },
        modalItemCheck: {
          fontSize: 16,
          color: colors.accent,
          fontWeight: '700',
        },
      }),
    [colors],
  );

  return (
    <SafeAreaProvider>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />

      <SafeAreaView style={dynamicStyles.container}>
        <ScrollView
          contentContainerStyle={{ paddingBottom: 40 }}
          keyboardShouldPersistTaps="handled"
        >
          {/* Header */}
          <View style={dynamicStyles.header}>
            <Text style={dynamicStyles.headerTitle}>图片混淆</Text>
            <Text style={dynamicStyles.headerSubtitle}>
              选择图片，选择算法，一键混淆或还原
            </Text>
          </View>

          {/* 图片预览 */}
          <View style={dynamicStyles.imageContainer}>
            {imageUri ? (
              <Image
                source={{ uri: imageUri }}
                style={dynamicStyles.image}
                resizeMode="contain"
              />
            ) : (
              <View style={dynamicStyles.imagePlaceholder}>
                <Text style={dynamicStyles.imagePlaceholderIcon}>🖼️</Text>
                <Text style={dynamicStyles.imagePlaceholderText}>
                  尚未选择图片
                </Text>
              </View>
            )}
          </View>

          {/* 控制区域 */}
          <View style={dynamicStyles.controlsContainer}>
            {/* 算法选择器 */}
            <TouchableOpacity
              activeOpacity={0.7}
              onPress={() => setShowAlgorithmPicker(true)}
              style={dynamicStyles.selectorButton}
            >
              <View>
                <Text style={dynamicStyles.selectorLabel}>混淆算法</Text>
                <Text style={dynamicStyles.selectorValue}>
                  {ALGORITHM_LABELS[selectedAlgorithm]}
                </Text>
              </View>
              <Text style={dynamicStyles.selectorArrow}>▼</Text>
            </TouchableOpacity>

            {/* Key 输入 */}
            <View style={dynamicStyles.keyInputContainer}>
              <Text style={dynamicStyles.keyInputLabel}>密钥 (Key)</Text>
              <TextInput
                style={dynamicStyles.keyInput}
                value={keyString}
                onChangeText={setKeyString}
                placeholder="输入密钥..."
                placeholderTextColor={colors.placeholder}
                keyboardType={
                  isDoubleKeyAlgorithm(selectedAlgorithm)
                    ? 'decimal-pad'
                    : 'default'
                }
                returnKeyType="done"
                autoCorrect={false}
              />
            </View>

            {/* 选择图片按钮 */}
            <TouchableOpacity
              activeOpacity={0.7}
              onPress={handlePickImage}
              style={dynamicStyles.pickButton}
              disabled={isLoading}
            >
              <Text style={dynamicStyles.pickButtonText}>📷 选择图片</Text>
            </TouchableOpacity>

            {/* 混淆 / 解除混淆按钮 */}
            <View style={dynamicStyles.actionButtonRow}>
              <TouchableOpacity
                activeOpacity={0.7}
                onPress={handleEncrypt}
                style={[
                  dynamicStyles.encryptButton,
                  (!imageUri || isLoading) && dynamicStyles.disabledButton,
                ]}
                disabled={!imageUri || isLoading}
              >
                <Text style={dynamicStyles.encryptButtonText}>🔒 混淆</Text>
              </TouchableOpacity>

              <TouchableOpacity
                activeOpacity={0.7}
                onPress={handleDecrypt}
                style={[
                  dynamicStyles.decryptButton,
                  (!imageUri || isLoading) && dynamicStyles.disabledButton,
                ]}
                disabled={!imageUri || isLoading}
              >
                <Text style={dynamicStyles.decryptButtonText}>🔓 解除混淆</Text>
              </TouchableOpacity>
            </View>
          </View>
        </ScrollView>

        {/* Loading overlay */}
        {isLoading && (
          <View style={dynamicStyles.loadingOverlay}>
            <View style={dynamicStyles.loadingCard}>
              <ActivityIndicator size="large" color={colors.accent} />
              <Text style={dynamicStyles.loadingText}>{loadingText}</Text>
            </View>
          </View>
        )}

        {/* 算法选择 Modal */}
        <Modal
          visible={showAlgorithmPicker}
          transparent
          animationType="slide"
          onRequestClose={() => setShowAlgorithmPicker(false)}
        >
          <TouchableOpacity
            activeOpacity={1}
            style={dynamicStyles.modalOverlay}
            onPress={() => setShowAlgorithmPicker(false)}
          >
            <TouchableOpacity activeOpacity={1}>
              <View style={dynamicStyles.modalContent}>
                <View style={dynamicStyles.modalHandle} />
                <Text style={dynamicStyles.modalTitle}>选择混淆算法</Text>
                {ALGORITHMS.map((algo) => {
                  const isSelected = algo === selectedAlgorithm;
                  return (
                    <TouchableOpacity
                      key={algo}
                      activeOpacity={0.6}
                      onPress={() => handleAlgorithmSelect(algo)}
                      style={[
                        dynamicStyles.modalItem,
                        isSelected && dynamicStyles.modalItemSelected,
                      ]}
                    >
                      <Text
                        style={[
                          dynamicStyles.modalItemText,
                          isSelected && dynamicStyles.modalItemSelectedText,
                        ]}
                      >
                        {ALGORITHM_LABELS[algo]}
                      </Text>
                      {isSelected && (
                        <Text style={dynamicStyles.modalItemCheck}>✓</Text>
                      )}
                    </TouchableOpacity>
                  );
                })}
              </View>
            </TouchableOpacity>
          </TouchableOpacity>
        </Modal>
      </SafeAreaView>
    </SafeAreaProvider>
  );
};
