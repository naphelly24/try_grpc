# Any project's top most CMakeLists.txt must start by specifying a minimum CMake version using the cmake_minimum_required() command.
cmake_minimum_required(VERSION 3.5.0)

# To start a project, we use the project() command to set the project name.
# This call is required with every project and should be called soon after cmake_minimum_required().
project(trygrpc VERSION 0.1.0 LANGUAGES C CXX)
add_subdirectory(server)
add_subdirectory(client)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CPack)


# Protobuf and gRPC
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
# set(protobuf_MODULE_COMPATIBLE TRUE)

# set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
# set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# set(_GRPC_GRPCPP gRPC::grpc++ gRPC::grpc++_reflection)
# set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

add_library(protolib protos/hello.proto)
target_link_libraries(protolib gRPC::grpc++)
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET protolib LANGUAGE cpp)
protobuf_generate(TARGET protolib LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")