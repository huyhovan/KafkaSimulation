cmake_minimum_required(VERSION 3.10)
cmake_minimum_required(VERSION 3.10)

project (KafkaSimulation)

set (CMAKE_BUILD_TYPE Release)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_definitions ( -D_UNICODE )
remove_definitions ( -D_MBCS )

#
file(GLOB INCLUDES "${PROJECT_SOURCE_DIR}/KafkaSimulation/*.h")
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/KafkaSimulation/*.cpp")


SET(KAFKA_SIMULATOR_DIR ${PROJECT_SOURCE_DIR} )
SET(KAFKA_SIMULATOR_INCLUDE_DIR ${KAFKA_SIMULATOR_DIR}/Libraries/include )
SET(KAFKA_SIMULATOR_LIB_DIR ${KAFKA_SIMULATOR_DIR}/Libraries/lib)

include_directories(${KAFKA_SIMULATOR_INCLUDE_DIR})
include_directories(${KAFKA_SIMULATOR_DIR})
include_directories(${KAFKA_SIMULATOR_DIR}/Dependencies)
include_directories(${KAFKA_SIMULATOR_DIR}/Dependencies/lua)
include_directories(${KAFKA_SIMULATOR_DIR}/Dependencies/sol)
include_directories("C:/local/boost_1_73_0")

link_directories(${KAFKA_SIMULATOR_LIB_DIR})
link_directories(${KAFKA_SIMULATOR_DIR}/Lib64)
link_directories("C:/local/boost_1_73_0/lib64-msvc-14.2")

add_executable(KafkaSimulation ${SOURCES} ${INCLUDES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/Debug64)
set_target_properties(KafkaSimulation PROPERTIES  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

set_property(TARGET KafkaSimulation PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${KAFKA_SIMULATOR_DIR}/Libraries/bin")

TARGET_LINK_LIBRARIES(KafkaSimulation  OABaseD.lib OAModelDataAPID.lib rdkafka++.lib rdkafka.lib ScriptModuleD.lib )

add_subdirectory(ScriptModule)
add_subdirectory(Lua)
