cmake_minimum_required(VERSION 3.18.4)

set(REPO shsh)
project(${REPO})
set(PROJECT_NAME ${REPO})

set(CMAKE_C_STANDARD 99)

set(CPP_FLAGS "-Wall -Wextra -pedantic -ggdb")

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/driver.c ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        COMPILE_FLAGS ${CPP_FLAGS}
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin
        )

set_property(
        TARGET ${PROJECT_NAME}
        PROPERTY
        CMAKE_OBJECT_PATH_PREFIX ${PROJECT_SOURCE_DIR}/build/obj/
)

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build/cmake)

include_directories(${PROJECT_SOURCE_DIR}/build/cmake)

target_link_libraries(shsh pthread) # Debian (skill) issue
