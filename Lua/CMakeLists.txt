#Build Shared library: Lua

cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Lua)

set(CMAKE_BUILD_TYPE Debug)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
SET(LUA_DIR ${PARENT_DIR}/Dependencies/lua)
SET(SOL_DIR ${PARENT_DIR}/Dependencies/sol)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PARENT_DIR}/Dependencies/lua)

add_compile_options(-D LUA_EXPORTS)
add_compile_options(-D Lua_EXPORTS)
add_compile_options(-D LUA_COMPAT_ALL)
add_compile_options(-D LUA_BUILD_AS_DLL)

add_definitions(-DUNICODE -D_UNICODE)

# lua lib
add_library(
    lua STATIC
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/linit.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lua.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PARENT_DIR}/Lib64)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PARENT_DIR}/Debug64)
set_target_properties(Lua PROPERTIES  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
set_target_properties(Lua PROPERTIES  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

#set_target_properties(SimulationDataCenter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set_target_properties(Lua PROPERTIES DEBUG_POSTFIX "D")

