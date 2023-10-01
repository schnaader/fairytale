[![Build Status](https://travis-ci.org/schnaader/fairytale.svg?branch=master)](https://travis-ci.org/schnaader/fairytale)
[![Build status](https://ci.appveyor.com/api/projects/status/k3y23dpxfu4rm108?svg=true)](https://ci.appveyor.com/project/schnaader/fairytale)
[![Join the chat at https://gitter.im/encode-ru-Community-Archiver/Lobby](https://badges.gitter.im/encode-ru-Community-Archiver/Lobby.svg)](https://gitter.im/encode-ru-Community-Archiver/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Important
---------
**This repository is moving to [Gitlab](https://gitlab.com/schnaader/fairytale) - please update your bookmarks and do any contributions there. The GitHub repository will be removed after successful migration.**

# Fairytale
A modern lossless community archiver featuring:

* State of the art analysis of input data
* Detection and transform / recompression of data to improve compression
* Sorting and arranging input it in different streams
* Deduplication across all streams and recursion levels
* Applying optimal compression algorithms on each stream
* Storing compressed data in a modern archiver format

Fairytale offers great modularity so that any algorithm can be added to it. Fairytale also brings flexbility in choosing between fast, best, practical, or experimental compression options. Fairytale is currently under production and seeks to be the next generation archiver.

How to build
------------

Using CMake ([install](https://cmake.org/download/)), you can build on many platforms using your favorite compiler (Visual Studio, MinGW, CodeBlocks, XCode, Unix Makefiles, ...). It will also detect automatically if zlib is installed and if not, compiles it from source.

For Windows, the make.bat batch script that works with MinGW. Use `make` for a 64-bit build and `make 32` for a 32-bit build.

For Linux, OSX and ARM, there are Makefiles. Use `make` for a 64-bit build, `make -f Makefile.32` for a 32-bit build.

Releases/Binaries
-----------------

Please note that this is a very rough prototype that allows for testing of the pre-processing library.
It doesn't apply any compression right now.

[ARM](https://drive.google.com/file/d/1Uc1w3Sf0J8A2wGZtcYtIDpHjcSuX8oY7/view)

[Linux](..)

[OSX](..)

[Windows](https://drive.google.com/drive/folders/1uj2YVjpbRscJiM0llTU-9uJuY5BmgBvt)

Contributing
------------
We are excited that you want to contribute to Fairytale. Our goal is to solidify this project and your contributions are appreciated. 

To contribute:
- Fork the repository
- Create a new branch
- Apply changes and open Pull Request to merge
    - Ensure the PR has concise title and detailed description
    - Description should include chagnes made and why they were made

To report bugs:
- Check current issue list to see if other users have experienced the bug. If they have, make a comment on that issue.
- If you experience a new bug, open up an issue in GitHub or contact us via email.

Join our [community](https://gitter.im/encode-ru-Community-Archiver) to stay updated and discuss improvments for Fairytale!

Contact
-------
Email: *Enter email here*    
Any other contacts 

License
-------

Licensed under the [LGPL-3.0 license](https://github.com/schnaader/fairytale/blob/master/LICENSE)
