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

link_directories(${KAFKA_SIMULATOR_LIB_DIR})

add_executable(KafkaSimulation ${SOURCES} ${INCLUDES})

set_property(TARGET KafkaSimulation PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${KAFKA_SIMULATOR_DIR}/Libraries/bin")

TARGET_LINK_LIBRARIES(KafkaSimulation  OABaseD.lib OAModelDataAPID.lib rdkafka++.lib rdkafka.lib ScriptModuleD.lib )