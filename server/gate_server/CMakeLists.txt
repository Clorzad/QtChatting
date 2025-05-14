
# Project name
project(gate_server LANGUAGES CXX)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/out/)
# Find Boost Jsoncpp
find_package(Boost CONFIG REQUIRED COMPONENTS system thread filesystem)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
# Add source files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

                
include_directories(
    ${BOOST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE ${Boost_LIBRARIES} 
    PRIVATE ${JSONCPP_LIBRARIES}
    PRIVATE qtchatting_proto
)
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/config.ini  ${EXECUTABLE_OUTPUT_PATH}
)

