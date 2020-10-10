cmake_minimum_required(VERSION 3.10)
project(gmsv_mongodb VERSION 1.0.0)

# Variables
set(VERSION "1.0.0")
set(output_template "gmsv_mongodb_platform")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Detecting platform
if (WIN32) # Windows
    set(output_platform "win32")
elseif (UNIX) # Linux
    set(output_platform "linux")
else() # Mac OS X
    set(output_platform "osx")
endif()

string(REPLACE "platform" ${output_platform} output_name ${output_template}) # https://wiki.facepunch.com/gmod/Creating_Binary_Modules#naminglocation

# Including mongocxx
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})
include_directories("/usr/local/include/mongocxx/v_noabi")
include_directories("/usr/local/include/bsoncxx/v_noabi")
include_directories("/usr/local/include/libmongoc-1.0")
include_directories("/usr/local/include/libbson-1.0")

# Including boost
option(USE_BOOST "If you are use boost" OFF)

if (USE_BOOST)
    set(Boost_USE_STATIC_LIBS OFF) 
    set(Boost_USE_MULTITHREADED ON)  
    set(Boost_USE_STATIC_RUNTIME OFF) 

    find_package( Boost 1.56.0 REQUIRED )
    include_directories(${Boost_INCLUDE_DIR})
endif()

# Other includes
include_directories(include)

add_subdirectory(src)