#######################################
# Required CMake version#
#######################################
cmake_minimum_required( VERSION 3.0.2 )

#######################################
# Project name                        #
#######################################
project(OpENerMain CXX)

include(buildsupport/macros_and_definitions.cmake)

#######################################
# Project version                     #
#######################################
set( OpENer_VERSION_MAJOR 0 )
set( OpENer_VERSION_MINOR 1 )

#######################################
# Platform switch                     #
#######################################
set( OpENer_KNOWN_PLATFORMS "POSIX" "WIN32" )
set( OpENer_PLATFORM CACHE STRINGS "Platform OpENer will be built for" )
set_property(CACHE OpENer_PLATFORM PROPERTY STRINGS ${OpENer_KNOWN_PLATFORMS} )

#######################################
# Thread switch                       #
#######################################
option(OpENer_USETHREAD "Use thread support" OFF)

#######################################
# OpENer tracer switches              #
#######################################
option( OpENer_TRACES "Activate OpENer traces" OFF)

#######################################
# Test switch                         #
#######################################
option( OpENer_TESTS "Enable tests to be built" OFF)
set(OpENer_TESTS ON)

#######################################
# Debug switch                         #
#######################################
option( OpENer_DEBUG "Build OpENer-lib in debug mode" OFF)
set(OpENer_DEBUG ON)


#######################################
# Process options                     #
#######################################
process_options()

#######################################
# Add subdirectories                  #
#######################################
opener_common_includes()

add_subdirectory( src )

add_executable(OpENerMain main.cpp)


if(WIN32)
    target_link_libraries(OpENerMain OpENerLib winmm.lib)
else()
    target_link_libraries(OpENerMain OpENerLib)
endif()

