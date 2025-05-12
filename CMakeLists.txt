cmake_minimum_required(VERSION 3.10)
project(CPUInfo LANGUAGES CXX)

# Force static linking
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# We require C++17 for <string_view>, structured bindings, etc.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers under 'include/'
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/cpu.cpp
    src/main.cpp
    src/tools.cpp
)

# Build the executable
add_executable(cpuinfo ${SOURCES})

# Optionally install
install(TARGETS cpuinfo RUNTIME DESTINATION bin)