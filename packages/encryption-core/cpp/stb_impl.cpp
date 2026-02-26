// stb 库的实现编译单元
// 确保 STB_IMAGE_IMPLEMENTATION 和 STB_IMAGE_WRITE_IMPLEMENTATION
// 仅在一个翻译单元中定义一次

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO  // 不使用 stdio，我们自行管理文件读取
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO  // 不使用 stdio，使用回调写入内存
#include "stb_image_write.h"
