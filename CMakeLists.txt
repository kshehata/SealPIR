cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SealPIR VERSION 2.1 LANGUAGES CXX)

include(FetchGRPC.cmake)
include(GenGRPC.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(EXTERNAL_DIR ${SealPIR_SOURCE_DIR}/external)
set(SEAL_SRC_DIR ${EXTERNAL_DIR}/seal/src/native/src)

message(STATUS "Setting up SEAL ...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    OUTPUT_QUIET
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${EXTERNAL_DIR}/seal)
if(result)
    message(FATAL_ERROR "Failed to download SEAL (${result})!")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
    OUTPUT_QUIET
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${EXTERNAL_DIR}/seal)
if(result)
    message(FATAL_ERROR "Failed to build SEAL (${result})!")
endif()

add_subdirectory(
  ${SEAL_SRC_DIR}
  ${EXTERNAL_DIR}/seal/build)

include_directories(
  ${SEAL_SRC_DIR})


grpc_generate_cpp(PROTO_SRC PROTO_HEADER pir.proto)
add_library(proto_lib
  ${PROTO_SRC}
  ${PROTO_HEADER}
)

add_executable(main 
	main.cpp
)

add_library(sealpir STATIC
  ${PROTO_HEADER}
  pir.cpp
  pir_client.cpp
  pir_database.cpp
  pir_server.cpp
)

target_link_libraries(main sealpir seal proto_lib ${_PROTOBUF_LIBPROTOBUF})


add_executable(server rpc_server.cpp ${PROTO_HEADER})

target_link_libraries(server
  proto_lib
  seal
  sealpir
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)

add_executable(client rpc_client.cpp ${PROTO_HEADER})

target_link_libraries(client
  proto_lib
  seal
  sealpir
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)

add_executable(gen_db gen_db.cpp ${PROTO_HEADER})

target_link_libraries(gen_db
  proto_lib
  seal
  sealpir
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)


#####################################################################
# Token Transforms
#####################################################################

message(STATUS "Setting up libsodium ...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    OUTPUT_QUIET
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${EXTERNAL_DIR}/libsodium)
if(result)
    message(FATAL_ERROR "Failed to download libsodium (${result})!")
endif()

message(STATUS "Building libsodium ...")
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    OUTPUT_QUIET
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${EXTERNAL_DIR}/libsodium)
if(result)
    message(FATAL_ERROR "Failed to build libsodium (${result})!")
endif()

include_directories(
  ${EXTERNAL_DIR}/libsodium/src/src/libsodium/include/)

link_directories(
  ${EXTERNAL_DIR}/libsodium/src/src/libsodium/.libs/)

add_library(token_transforms_lib STATIC token_transforms.cpp)


include(FetchGoogletest.cmake)
enable_testing()

add_executable(TokenTransformsTest token_transforms_test.cpp)
target_link_libraries(TokenTransformsTest
  token_transforms_lib
  libsodium.a
  gtest_main)
add_test(TokenTransformsTest bin/TokenTransformsTest)

