add_executable(
  main
  main.cpp
  camera.h
  camera.cpp
  shader.h
  shader.cpp
  model.hpp
  mesh.hpp
  light.hpp
  # imgui backends
  imgui/imgui_impl_glfw.h
  imgui/imgui_impl_glfw.cpp
  imgui/imgui_impl_opengl3.h
  imgui/imgui_impl_opengl3.cpp)
target_link_libraries(main ${CONAN_LIBS})

# linkage to Windows IMM library is necessary for Dear Imgui
if(WIN32)
  target_link_libraries(main imm32 version winmm)
endif()

add_custom_target(
  copy_models
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_custom_target(
  copy_shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders
          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_dependencies(main copy_shaders copy_models)
