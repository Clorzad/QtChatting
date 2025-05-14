find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/protos/message.proto)

add_library(qtchatting_proto ${PROTO_FILES})
target_link_libraries(qtchatting_proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(qtchatting_proto 
    PUBLIC 
        ${CMAKE_CURRENT_BINARY_DIR}
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(
    TARGET qtchatting_proto 
    LANGUAGE cpp
)
protobuf_generate(
    TARGET qtchatting_proto 
    LANGUAGE grpc 
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)
