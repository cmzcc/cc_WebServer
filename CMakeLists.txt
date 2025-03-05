cmake_minimum_required(VERSION 3.10)

project(cc_WebServer LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录并确保其存在
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY})

# 查找源文件
file(GLOB_RECURSE SOURCES "*.cpp")

# 查找cc_muduo库路径（假设您已经安装了cc_muduo或已将其源代码添加到项目中）
# 请根据您的cc_muduo库实际路径修改该路径
set(CC_MUDUO_PATH "/usr/include/cc_muduo")
include_directories(${CC_MUDUO_PATH})

# 查找cc_muduo库文件
find_library(CC_MUDUO_LIBRARY NAMES cc_muduo PATHS /usr/lib)

# 如果找到了cc_muduo库，链接到项目
if(CC_MUDUO_LIBRARY)
    message(STATUS "Found cc_muduo library: ${CC_MUDUO_LIBRARY}")
else()
    message(WARNING "cc_muduo library not found, make sure the path is correct")
endif()

# 查找并链接系统线程库
find_package(Threads REQUIRED)

# 创建HttpServer可执行文件
add_executable(HttpServer ${SOURCES})

# 链接cc_muduo库和线程库
target_link_libraries(HttpServer ${CC_MUDUO_LIBRARY} Threads::Threads)

# 可选：调试模式下添加调试信息
target_compile_options(HttpServer PRIVATE $<$<CONFIG:Debug>:-g>)

# 设置目标输出目录
set_target_properties(HttpServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
