cmake_minimum_required(VERSION 2.8.9)
project (fairytale)

set(TRANSFORMSCPP transforms/zlibtransform.cpp)
set(PARSERSCPP parsers/ddsparser.cpp parsers/modparser.cpp parsers/textparser.cpp)
set(MAINCPP fairytale.cpp analyser.cpp block.cpp deduper.cpp filestream.cpp hybridstream.cpp storagemanager.cpp)


add_executable(fairytale ${TRANSFORMSCPP} ${PARSERSCPP} ${MAINCPP})

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(fairytale ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)
