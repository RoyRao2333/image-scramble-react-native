const path = require('path');
const { withNativeWind } = require('nativewind/metro');
const {
  wrapWithReanimatedMetroConfig,
} = require('react-native-reanimated/metro-config');
const { makeMetroConfig } = require('@rnx-kit/metro-config');
const { findWorkspaceRootSync } = require('@rnx-kit/tools-workspaces');
const MetroSymlinksResolver = require('@rnx-kit/metro-resolver-symlinks');

const projectRoot = __dirname;
const workspaceRoot = findWorkspaceRootSync();

let config = makeMetroConfig({
  projectRoot,
  resolver: {
    resolveRequest: MetroSymlinksResolver(),
    nodeModulesPaths: [
      path.resolve(projectRoot, 'node_modules'),
      path.resolve(workspaceRoot, 'node_modules'),
    ],
    unstable_enablePackageExports: true,
  },
});

config = wrapWithReanimatedMetroConfig(config);

// withNativeWind must be the last
config = withNativeWind(config, { input: './src/styles/global.css' });

module.exports = config;
