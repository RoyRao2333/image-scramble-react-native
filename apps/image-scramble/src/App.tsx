import {
  type Algorithm,
  type ScrambleResult,
  decrypt,
  encrypt,
  pixelsToBase64Png,
  readImagePixels,
} from '@package/encryption-core';
import { useCallback, useMemo, useRef, useState } from 'react';
import {
  Alert,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  View,
  useColorScheme,
} from 'react-native';
import ImageCropPicker from 'react-native-image-crop-picker';
import { SafeAreaProvider, SafeAreaView } from 'react-native-safe-area-context';

import { ActionButtons } from '@/components/ActionButtons';
import { AlgorithmSelector } from '@/components/AlgorithmSelector';
import { ImagePreview } from '@/components/ImagePreview';
import { KeyInput } from '@/components/KeyInput';
import { LoadingOverlay } from '@/components/LoadingOverlay';
import {
  getDefaultKey,
  isDoubleKeyAlgorithm,
  validateKey,
} from '@/utils/algorithm';
import { useThemeColors } from '@/utils/theme';

import './styles/global.css';

export const App = () => {
  const isDarkMode = useColorScheme() === 'dark';
  const colors = useThemeColors();

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
  const handleAlgorithmSelect = useCallback((algo: Algorithm) => {
    setSelectedAlgorithm((prevAlgo) => {
      // 切换算法类型时自动切换默认 key
      if (isDoubleKeyAlgorithm(prevAlgo) !== isDoubleKeyAlgorithm(algo)) {
        setKeyString(getDefaultKey(algo));
      } else if (prevAlgo === 'tomato' && algo !== 'tomato') {
        setKeyString(getDefaultKey(algo));
      } else if (prevAlgo !== 'tomato' && algo === 'tomato') {
        setKeyString(getDefaultKey(algo));
      }
      return algo;
    });
    setShowAlgorithmPicker(false);
  }, []);

  const openAlgorithmPicker = useCallback(
    () => setShowAlgorithmPicker(true),
    [],
  );
  const closeAlgorithmPicker = useCallback(
    () => setShowAlgorithmPicker(false),
    [],
  );

  // ------ 选择图片 ------
  const handlePickImage = useCallback(async () => {
    try {
      const result = await ImageCropPicker.openPicker({
        mediaType: 'photo',
        // 不裁剪，保持原始图片
        cropping: false,
      });

      if (!result.path) return;

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
      if (error?.code === 'E_PICKER_CANCELLED') return;
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

  const styles = useMemo(
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
        controlsContainer: {
          marginTop: 20,
          marginHorizontal: 20,
          gap: 12,
        },
      }),
    [colors],
  );

  return (
    <SafeAreaProvider>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />

      <SafeAreaView style={styles.container}>
        <ScrollView
          contentContainerStyle={{ paddingBottom: 40 }}
          keyboardShouldPersistTaps="handled"
        >
          {/* Header */}
          <View style={styles.header}>
            <Text style={styles.headerTitle}>图片混淆</Text>
            <Text style={styles.headerSubtitle}>
              选择图片，选择算法，一键混淆或还原
            </Text>
          </View>

          {/* 图片预览 */}
          <ImagePreview imageUri={imageUri} />

          {/* 控制区域 */}
          <View style={styles.controlsContainer}>
            {/* 算法选择器 */}
            <AlgorithmSelector
              selectedAlgorithm={selectedAlgorithm}
              onSelect={handleAlgorithmSelect}
              visible={showAlgorithmPicker}
              onOpen={openAlgorithmPicker}
              onClose={closeAlgorithmPicker}
            />

            {/* Key 输入 */}
            <KeyInput
              selectedAlgorithm={selectedAlgorithm}
              keyString={keyString}
              onChangeText={setKeyString}
            />

            {/* 操作按钮 */}
            <ActionButtons
              isLoading={isLoading}
              hasImage={!!imageUri}
              onPickImage={handlePickImage}
              onEncrypt={handleEncrypt}
              onDecrypt={handleDecrypt}
            />
          </View>
        </ScrollView>

        {/* Loading overlay */}
        <LoadingOverlay visible={isLoading} text={loadingText} />
      </SafeAreaView>
    </SafeAreaProvider>
  );
};

export default App;
