# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

cmake_minimum_required(VERSION 3.1)
project (fairytale)
option(BUILD_ZLIB "BUILD ZLIB" ON)

set (CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
	set(CMAKE_C_FLAGS "-Wall")
	set(CMAKE_C_FLAGS_DEBUG "-g")
	set(CMAKE_C_FLAGS_RELEASE "-O2")
	set(CMAKE_C_FLAGS_CUSTOM "-Ofast -march=native")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	set(CMAKE_CXX_FLAGS "-Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
	set(CMAKE_CXX_FLAGS_CUSTOM "-Ofast -march=native")
endif()


## SOURCES
set(TRANSFORMSCPP transforms/zlibtransform.cpp)
set(PARSERSCPP parsers/ddsparser.cpp parsers/modparser.cpp parsers/textparser.cpp parsers/jsonparser.cpp)
set(MAINCPP fairytale.cpp analyser.cpp block.cpp deduper.cpp filestream.cpp hybridstream.cpp storagemanager.cpp)

## CONFIGURE ZLIB FOR BUILD
if (BUILD_ZLIB) 
	message("BUILDING ZLIB TOO")
	include_directories(fairytale contrib/zlib)
	list(APPEND ZLIB_SOURCES	
				contrib/zlib/inflate.c 
				contrib/zlib/infback.c 
				contrib/zlib/trees.c 
				contrib/zlib/compress.c 
				contrib/zlib/inftrees.c 
				contrib/zlib/zutil.c 
				contrib/zlib/deflate.c 
				contrib/zlib/inffast.c 
				contrib/zlib/uncompr.c 
				contrib/zlib/adler32.c 
				contrib/zlib/crc32.c)
endif(BUILD_ZLIB)

add_executable(fairytale ${TRANSFORMSCPP} ${PARSERSCPP} ${MAINCPP} ${ZLIB_SOURCES})

if (NOT BUILD_ZLIB)
	message("Using Installed ZLIB")
	find_package(ZLIB REQUIRED)
	if (ZLIB_FOUND)
		include_directories(${ZLIB_INCLUDE_DIRS})
		target_link_libraries(fairytale ${ZLIB_LIBRARIES})
	endif(ZLIB_FOUND)	
endif(NOT BUILD_ZLIB)

file(GLOB test_source test/*.cpp)
add_executable(runTests ${test_source})

target_link_libraries(runTests gtest_main)
