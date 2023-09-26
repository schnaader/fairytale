[![Build Status](https://travis-ci.org/schnaader/fairytale.svg?branch=master)](https://travis-ci.org/schnaader/fairytale)
[![Build status](https://ci.appveyor.com/api/projects/status/k3y23dpxfu4rm108?svg=true)](https://ci.appveyor.com/project/schnaader/fairytale)
[![Join the chat](https://badges.gitter.im/encode-ru-Community-Archiver/Lobby.svg)](https://gitter.im/encode-ru-Community-Archiver/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Important
---------
**This repository is moving to [Gitlab](https://gitlab.com/schnaader/fairytale) - please update your bookmarks and do any contributions there. The GitHub repository will be removed after successful migration.**
When accessing our repository from the Open Source projects on Gitlab, the link will redirect to this Github Repository. Please access the Gitlab repository by clicking the Gitlab link above.

# Fairytale
Modern lossless community archiver that features:

* State of the art analysis of input data
* Detection, transform, and recompression of data to improve compression
* Sorting inputs and arranging in different streams
* Deduplication across all streams and recursion levels
* Applying best fit compression algorithms to each stream
* Storing compressed data in a modern archiver format
* Great modularity so any algorithm can be added
* Flexibility to choose between fast, best practical, or experimental compression

Our Mission
------------
Fairytale is a dream of a next generation archiver and it is currently a work in progress.
We wanted to create an archiver with the most optimal speed and compression possible, with features such as recompression and deduplication.
If you want to contribute, [Join Our Community Here](https://gitter.im/encode-ru-Community-Archiver)

How to build
------------

Using CMake ([download](https://cmake.org/download/)), you can build on many platforms using your favorite compiler (Visual Studio, MinGW, CodeBlocks, XCode, Unix Makefiles, ...). It will also detect automatically if zlib is installed and if not, compiles it from source.

For Windows, there's a make.bat batch script that works with MinGW.
* Use `make` for a 64-bit build
* Use `make 32` for a 32-bit build

For Linux, OSX and ARM, there are Makefiles.
* Use `make` for a 64-bit build
* Use `make -f Makefile.32` for a 32-bit build.

Releases/Binaries
-----------------

Please note that this is a very rough prototype that allows for testing of the pre-processing library.
It currently doesn't apply any compression.

[for ARM](https://drive.google.com/file/d/1Uc1w3Sf0J8A2wGZtcYtIDpHjcSuX8oY7/view)

[for Linux](..)

[for OSX](..)

[for Windows](https://drive.google.com/drive/folders/1uj2YVjpbRscJiM0llTU-9uJuY5BmgBvt)

License
-------

Licensed under the [LGPL-3.0 license](https://github.com/schnaader/fairytale/blob/master/LICENSE)
