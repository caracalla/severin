add_executable(severin
  main.cpp
  util.h
  util.cpp
  input.h
  window_handler.h
  window_handler_sdl.cpp
  engine.h
  engine.cpp
  level.h
  model.h
  model.cpp
  entity.h
  collision.h
  scene.h
  scene.cpp
  renderer.h)

target_include_directories(severin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(severin vkbootstrap vma glm tinyobjloader stb)

target_link_libraries(severin Vulkan::Vulkan sdl2)
