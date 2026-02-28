package com.encryptioncore

import android.util.Log
import com.encryptioncore.specs.NativeEncryptionCoreSpec
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReadableMap
import com.facebook.react.bridge.WritableMap

class EncryptionCoreModule(reactContext: ReactApplicationContext) : NativeEncryptionCoreSpec(reactContext) {

    companion object {
        init {
            try {
                System.loadLibrary("EncryptionCore")
            } catch (e: Exception) {
                Log.e("EncryptionCore", "royrao: Failed to load library", e)
            }
        }
    }

    override fun initialize() {
        super.initialize()
        try {
            val contextHolder = reactApplicationContext.javaScriptContextHolder
            if (contextHolder != null) {
                val ptr = contextHolder.get()
                Log.i("EncryptionCore", "royrao: JSI Runtime pointer is $ptr")
                nativeInstallJSI(ptr)
            } else {
                Log.e("EncryptionCore", "royrao: JSI Runtime is null, unable to install JSI bindings")
            }
        } catch (e: Exception) {
            Log.e("EncryptionCore", "royrao: Failed to initialize JSI bindings", e)
        }
    }

    private external fun nativeInstallJSI(jsiRuntimePointer: Long)

    override fun getName(): String {
        return "NativeEncryptionCore"
    }

    // 这些方法纯由 C++ TurboModule 自身在底层覆盖和挂载，
    // Kotlin 层面只是提供给外部作为 TurboModuleRegistry 以及 CLI 的句柄使用，
    // 我们必须实现 Codegen 所规定的虚方法但保持空跑，不会真的被 JS 调用（因为 C++ 直接挂载）。
    override fun readImagePixels(filePath: String?): WritableMap? {
        return null
    }

    override fun pixelsToBase64Png(pixelBuffer: ReadableMap?, width: Double, height: Double): String? {
        return ""
    }

    override fun encrypt(
        pixelBuffer: ReadableMap?,
        width: Double,
        height: Double,
        algorithm: String?,
        key: String?
    ): WritableMap? {
        return null
    }

    override fun decrypt(
        pixelBuffer: ReadableMap?,
        width: Double,
        height: Double,
        algorithm: String?,
        key: String?
    ): WritableMap? {
        return null
    }
}
