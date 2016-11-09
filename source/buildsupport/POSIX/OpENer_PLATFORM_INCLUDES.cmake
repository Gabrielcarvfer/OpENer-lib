macro(opener_platform_spec)
  include_directories(${PORTS_SRC_DIR}/${OpENer_PLATFORM} ${PORTS_SRC_DIR}/${OpENer_PLATFORM}/sample_application)
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -std=c99 -Werror=implicit-function-declaration" )
  add_definitions( -D_POSIX_C_SOURCE=199309L )
endmacro(opener_platform_spec)

