cmake_minimum_required(VERSION 3.22)
project(FireworksGL C)
set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
)
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
)
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(
  ${PROJECT_NAME}
    PROPERTIES 
    OUTPUT_NAME ${PROJECT_NAME}
    SUFFIX ".scr"
)

message(STATUS "Found source files:")
message(STATUS ${SOURCE_FILES})
message(STATUS "Found header files:")
message(STATUS ${SOURCE_FILES})

# Variable for the Libs to add to the Linkers
if (WIN32)
	set(LIBS glfw opengl32 glad)
elseif (UNIX)
	set(LIBS glfw GL glad)
endif ()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GLFW_ROOT lib/glfw)
set(GLAD_ROOT lib/glad)
message(STATUS "Using GLFW root:")
message(STATUS ${GLFW_ROOT})
message(STATUS "Using GLAD root:")
message(STATUS ${GLAD_ROOT})
add_subdirectory(lib/glfw)
add_subdirectory(lib/glad)

target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_ROOT}/include PUBLIC ${GLAD_ROOT}/include/glad)
target_link_directories(${PROJECT_NAME} PRIVATE ${GLFW_ROOT}/src PRIVATE ${GLAD_ROOT}/src)
target_link_libraries(${PROJECT_NAME} ${LIBS})
