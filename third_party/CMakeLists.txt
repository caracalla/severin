find_package(Vulkan REQUIRED)

# glm
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

# vma
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# tinyobjloader
add_library(tinyobjloader STATIC)

target_sources(tinyobjloader PRIVATE
    tinyobjloader/tiny_obj_loader.h
    tinyobjloader/tiny_obj_loader.cc
    )

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

# vk-bootstrap
add_library(vkbootstrap STATIC)

target_sources(vkbootstrap PRIVATE
    vk-bootstrap/VkBootstrap.h
    vk-bootstrap/VkBootstrap.cpp
    )
target_include_directories(vkbootstrap PUBLIC vk-bootstrap)

target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

# sdl2
add_library(sdl2 INTERFACE)

if (WIN32)
# obviously this is for my own Windows setup, adjust as needed
set(sdl2_DIR "C:/Users/james/source/sdl_dev" CACHE PATH "path to your SDL installation")
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include ${sdl2_image_DIR}/include)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib/x64 ${sdl2_image_DIR}/lib/x64)
elseif (UNIX)
set(sdl2_DIR "SDL_PATH" CACHE PATH "path to your SDL installation")
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2 ${sdl2_image_DIR}/include/SDL2)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
endif()

target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
