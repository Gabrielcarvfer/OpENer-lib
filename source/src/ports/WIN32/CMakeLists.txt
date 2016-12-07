#add_subdirectory(sample_application)

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform specific things        #
#######################################
opener_platform_support("INCLUDES")


add_executable(OpENerMain main.cpp)

target_link_libraries( OpENerMain OpENer )#SAMPLE_APP  )
