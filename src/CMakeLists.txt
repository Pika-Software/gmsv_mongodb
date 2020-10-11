# Files to compile
set(SOURCES
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

# Generating configure file
configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE gmod-module-base)
set_gmod_suffix_prefix(${PROJECT_NAME})

# Visual Studio pre 2017 requires boost polyfill.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_STANDARD LESS 17)
    find_package(Boost 1.56.0 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
endif() 

target_include_directories(${PROJECT_NAME} PRIVATE ${LIBMONGOCXX_STATIC_INCLUDE_DIRS} ${LIBBSONCXX_STATIC_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBMONGOCXX_STATIC_LIBRARIES} ${LIBBSONCXX_STATIC_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${LIBMONGOCXX_STATIC_DEFINITIONS} ${LIBBSONCXX_STATIC_DEFINITIONS})