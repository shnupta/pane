cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(pane)

add_subdirectory(spdlog)
find_package(CapnProto REQUIRED)

set(PANE_LIB_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE PANE_LIB_SOURCE_FILES "${PANE_LIB_SRC_DIR}/*.cc")
message("lib files ${PANE_LIB_SOURCE_FILES}")

file(GLOB PANE_PROTO_FILES "${PANE_LIB_SRC_DIR}/pane/proto/*.capnp")
message("proto files ${PANE_PROTO_FILES}")
capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS ${PANE_PROTO_FILES})

add_library(panelib ${PANE_LIB_SOURCE_FILES} ${CAPNP_SRCS} ${CAPNP_HDRS})
target_include_directories(panelib PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
	$<BUILD_INTERFACE:${PANE_LIB_SRC_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(panelib PUBLIC spdlog::spdlog CapnProto::capnp-rpc)

set(PANE_BIN_DIR ${PROJECT_SOURCE_DIR}/pane)
file(GLOB_RECURSE PANE_BIN_SOURCE_FILES "${PANE_BIN_DIR}/*.cc")

add_executable(pane ${PANE_BIN_SOURCE_FILES})
target_link_libraries(pane PRIVATE panelib)
