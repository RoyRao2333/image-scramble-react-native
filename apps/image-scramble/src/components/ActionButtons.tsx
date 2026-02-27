import { useThemeColors } from '@/utils/theme';
import React, { memo, useMemo } from 'react';
import { StyleSheet, Text, TouchableOpacity, View } from 'react-native';

interface ActionButtonsProps {
  isLoading: boolean;
  hasImage: boolean;
  onPickImage: () => void;
  onEncrypt: () => void;
  onDecrypt: () => void;
}

export const ActionButtons = memo(
  ({
    isLoading,
    hasImage,
    onPickImage,
    onEncrypt,
    onDecrypt,
  }: ActionButtonsProps) => {
    const colors = useThemeColors();

    const styles = useMemo(
      () =>
        StyleSheet.create({
          pickButton: {
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
        }),
      [colors],
    );

    return (
      <>
        <TouchableOpacity
          activeOpacity={0.7}
          onPress={onPickImage}
          style={styles.pickButton}
          disabled={isLoading}
        >
          <Text style={styles.pickButtonText}>📷 选择图片</Text>
        </TouchableOpacity>

        <View style={styles.actionButtonRow}>
          <TouchableOpacity
            activeOpacity={0.7}
            onPress={onEncrypt}
            style={[
              styles.encryptButton,
              (!hasImage || isLoading) && styles.disabledButton,
            ]}
            disabled={!hasImage || isLoading}
          >
            <Text style={styles.encryptButtonText}>🔒 混淆</Text>
          </TouchableOpacity>

          <TouchableOpacity
            activeOpacity={0.7}
            onPress={onDecrypt}
            style={[
              styles.decryptButton,
              (!hasImage || isLoading) && styles.disabledButton,
            ]}
            disabled={!hasImage || isLoading}
          >
            <Text style={styles.decryptButtonText}>🔓 解除混淆</Text>
          </TouchableOpacity>
        </View>
      </>
    );
  },
);

ActionButtons.displayName = 'ActionButtons';
