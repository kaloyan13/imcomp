* install git for windows from https://git-scm.com/download/win

* git clone git@gitlab.com:vgg/imcomp.git

* Install dependencies
 * install boost library
	- https://sourceforge.net/projects/boost/files/boost-binaries/1.65.1/boost_1_65_1-msvc-14.1-64.exe/download
    - Update imcomp/CMakeLists.txt file to point to boost install location
	
 * install imagemagick library
	- only 32 bit library is found by cmake : ImageMagick-6.9.9-33-Q16-x86-dll.exe and ImageMagick-6.9.9-33-Q8-x86-dll.exe
	- why?
 
* Download and install cmake from https://cmake.org/download/ and run cmake-gui tool
	- help
		- https://dmerej.info/blog/post/cmake-visual-studio-and-the-command-line/
	- cd C:\Users\tlm\build\imcomp
	- "c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_GENERATOR_PLATFORM=x64
	- "c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_GENERATOR_PLATFORM=x86 ## @todo: setup configuration
	- devenv /build Debug imcomp_server.sln
