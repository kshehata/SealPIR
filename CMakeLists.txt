cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SealPIR VERSION 2.1 LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(EXTERNAL_DIR ${SealPIR_SOURCE_DIR}/external)

message(STATUS "Setting up SEAL ...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    OUTPUT_QUIET
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${EXTERNAL_DIR}/seal)
if(result)
    message(WARNING "Failed to download SEAL (${result})!")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
    OUTPUT_QUIET
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${EXTERNAL_DIR}/seal)
if(result)
    message(WARNING "Failed to build SEAL (${result})!")
endif()

add_subdirectory(
  ${EXTERNAL_DIR}/seal/src
  ${EXTERNAL_DIR}/seal/build)

include_directories(
  ${EXTERNAL_DIR}/seal/src/native/src/
  ${EXTERNAL_DIR}/seal/src/thirdparty/msgsl/src/include/)

add_executable(main 
	main.cpp
)

add_library(sealpir STATIC
  pir.cpp
  pir_client.cpp
  pir_server.cpp
)

# find_package(SEAL 3.5.0 EXACT REQUIRED)

target_link_libraries(main sealpir seal)
