opENer_common_includes()

set( UTILS_SRC random.cpp xorshiftrandom.cpp eletronicDatasheetUtilities.cpp)
add_library( OpENer_UTILS STATIC ${UTILS_SRC})
