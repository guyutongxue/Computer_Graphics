
add_executable(main 
    main.cpp 
    texture_image.h 
    texture_image.cpp
    skeletal_mesh.h
    skeletal_mesh.cpp

    # imgui backends
    imgui/imgui_impl_glfw.h
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_impl_opengl3.cpp
)

conan_target_link_libraries(main PRIVATE glfw glew stb glm assimp imgui)

# linkage to Windows IMM library is necessary for Dear Imgui
if(WIN32)
    target_link_libraries(main imm32 version winmm)
endif()

add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/data/Hand.fbx
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Hand.fbx)