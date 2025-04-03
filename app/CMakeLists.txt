cmake_minimum_required(VERSION 3.30) # CMake install : https://cmake.org/download/
project(QtChatting LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/Qt/6.8.2/msvc2022_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Network)# Qt COMPONENTS
# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

aux_source_directory(./src SRC1)
aux_source_directory(./widgets SRC2)
file(GLOB_RECURSE SRC3 ${CMAKE_CURRENT_SOURCE_DIR}/detail/*.cpp )
set(SRCS ${SRC1} ${SRC2} ${SRC3})
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/detail/include
)
file(GLOB_RECURSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/out/)

add_executable(${PROJECT_NAME}
    ${SRCS}
    ${RESOURCES}
) 
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets PRIVATE Qt6::Network) 
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE ON
)
set(QT_DIR D:/Qt/6.8.2/msvc2022_64)
if(WIN32)
    # 定义部署命令
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt dependencies..."
        COMMAND ${QT_DIR}/bin/windeployqt.exe
            --no-translations
            --no-system-d3d-compiler
            --no-compiler-runtime
            --dir ${CMAKE_CURRENT_SOURCE_DIR}/out/
            $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Running windeployqt"
    )
endif()