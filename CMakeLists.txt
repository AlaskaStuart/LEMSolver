cmake_minimum_required(VERSION 3.15)

Project(LEMSolver LANGUAGES CXX)
add_compile_options(/utf-8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# if(MSVC)
#     add_compile_options(/EHsc)
# else()
#     add_compile_options(-Wall -Wextra -02)
# endif()
# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     add_compile_options(-Wall -Wextra -Wpedantic -O2)
# endif()

# set(CMAKE_TOOLCHAIN_FILE "$ENV{vcpkgRoot}/scripts/buildsystems/vcpkg.cmake")

find_package(Eigen3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB SRC_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# add_executable(${PROJECT_NAME} main.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json Eigen3::Eigen)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler path:${CMAKE_CXX_COMPILER}")