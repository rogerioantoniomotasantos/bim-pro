cmake_minimum_required(VERSION 3.15)
project(BIMGeometryServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# Find packages
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# IfcOpenShell - assume installed in /usr/local
set(IFCOPENSHELL_INCLUDE_DIR "/usr/local/include/ifcopenshell")
set(IFCOPENSHELL_LIBRARY_DIR "/usr/local/lib")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIRS}
    ${IFCOPENSHELL_INCLUDE_DIR}
)

# Generate protobuf and gRPC code
set(PROTO_FILES proto/geometry.proto)
set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto)

add_custom_command(
    OUTPUT 
        ${CMAKE_CURRENT_BINARY_DIR}/geometry.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/geometry.pb.h
        ${CMAKE_CURRENT_BINARY_DIR}/geometry.grpc.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/geometry.grpc.pb.h
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS 
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        -I${PROTO_PATH}
        ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
)

# Source files
set(SOURCES
    src/main.cpp
    src/geometry_server.cpp
    src/ifc_processor.cpp
    src/mesh_generator.cpp
    src/lod_generator.cpp
    src/mesh_optimizer.cpp
    src/tile_generator.cpp
    src/spatial_index.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/geometry.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/geometry.grpc.pb.cc
)

# Executable
add_executable(geometry_server ${SOURCES})

# Link libraries
target_link_libraries(geometry_server
    ${PROTOBUF_LIBRARIES}
    gRPC::grpc++
    gRPC::grpc++_reflection
    Threads::Threads
    ${IFCOPENSHELL_LIBRARY_DIR}/libIfcGeom.so
    ${IFCOPENSHELL_LIBRARY_DIR}/libIfcParse.so
)

# Install
install(TARGETS geometry_server DESTINATION bin)
