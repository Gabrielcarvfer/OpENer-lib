
opener_common_includes()
opener_platform_spec()

set( UTILS_SRC random.cpp xorshiftrandom.cpp endianconv.cpp)

add_library( Utils ${UTILS_SRC} )
