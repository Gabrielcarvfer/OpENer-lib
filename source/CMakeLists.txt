#######################################
# Required CMake version#
#######################################
cmake_minimum_required( VERSION 2.8.9 )

#######################################
# Project name                        #
#######################################
project(OpENerMain CXX)

#######################################
# Project version                     #
#######################################
set( OpENer_VERSION_MAJOR 1 )
set( OpENer_VERSION_MINOR 2 )

#find_path( OpENer_BUILDSUPPORT_DIR OpENer.cmake ${PROJECT_SOURCE_DIR}/buildsupport )
 
#INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer.cmake )


#######################################
# Platform switches                   #
#######################################
set( OpENer_KNOWN_PLATFORMS "POSIX" "WIN32" )

set( OpENer_PLATFORM CACHE STRINGS "Platform OpENer will be built for" )
set_property(CACHE OpENer_PLATFORM PROPERTY STRINGS ${OpENer_KNOWN_PLATFORMS} )

#######################################
# Platform-dependent functions check  #
#######################################
#INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer_function_checks.cmake )

#######################################
# OpENer 64-bit data types            #
#######################################
set( OpENer_64_BIT_DATA_TYPES_ENABLED CACHE BOOL "Enable 64-bit data type support" )
if( OpENer_64_BIT_DATA_TYPES_ENABLED )
  add_definitions( -DOPENER_SUPPORT_64BIT_DATATYPES )
endif( OpENer_64_BIT_DATA_TYPES_ENABLED )

#######################################
# OpENer tracer switches              #
#######################################
set( OpENer_TRACES OFF CACHE BOOL "Activate OpENer traces" )
if(OpENer_TRACES)
  createTraceLevelOptions()
endif(OpENer_TRACES)

#######################################
# Test switch                         #
#######################################
set( OpENer_TESTS OFF CACHE BOOL "Enable tests to be built" )
if( OpENer_TESTS )
  enable_testing()
  enable_language( CXX )
  set( CPPUTEST_HOME "" CACHE PATH "Path to CppUTest directory" )
  INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer_Tests.cmake )
  add_subdirectory( tests )
endif( OpENer_TESTS )

#######################################
# OpENer C flags                      #
#######################################
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

####################################################
# Internal cache holding the available CIP objects #
####################################################
#set( OpENer_ADD_CIP_OBJECTS "" CACHE INTERNAL STRING )

#######################################
# Add custom CIP objects              #
#######################################
#set( OpENer_CIP_OBJECTS_DIR ${PROJECT_SOURCE_DIR}/src/cip_objects )
#include(${OpENer_BUILDSUPPORT_DIR}/OpENer_CIP_Object_generator.cmake)


MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES(includes)
include_directories(${includes})
#######################################
# Add subdirectories                  #
#######################################

add_subdirectory( src )

if(WIN32)
  add_executable(OpENerMain src/ports/WIN32/main.cpp)# ${OPENER_SRC})
  target_link_libraries(OpENerMain OpENer wsock32 ws2_32)# OpENer_CIP OpENer_CLASS OpENer_UTILS OpENer_CONN OpENer_NET CIP_NET_ETHIP)
endif()
