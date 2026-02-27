import { ALGORITHM_LABELS } from '@/utils/algorithm';
import { useThemeColors } from '@/utils/theme';
import { type Algorithm, ALGORITHMS } from '@package/encryption-core';
import React, { memo, useMemo } from 'react';
import { Modal, StyleSheet, Text, TouchableOpacity, View } from 'react-native';

interface AlgorithmSelectorProps {
  selectedAlgorithm: Algorithm;
  onSelect: (algo: Algorithm) => void;
  visible: boolean;
  onOpen: () => void;
  onClose: () => void;
}

export const AlgorithmSelector = memo(
  ({
    selectedAlgorithm,
    onSelect,
    visible,
    onOpen,
    onClose,
  }: AlgorithmSelectorProps) => {
    const colors = useThemeColors();

    const styles = useMemo(
      () =>
        StyleSheet.create({
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
      <>
        <TouchableOpacity
          activeOpacity={0.7}
          onPress={onOpen}
          style={styles.selectorButton}
        >
          <View>
            <Text style={styles.selectorLabel}>混淆算法</Text>
            <Text style={styles.selectorValue}>
              {ALGORITHM_LABELS[selectedAlgorithm]}
            </Text>
          </View>
          <Text style={styles.selectorArrow}>▼</Text>
        </TouchableOpacity>

        <Modal
          visible={visible}
          transparent
          animationType="slide"
          onRequestClose={onClose}
        >
          <TouchableOpacity
            activeOpacity={1}
            style={styles.modalOverlay}
            onPress={onClose}
          >
            <TouchableOpacity activeOpacity={1}>
              <View style={styles.modalContent}>
                <View style={styles.modalHandle} />
                <Text style={styles.modalTitle}>选择混淆算法</Text>
                {ALGORITHMS.map((algo) => {
                  const isSelected = algo === selectedAlgorithm;
                  return (
                    <TouchableOpacity
                      key={algo}
                      activeOpacity={0.6}
                      onPress={() => onSelect(algo)}
                      style={[
                        styles.modalItem,
                        isSelected && styles.modalItemSelected,
                      ]}
                    >
                      <Text
                        style={[
                          styles.modalItemText,
                          isSelected && styles.modalItemSelectedText,
                        ]}
                      >
                        {ALGORITHM_LABELS[algo]}
                      </Text>
                      {isSelected && (
                        <Text style={styles.modalItemCheck}>✓</Text>
                      )}
                    </TouchableOpacity>
                  );
                })}
              </View>
            </TouchableOpacity>
          </TouchableOpacity>
        </Modal>
      </>
    );
  },
);

AlgorithmSelector.displayName = 'AlgorithmSelector';
