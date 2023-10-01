[![Build Status](https://travis-ci.org/schnaader/fairytale.svg?branch=master)](https://travis-ci.org/schnaader/fairytale)
[![Build status](https://ci.appveyor.com/api/projects/status/k3y23dpxfu4rm108?svg=true)](https://ci.appveyor.com/project/schnaader/fairytale)
[![Join the chat at https://gitter.im/encode-ru-Community-Archiver/Lobby](https://badges.gitter.im/encode-ru-Community-Archiver/Lobby.svg)](https://gitter.im/encode-ru-Community-Archiver/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# Fairytale: Modern Lossless Community Archiver


Important
---------
**This repository has migrated to [Gitlab](https://gitlab.com/schnaader/fairytale). Please update your bookmarks and make contributions there. The GitHub repository will be removed after successful migration.**


# Fairytale is a modern lossless community archiver that boasts several unique features including:

* State-of-the-art analysis of input data.
* Detection and transformation/recompression of data to enhance compression.
* Sorting input data and organizing it into different streams.
* Deduplication across all streams and recursion levels.
* Application of various (best-fit) compression algorithms to each stream.
* Storage of compressed data in a modern archiver format.

The architecture of Fairytale is highly modular, allowing for the addition of any compression algorithm, and offers the flexibility to choose between fast, practical, or experimental state-of-the-art compression methods. Fairytale is a vision of the next-generation archiver and is a work in progress. If you share our dream and want to contribute, please [join our vibrant community here](https://gitter.im/encode-ru-Community-Archiver).

How to build
------------

You can build Fairytale on many platforms using CMake (download it [here](https://cmake.org/download/)) with your favorite compiler (Visual Studio, MinGW, CodeBlocks, XCode, Unix Makefiles, and more). It will also automatically detect if zlib is installed; if not, it will compile it from source.

* For Windows, use the `make.bat` batch script with MinGW. Run `make` for a 64-bit build or `make 32` for a 32-bit build.

* For Linux, OSX, and ARM, there are Makefiles available. Run `make` for a 64-bit build and `make -f Makefile.32` for a 32-bit build.

## Usage

Fairytale is a command-line application with various options and features. Here are some common usage examples within the command-line:

* Compress a file:
  fairytale compress input.txt -o compressed.ftl
  
* Decompress a file:
  fairytale decompress compressed.ftl -o output.txt
  
* Configure compression options:
  fairytale config --level 2

## Performance Tips

To optimize Fairytale's performance, consider the following tips:

* Use higher compression levels for better compression ratios.
  
* Allocate more memory to the application for faster processing.

## Known Issues and Limitations

* Fairytale may not perform well on large files with certain data patterns.
  
* The current version lacks support for multi-threaded compression.


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
