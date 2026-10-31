cmake_minimum_required(VERSION 3.16)
project(QuartoInterativo LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Esta versao inicial usa Win32/WGL e precisa de Windows.")
endif()

set(SCENE_SOURCES src/scene.cpp src/geometry.cpp src/textures.cpp src/behavior.cpp src/landscape.cpp)
add_executable(quarto src/main.cpp src/image_loader.cpp src/verification.cpp ${SCENE_SOURCES})
target_compile_features(quarto PRIVATE cxx_std_17)
target_link_libraries(quarto PRIVATE opengl32 gdi32 user32 ole32 windowscodecs)
add_custom_command(TARGET quarto POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:quarto>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/quadro.jpg"
            "$<TARGET_FILE_DIR:quarto>/assets/quadro.jpg")

if(MINGW)
    target_compile_options(quarto PRIVATE -Wall -Wextra -Wpedantic)
else()
    target_compile_options(quarto PRIVATE /W4)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(scene_tests tests/scene_tests.cpp ${SCENE_SOURCES})
    target_compile_features(scene_tests PRIVATE cxx_std_17)
    target_include_directories(scene_tests PRIVATE src)
    add_test(NAME scene_geometry_navigation COMMAND scene_tests)
    add_executable(behavior_tests tests/behavior_tests.cpp ${SCENE_SOURCES})
    target_compile_features(behavior_tests PRIVATE cxx_std_17)
    target_include_directories(behavior_tests PRIVATE src)
    add_test(NAME interactions_physics_lighting COMMAND behavior_tests)
endif()
