# Files to compile
set(source_files
    main.cpp
    main.h
    client.cpp
    client.h
    database.cpp
    database.h
    collection.cpp
    collection.h
    bson/core.cpp
    bson/core.h
    bson/types.cpp
    bson/types.h
    #bson/types/document.cpp # Maybe i will use it.
    #bson/types/document.h
    bson/types/oid.cpp
    bson/types/oid.h
)

# Configuration files
configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Adding source files to dll
add_library(${PROJECT_NAME} SHARED ${source_files})

# Including boost if found
if (${Boost_FOUND})
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES})
endif()

# MongoDB libraries
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::bsoncxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared)

# Renaming dll
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${output_name})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dll")

# Compiling dll
#install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin RENAME ${output_name})