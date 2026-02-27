import { isDoubleKeyAlgorithm } from '@/utils/algorithm';
import { useThemeColors } from '@/utils/theme';
import { type Algorithm } from '@package/encryption-core';
import React, { memo, useMemo } from 'react';
import { StyleSheet, Text, TextInput, View } from 'react-native';

interface KeyInputProps {
  selectedAlgorithm: Algorithm;
  keyString: string;
  onChangeText: (text: string) => void;
}

export const KeyInput = memo(
  ({ selectedAlgorithm, keyString, onChangeText }: KeyInputProps) => {
    const colors = useThemeColors();

    const styles = useMemo(
      () =>
        StyleSheet.create({
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
        }),
      [colors],
    );

    return (
      <View style={styles.keyInputContainer}>
        <Text style={styles.keyInputLabel}>密钥 (Key)</Text>
        <TextInput
          style={styles.keyInput}
          value={keyString}
          onChangeText={onChangeText}
          placeholder="输入密钥..."
          placeholderTextColor={colors.placeholder}
          keyboardType={
            isDoubleKeyAlgorithm(selectedAlgorithm) ? 'decimal-pad' : 'default'
          }
          returnKeyType="done"
          autoCorrect={false}
        />
      </View>
    );
  },
);

KeyInput.displayName = 'KeyInput';
