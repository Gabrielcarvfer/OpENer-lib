#Fixed definitions
unset(CMAKE_LINK_LIBRARY_SUFFIX)

#Output folders
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../build/bin)
set(CMAKE_HEADER_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/../build/include)

#macros

#fetch all .hpp files with full path and set as ${includes}
macro(opener_common_includes)
    HEADER_DIRECTORIES(includes)
    include_directories(${includes})

    #workaround for test building without calling process_options
    if(OpENer_DEBUG)
        add_definitions(-std=c++11)
    else()
        add_definitions(-std=c++11 -g)
    endif()
endmacro(opener_common_includes)

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list ${PROJECT_SOURCE_DIR}/*.hpp)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()
#-------------------------------------------------------------

#process all options passed in main cmakeLists
macro(process_options)
    #process platform switch
    if (OpENer_PLATFORM STREQUAL WIN32)
    else()
        add_definitions(-D__linux__)
    endif()

    #process thread switch
    if (OpENer_USETHREAD)
        add_definitions(-DUSETHREAD)
    endif()

    #process trace switch
    if(OpENer_TRACES)
        createTraceLevelOptions()
    endif(OpENer_TRACES)

    #process test switch
    if( OpENer_TESTS )
        add_subdirectory(tests)
    endif( OpENer_TESTS )

    #process debug switch
    if(OpENer_DEBUG)
        add_definitions(-std=c++11)
    else()
        add_definitions(-std=c++11 -g)
    endif()
endmacro()
#----------------------------------------------

#Copy all header files to outputfolder/include/
FILE(GLOB_RECURSE include_files ${PROJECT_SOURCE_DIR}/*.hpp)
file(COPY ${include_files} DESTINATION ${CMAKE_HEADER_OUTPUT_DIRECTORY})