FUNCTION(opener_add_definition)
  FOREACH(ARG ${ARGV})
    set_property(GLOBAL APPEND PROPERTY OPENER_DEFINITION ${ARG})
  ENDFOREACH(ARG)
ENDFUNCTION(opener_add_definition)

##############################################
# Adds platform specific include directories #
############################################## 
macro(opener_platform_support ARGS)

  if(OpENer_PLATFORM STREQUAL "")
    message(FATAL_ERROR "No platform selected!")  
  endif(OpENer_PLATFORM STREQUAL "")

  include( ${OpENer_BUILDSUPPORT_DIR}/${OpENer_PLATFORM}/OpENer_PLATFORM_${ARGS}.cmake)
  opener_platform_spec()
endmacro(opener_platform_support ARGS)


#######################################
# Adds common Include directories     #
####################################### 
macro(opener_common_includes)
  HEADER_DIRECTORIES(includes)
  include_directories(${includes})
endmacro(opener_common_includes)

MACRO(HEADER_DIRECTORIES return_list)
  FILE(GLOB_RECURSE new_list ${PROJECT_SOURCE_DIR}/*.h)
  SET(dir_list "")
  FOREACH(file_path ${new_list})
    GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
    SET(dir_list ${dir_list} ${dir_path})
  ENDFOREACH()
  LIST(REMOVE_DUPLICATES dir_list)
  SET(${return_list} ${dir_list})
ENDMACRO()

MACRO(opener_add_cip_object NAME DESCRIPTION)
  set(OpENer_CIP_OBJECT_${NAME} OFF CACHE BOOL "${DESCRIPTION}")
  FOREACH(dependencies ${ARGN})
	if(NOT ${dependencies})
	return()
	endif(NOT ${dependencies})
  ENDFOREACH(dependencies)
  if(NOT OpENer_CIP_OBJECT_${NAME})
    return()
  endif(NOT OpENer_CIP_OBJECT_${NAME})
ENDMACRO(opener_add_cip_object)

#######################################
# Creates options for trace level     #
####################################### 
macro(createTraceLevelOptions)
  add_definitions( -DOPENER_WITH_TRACES )
  set( TRACE_LEVEL 0 )
  set( OpENer_TRACE_LEVEL_ERROR ON CACHE BOOL "Error trace level" )
  set( OpENer_TRACE_LEVEL_WARNING ON CACHE BOOL "Warning trace level" )
  set( OpENer_TRACE_LEVEL_STATE ON CACHE BOOL "State trace level" )
  set( OpENer_TRACE_LEVEL_INFO ON CACHE BOOL "Info trace level" )
  
  if(OpENer_TRACE_LEVEL_ERROR)
    math( EXPR TRACE_LEVEL "${TRACE_LEVEL} + 1" )
  endif(OpENer_TRACE_LEVEL_ERROR)
  if(OpENer_TRACE_LEVEL_WARNING)
    math( EXPR TRACE_LEVEL "${TRACE_LEVEL} + 2" )
  endif(OpENer_TRACE_LEVEL_WARNING)
  if(OpENer_TRACE_LEVEL_STATE)
    math( EXPR TRACE_LEVEL "${TRACE_LEVEL} + 4" )
  endif(OpENer_TRACE_LEVEL_STATE)
  if(OpENer_TRACE_LEVEL_INFO)
    math( EXPR TRACE_LEVEL "${TRACE_LEVEL} + 8" )
  endif(OpENer_TRACE_LEVEL_INFO)
  
  add_definitions(-DOPENER_TRACE_LEVEL=${TRACE_LEVEL})
endmacro(createTraceLevelOptions)
