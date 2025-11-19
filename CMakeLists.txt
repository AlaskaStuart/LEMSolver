cmake_minimum_required(VERSION 3.15)
Project(CMakeDemo LANGUAGES CXX)

# ===== 基本配置 =====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ===== 编译器选项 =====
# if(MSVC)
#     message(STATUS "Using MSVC compiler")
#     add_compile_options(/EHsc)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
#     message(STATUS "Using Clang compiler")
#     add_compile_options(-Wall -Wextra -Wpedantic -O2)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     message(STATUS "Using GCC compiler")
#     add_compile_options(-Wall -Wextra -Wpedantic -O2)
# else()
#     message(WARNING "Unknown compiler, default flags may not be optimal.")
# endif()
# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     add_compile_options(-Wall -Wextra -Wpedantic -O2)
# endif()

# ===== VCPKG（推荐用外部配置）=====
# 不直接在这里写 toolchain 文件路径
# set(CMAKE_TOOLCHAIN_FILE "$ENV{vcpkgRoot}/scripts/buildsystems/vcpkg.cmake")
# ===== MSVC 编码修复（核心）=====
if (MSVC)
    add_compile_options(/utf-8)
endif()
# ===== 查找依赖 =====
find_package(Eigen3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
# ===== 源文件 =====
file(GLOB SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})

# ===== include 路径 =====
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        nlohmann_json::nlohmann_json
        Eigen3::Eigen
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")