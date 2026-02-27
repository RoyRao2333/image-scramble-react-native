import { useThemeColors } from '@/utils/theme';
import React, { memo, useMemo } from 'react';
import { ActivityIndicator, StyleSheet, Text, View } from 'react-native';

interface LoadingOverlayProps {
  visible: boolean;
  text: string;
}

export const LoadingOverlay = memo(({ visible, text }: LoadingOverlayProps) => {
  const colors = useThemeColors();

  const styles = useMemo(
    () =>
      StyleSheet.create({
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
      }),
    [colors],
  );

  if (!visible) return null;

  return (
    <View style={styles.loadingOverlay}>
      <View style={styles.loadingCard}>
        <ActivityIndicator size="large" color={colors.accent} />
        <Text style={styles.loadingText}>{text}</Text>
      </View>
    </View>
  );
});

LoadingOverlay.displayName = 'LoadingOverlay';
