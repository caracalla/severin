cmake_minimum_required(VERSION 3.8)

project("severin")

set(CMAKE_CXX_STANDARD 17)

# find_package(Vulkan REQUIRED)

# add_subdirectory(third_party)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(src)
