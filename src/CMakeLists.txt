add_executable(severin
  main.cpp
  util.h
  util.cpp
  input.h
  window_handler.h
  window_handler_sdl.cpp
  engine.h
  scene.h
  renderer.h
  renderer.cpp)

target_include_directories(severin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(severin glm)

target_link_libraries(severin Vulkan::Vulkan sdl2)
