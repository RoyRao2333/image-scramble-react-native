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
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    log
    ReactAndroid::reactnative
    ReactAndroid::jsi
    fbjni::fbjni
)
