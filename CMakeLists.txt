cmake_minimum_required(VERSION 3.10)

# 定义项目名称
project(MiniVim)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 手动设置 ncurses 的头文件路径和库路径
include_directories("/usr/include")
set(CURSES_LIBRARIES "/usr/lib/x86_64-linux-gnu/libncurses.so")

# 打印调试信息，确保路径正确
message(STATUS "CURSES_INCLUDE_DIR: /usr/include")
message(STATUS "CURSES_LIBRARIES: ${CURSES_LIBRARIES}")

# 添加头文件路径
include_directories(include)

# 定义源文件
set(SOURCES
    src/MiniVim.cpp
    main.cpp
)

# 定义可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接 ncurses 库
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
