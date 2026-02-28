project(EncryptionCore)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(LIB_CPP_DIR "${CMAKE_SOURCE_DIR}/../cpp")

file(GLOB LIB_CPP_SOURCES CONFIGURE_DEPENDS "${LIB_CPP_DIR}/*.cpp")
# cpp/tests/ 下的文件为独立测试，不参与库编译

find_package(fbjni REQUIRED CONFIG)
find_package(ReactAndroid REQUIRED CONFIG)

add_library(${CMAKE_PROJECT_NAME} SHARED ${LIB_CPP_SOURCES})

include("${REACT_NATIVE_DIR}/ReactCommon/cmake-utils/react-native-flags.cmake")
target_compile_reactnative_options(${CMAKE_PROJECT_NAME} PUBLIC)

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC "${LIB_CPP_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/build/generated/source/codegen/jni"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/build/generated/source/codegen/jni/react/renderer/components/EncryptionCoreSpecs"
)

# Codegen's CMakeLists expects un-namespaced targets like `reactnative` and `jsi`
if(NOT TARGET jsi)
    add_library(jsi ALIAS ReactAndroid::jsi)
endif()
if(NOT TARGET reactnative)
    add_library(reactnative ALIAS ReactAndroid::reactnative)
endif()
if(NOT TARGET fbjni)
    add_library(fbjni ALIAS fbjni::fbjni)
endif()

# Link codegen target if it exists, or include it directly
if(NOT TARGET react_codegen_EncryptionCoreSpecs)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/generated/source/codegen/jni")
        add_subdirectory(
            "${CMAKE_CURRENT_SOURCE_DIR}/build/generated/source/codegen/jni"
            react_codegen_EncryptionCoreSpecs
        )
    endif()
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
    log
    ReactAndroid::reactnative
    ReactAndroid::jsi
    fbjni::fbjni
    react_codegen_EncryptionCoreSpecs
)
