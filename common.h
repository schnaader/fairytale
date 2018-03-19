/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Márcio Pais

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COMMON_H
#define COMMON_H

#if defined(_WIN32) || defined(_MSC_VER)
  #ifndef WINDOWS
    #define WINDOWS  //to compile for Windows
  #endif
#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
  #ifndef UNIX
    #define UNIX //to compile for Unix, Linux, Solaris, MacOS / Darwin, etc)
  #endif
#endif

#if !defined(WINDOWS) && !defined(UNIX)
#error Unknown target system
#endif

enum class Endian { Little, Big };

#ifdef BIG_ENDIAN
  #define betoh64(x) (x) // Big-Endian to Host translation, 64 bit
  #define betoh32(x) (x) // Big-Endian to Host translation, 32 bit
  #define betoh16(x) (x) // Big-Endian to Host translation, 16 bit
  #if defined(__GNUG__) || defined(__clang__)
    #define letoh64(x) (x=__builtin_bswap64(x)) // Little-Endian to Host translation, 64 bit
    #define letoh32(x) (x=__builtin_bswap32(x)) // Little-Endian to Host translation, 32 bit
    #define letoh16(x) (x=__builtin_bswap16(x)) // Little-Endian to Host translation, 16 bit
  #elif defined(_MSC_VER)
    #define letoh64(x) (x=_byteswap_uint64(x))  // Little-Endian to Host translation, 64 bit
    #define letoh32(x) (x=_byteswap_ulong(x))   // Little-Endian to Host translation, 32 bit
    #define letoh16(x) (x=_byteswap_ushort(x))  // Little-Endian to Host translation, 16 bit
  #endif
#else
  #define letoh64(x) (x) // Little-Endian to Host translation, 64 bit
  #define letoh32(x) (x) // Little-Endian to Host translation, 32 bit
  #define letoh16(x) (x) // Little-Endian to Host translation, 16 bit
  #if defined(__GNUG__) || defined(__clang__)
    #define betoh64(x) (x=__builtin_bswap64(x)) // Big-Endian to Host translation, 64 bit
    #define betoh32(x) (x=__builtin_bswap32(x)) // Big-Endian to Host translation, 32 bit
    #define betoh16(x) (x=__builtin_bswap16(x)) // Big-Endian to Host translation, 16 bit
  #elif defined(_MSC_VER)
    #define betoh64(x) (x=_byteswap_uint64(x))  // Big-Endian to Host translation, 64 bit
    #define betoh32(x) (x=_byteswap_ulong(x))   // Big-Endian to Host translation, 32 bit
    #define betoh16(x) (x=_byteswap_ushort(x))  // Big-Endian to Host translation, 16 bit
  #endif
#endif

#ifdef UNIX
  #include <dirent.h> //opendir(), readdir(), dirent()
  #include <string.h> //strlen(), strcpy(), strcat(), strerror(), memset(), memcpy(), memmove()
  #include <limits.h> //PATH_MAX (for OSX)
  #include <unistd.h> //isatty()
  #include <errno.h>  //errno
  #include <new>      //std::bad_alloc()
#else
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include <time.h>   //clock_t, CLOCKS_PER_SEC
  #include <io.h>     //_open_osfhandle
  #include <Fcntl.h>  //_O_APPEND
#endif

// Platform-independent includes
#include <sys/stat.h> //stat(), mkdir(), stat()
#include <math.h>     //floor(), sqrt()
#include <stdexcept>  //std::exception
#include <assert.h>
#include <cinttypes> //PRIu64   (C99 standard, available since VS 2013 RTM)
#ifndef _MSC_VER
#include <stddef.h>
#endif

#define _FILE_OFFSET_BITS 64
#if defined(WINDOWS) && (defined(__x86_64) || defined(_M_X64))
#define off_t int64_t
#endif
#ifdef _MSC_VER
  #define fseeko(a,b,c) _fseeki64(a,b,c)
  #define ftello(a) _ftelli64(a)
#else
  #ifndef UNIX
    #ifndef fseeko
      #define fseeko(a,b,c) fseeko64(a,b,c)
    #endif
    #ifndef ftello
      #define ftello(a) ftello64(a)
    #endif
  #endif
#endif

#ifdef WINDOWS
  #ifdef GetTempFileName
    #undef GetTempFileName
  #endif
  #define GetTempFileName  GetTempFileNameW
  #ifdef CreateFile
    #undef CreateFile
  #endif
  #define CreateFile  CreateFileW
#endif

#define VERBOSE
#ifdef VERBOSE
  #define LOG(fmt, ...) do { printf(fmt, ##__VA_ARGS__); } while (0)
#else
  #define LOG(fmt, ...)
#endif
#ifndef NDEBUG
  #define TRACE(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#else
  #define TRACE(fmt, ...)
#endif

#define MAX_INDEXABLE size_t((1ull<<(sizeof(void*)*8-1))-1)
#define MEM_LIMIT(mem) (mem>MAX_INDEXABLE?MAX_INDEXABLE:size_t(mem))

#define GENERIC_BUFFER_SIZE 0x8000ll //32KB
#if (GENERIC_BUFFER_SIZE&(GENERIC_BUFFER_SIZE-1)) || (GENERIC_BUFFER_SIZE<=4096)
#error GENERIC_BUFFER_SIZE must be a power of 2 bigger than 4096
#endif

#define MAX_RECURSION_LEVEL 4

inline int min(int a, int b) { return a<b?a:b; }
inline int max(int a, int b) { return a<b?b:a; }
inline uint32_t min(uint32_t a, uint32_t b) { return a<b?a:b; }
inline uint32_t max(uint32_t a, uint32_t b) { return a<b?b:a; }
#ifndef BIT32
inline size_t min(size_t a, size_t b) { return a<b?a:b; }
inline size_t max(size_t a, size_t b) { return a<b?b:a; }
#endif
inline int64_t min(int64_t a, int64_t b) { return a<b?a:b; }
inline int64_t max(int64_t a, int64_t b) { return a<b?b:a; }

class ExhaustedStorageException : public std::exception {};

#define TAB 0x09
#define NEW_LINE 0x0A
#define CARRIAGE_RETURN 0x0D
#define SPACE 0x20

#endif
