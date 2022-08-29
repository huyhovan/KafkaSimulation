#Build Shared library: ScriptModule 

cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(ScriptModule)

set(CMAKE_BUILD_TYPE Debug)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
SET(SIMULATOR_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PARENT_DIR}/Dependencies)
include_directories(${PARENT_DIR}/Dependencies/lua)
include_directories(${SIMULATOR_DIR})

link_directories(${PARENT_DIR}/Lib64)

#
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB INCLUDES "${PROJECT_SOURCE_DIR}/*.h")

add_compile_options(-D SCRIPTMODULE_EXPORTS)
add_definitions(-DUNICODE -D_UNICODE)

add_library(ScriptModule SHARED ${SOURCES} ${INCLUDES})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PARENT_DIR}/Lib64)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PARENT_DIR}/Debug64)
set_target_properties(ScriptModule PROPERTIES  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
set_target_properties(ScriptModule PROPERTIES  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

#set_target_properties(SimulationDataCenter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set_target_properties(ScriptModule PROPERTIES DEBUG_POSTFIX "D")

TARGET_LINK_LIBRARIES(ScriptModule LuaD.lib OABaseD.lib OAModelDataAPID.lib)