package com.encryptioncore

import com.facebook.react.TurboReactPackage
import com.facebook.react.bridge.NativeModule
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.module.model.ReactModuleInfo
import com.facebook.react.module.model.ReactModuleInfoProvider

class EncryptionCorePackage : TurboReactPackage() {
    override fun getModule(name: String, reactContext: ReactApplicationContext): NativeModule? {
        if (name == "NativeEncryptionCore") {
            return EncryptionCoreModule(reactContext)
        }
        return null
    }

    override fun getReactModuleInfoProvider(): ReactModuleInfoProvider {
        return ReactModuleInfoProvider {
            val moduleInfos: MutableMap<String, ReactModuleInfo> = HashMap()
            val isTurboModule = true
            moduleInfos["NativeEncryptionCore"] = ReactModuleInfo(
                "NativeEncryptionCore",
                "com.encryptioncore.EncryptionCoreModule",
                false, // canOverrideExistingModule
                false, // needsEagerInit
                true, // hasConstants
                false, // isCxxModule
                isTurboModule // isTurboModule
            )
            moduleInfos
        }
    }
}
