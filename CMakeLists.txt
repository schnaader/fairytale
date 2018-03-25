cmake_minimum_required(VERSION 3.1)
project (fairytale)
option(BUILD_ZLIB "BUILD ZLIB" OFF)

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
set(PARSERSCPP parsers/ddsparser.cpp parsers/modparser.cpp parsers/textparser.cpp)
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
