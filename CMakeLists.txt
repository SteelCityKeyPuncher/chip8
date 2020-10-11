# Project definition
cmake_minimum_required(VERSION 3.1)
project(chip8)

# Create a file that's used by clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(SOURCES
    "${SRC_DIR}/Main.cpp"
    "${SRC_DIR}/Window.cpp"
)

# Executable definition and properties
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
    set_source_files_properties("${SRC_DIR}/Util/StbImage.cpp" PROPERTIES COMPILE_FLAGS -Wno-error)
endif()

# GLFW
set(GLFW_DIR "${THIRD_PARTY_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# glad
set(GLAD_DIR "${THIRD_PARTY_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

# glm
set(GLM_DIR "${THIRD_PARTY_DIR}/glm")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLM_DIR}")
