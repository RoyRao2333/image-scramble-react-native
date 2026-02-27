import { useThemeColors } from '@/utils/theme';
import React, { memo, useMemo } from 'react';
import { Image, StyleSheet, Text, View } from 'react-native';

interface ImagePreviewProps {
  imageUri: string | null;
}

export const ImagePreview = memo(({ imageUri }: ImagePreviewProps) => {
  const colors = useThemeColors();

  const styles = useMemo(
    () =>
      StyleSheet.create({
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
      }),
    [colors],
  );

  return (
    <View style={styles.imageContainer}>
      {imageUri ? (
        <Image
          source={{ uri: imageUri }}
          style={styles.image}
          resizeMode="contain"
        />
      ) : (
        <View style={styles.imagePlaceholder}>
          <Text style={styles.imagePlaceholderIcon}>🖼️</Text>
          <Text style={styles.imagePlaceholderText}>尚未选择图片</Text>
        </View>
      )}
    </View>
  );
});

ImagePreview.displayName = 'ImagePreview';
