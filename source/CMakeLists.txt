#######################################
# Required CMake version#
#######################################
cmake_minimum_required( VERSION 2.8.9 )

#######################################
# Project name                        #
#######################################
project(OpENerMain CXX)

add_definitions(-std=c++11)

#set(OpENer_USETHREAD ON)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
unset(CMAKE_LINK_LIBRARY_SUFFIX)

#######################################
# Project version                     #
#######################################
set( OpENer_VERSION_MAJOR 1 )
set( OpENer_VERSION_MINOR 2 )

find_path( OpENer_BUILDSUPPORT_DIR OpENer.cmake ${PROJECT_SOURCE_DIR}/buildsupport )
 
INCLUDE( ${OpENer_BUILDSUPPORT_DIR}/OpENer.cmake )


#######################################
# Platform switches                   #
#######################################
set( OpENer_KNOWN_PLATFORMS "POSIX" "WIN32" )
set( OpENer_PLATFORM CACHE STRINGS "Platform OpENer will be built for" )
set_property(CACHE OpENer_PLATFORM PROPERTY STRINGS ${OpENer_KNOWN_PLATFORMS} )

if (OpENer_PLATFORM STREQUAL WIN32)
  add_definitions(-D__WIN32__)
  set (__WIN32__)
else()
  add_definitions(-D__linux__)
  unset(__WIN32__)
endif()

if (OpENer_USETHREAD)
    add_definitions(-DUSETHREAD)
endif()

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
  add_subdirectory(tests)
endif( OpENer_TESTS )

#######################################
# OpENer C flags                      #
#######################################
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )


#######################################
# Add subdirectories                  #
#######################################
opener_common_includes()

add_subdirectory( src )
add_executable(OpENerMain src/main.cpp)


if(__WIN32__)
    if(MINGW)
        target_link_libraries(OpENerMain OpENerLib winmm)
    else ()
        add_definitions(-DVSTUDIO)
        target_link_libraries(OpENerMain OpENerLib)
    endif()
else()
    target_link_libraries(OpENerMain OpENerLib)
endif()