cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(teja)

add_subdirectory(quill)

set(TEJA_LIB_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE TEJA_LIB_SOURCE_FILES "${TEJA_LIB_SRC_DIR}/*.cc")
message("lib files ${TEJA_LIB_SOURCE_FILES}")

add_library(tejalib ${TEJA_LIB_SOURCE_FILES})
target_include_directories(tejalib PUBLIC
	$<BUILD_INTERFACE:${TEJA_LIB_SRC_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(tejalib PUBLIC quill::quill)

set(TEJA_BIN_DIR ${PROJECT_SOURCE_DIR}/teja)
file(GLOB_RECURSE TEJA_BIN_SOURCE_FILES "${TEJA_BIN_DIR}/*.cc")

add_executable(teja ${TEJA_BIN_SOURCE_FILES})
target_link_libraries(teja PRIVATE tejalib)
