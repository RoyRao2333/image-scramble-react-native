import { useMemo } from 'react';
import { useColorScheme } from 'react-native';

export const useThemeColors = () => {
  const isDarkMode = useColorScheme() === 'dark';
  return useMemo(
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
};
