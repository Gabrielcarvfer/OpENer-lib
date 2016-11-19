#######################################
# CIP library                         #
#######################################

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform-specific includes      #
#######################################
opener_platform_support("INCLUDES")

set( CIP_CLASS_SRC CIP_Attribute.cpp CIP_ClassInstance.cpp CIP_Service.cpp CIP_Template.cpp)

