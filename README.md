[![Build Status](https://travis-ci.org/schnaader/fairytale.svg?branch=master)](https://travis-ci.org/schnaader/fairytale)
[![Build status](https://ci.appveyor.com/api/projects/status/k3y23dpxfu4rm108?svg=true)](https://ci.appveyor.com/project/schnaader/fairytale)
[![Join the chat at https://gitter.im/encode-ru-Community-Archiver/Lobby](https://badges.gitter.im/encode-ru-Community-Archiver/Lobby.svg)](https://gitter.im/encode-ru-Community-Archiver/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Important
---------
**This repository is moving to [Gitlab](https://gitlab.com/schnaader/fairytale) - please update your bookmarks and do any contributions there. The GitHub repository will be removed after successful migration.**

# Fairytale
modern lossless community archiver that features

* state of the art analysis of input data
* detection and transform / recompression of data to improve compression
* sorting input and arrange it in different streams
* deduplication across all streams and recursion levels
* applying different (best fit) compression algorithms on each stream
* storing compressed data in a modern archiver format

It offers great modularity so that any algorithm can be added to it
and enough flexibility to chose between fast, best practical or experimental state-of-the-art compression.
Fairytale is a dream of a next generation archiver and it is a work in progress
so if you share our dream and want to contribute, [join our great community here](https://gitter.im/encode-ru-Community-Archiver)

How to build
------------

Using CMake (download it [here](https://cmake.org/download/)), you can build on many platforms using your favorite compiler (Visual Studio, MinGW, CodeBlocks, XCode, Unix Makefiles, ...). It will also detect automatically if zlib is installed and if not, compiles it from source.

For Windows, there's a make.bat batch script that works with MinGW. Use `make` for a 64-bit build, `make 32` for a 32-bit build.

For Linux, OSX and ARM, there are Makefiles. Use `make` for a 64-bit build, `make -f Makefile.32` for a 32-bit build.

Releases/Binaries
-----------------

Please note that this is a very rough prototype that allows for testing of the pre-processing library.
It doesn't apply any compression right now.

[for ARM](https://drive.google.com/file/d/1Uc1w3Sf0J8A2wGZtcYtIDpHjcSuX8oY7/view)

[for Linux](..)

[for OSX](..)

[for Windows](https://drive.google.com/drive/folders/1uj2YVjpbRscJiM0llTU-9uJuY5BmgBvt)

License
-------

Licensed under the [LGPL-3.0 license](https://github.com/schnaader/fairytale/blob/master/LICENSE)
