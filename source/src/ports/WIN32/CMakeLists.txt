add_subdirectory(sample_application)

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform specific things        #
#######################################
opener_platform_support("INCLUDES")


add_executable(OpENer main.cpp)

target_link_libraries( OpENer CIP SAMPLE_APP ws2_32 ${OpENer_CIP_OBJECTS} )
