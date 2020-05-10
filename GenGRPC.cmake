
function(GRPC_GENERATE_CPP SRCS HDRS)
    cmake_parse_arguments(grpc "" "" "" ${ARGN})

    set(PROTO_FILES "${grpc_UNPARSED_ARGUMENTS}")
    if(NOT PROTO_FILES)
        message(SEND_ERROR "Error: GRPC_GENERATE_CPP() called without any proto files")
        return()
    endif()

    set(${SRCS})
    set(${HDRS})
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_FILE_ABS ${PROTO_FILE} ABSOLUTE)
        get_filename_component(PROTO_FILE_WE ${PROTO_FILE} NAME_WE)
        set(PROTO_PB_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_FILE_WE}.pb.cc")
        set(PROTO_PB_H "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_FILE_WE}.pb.h")
        set(PROTO_GRPC_H "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_FILE_WE}.grpc.pb.cc")
        set(PROTO_GRPC_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_FILE_WE}.grpc.pb.h")

        list(APPEND ${SRCS} "${PROTO_PB_CC}" "${PROTO_GRPC_CC}")
        list(APPEND ${HDRS} "${PROTO_PB_H}" "${PROTO_GRPC_H}")

        add_custom_command(
            OUTPUT "${PROTO_PB_CC}" "${PROTO_GRPC_CC}" "${PROTO_PB_H}"
              "${PROTO_GRPC_H}"
            COMMAND ${_PROTOBUF_PROTOC}
            ARGS --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
              --grpc_out=generate_mock_code=true:"${CMAKE_CURRENT_BINARY_DIR}"
              --proto_path="${CMAKE_CURRENT_SOURCE_DIR}"
              --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
              ${PROTO_FILE}
            DEPENDS ${PROTO_FILE_ABS} ${_PROTOBUF_PROTOC} ${_GRPC_CPP_PLUGIN_EXECUTABLE}
            COMMENT "Running C++ protocol buffer compiler on ${PROTO_FILE}"
        )
    endforeach()

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
