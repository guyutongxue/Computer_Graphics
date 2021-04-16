
add_executable(main 
    main.cpp 
    texture_image.h 
    texture_image.cpp
    skeletal_mesh.h
    skeletal_mesh.cpp
)

conan_target_link_libraries(main PRIVATE glfw glew stb glm assimp)

add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/data/Hand.fbx
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Hand.fbx)