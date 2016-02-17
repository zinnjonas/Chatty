cmake_minimum_required(VERSION 2.8.11)

## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "chatty" )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-extra/")

## Set our project name
project(${PROJECT_NAME})

include_directories(
    /home/jonas/develop/cpp/Chatty/
    /home/jonas/develop/cpp/Chatty/

)

add_definitions(
    -DNDEBUG
)

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L\".\" ")

# Define the CXX sources
set ( CXX_SRCS
    main.cpp mongo.cpp database.cpp xmpp.cpp mime.cpp
)

set( CMAKE_VERBOSE_MAKEFILE on)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -O2 -Wall")

add_executable(${PROJECT_NAME} ${CXX_SRCS} ${C_SRCS})

find_package(Boost REQUIRED system)
include_directories(${Boost_INCLUDE_DIRS})

find_package(MongoDB REQUIRED)
include_directories(${MONGO_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${MONGO_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${Boost_SYSTEM_LIBRARIES})

target_link_libraries(${PROJECT_NAME} curses)

find_package( LibStrophe REQUIRED)

include_directories(${LibStrophe_INCLUDE_DIR})
target_link_libraries( ${PROJECT_NAME} ${LibStrophe_LIBRARY})

find_package(CryptoPP REQUIRED)
include_directories(${PROJECT_NAME} ${CRYPTOPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${CRYPTOPP_LIBRARIES})

find_package(LibXml2 REQUIRED)
include_directories(${PROJECT_NAME} ${LIBXML2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})
