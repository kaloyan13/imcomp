# MacOS Compilation and Build

## Compiling Dependencies
```
cd /Users/tlm/dev/
git clone git@gitlab.com:vgg/imcomp.git
./scripts/install_mac_clang_deps.sh /Users/tlm/deps/clang/imcomp/

```

## Compiling imcomp
```
brew install caskroom/cask/cmake # install cmake 3.10.x

cd /Users/tlm/builds/imcomp
rm -fr /Users/tlm/builds/imcomp/ && ./scripts/dist/build_mac_release.sh

## or, alternatively,
## cmake DCMAKE_C_COMPILER=/usr/local/bin/gcc-6 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-6 -DCMAKE_BUILD_TYPE=Release -DIMCOMP_SERVER_VERSION_PATCH=3 -DCMAKE_PREFIX_PATH=/Users/tlm/deps/imcomp/lib /Users/tlm/dev/imcomp/
## make -j 8
```
## Building imcomp installers for MacOS

## References
 * [Bundle Programming Guide](https://developer.apple.com/library/content/documentation/CoreFoundation/Conceptual/CFBundles/Introduction/Introduction.html#//apple_ref/doc/uid/10000123i-CH1-SW1)
 * [CMake Bundle Example](https://cmake.org/Wiki/BundleUtilitiesExample)
 * [Using cmake to create bundle](https://feralchicken.wordpress.com/2013/11/28/using-cmake-to-create-a-bundle-for-a-qt4vtkcgal-project/)
 * [Packaging with CPack](https://feralchicken.wordpress.com/2013/11/28/using-cmake-to-create-a-bundle-for-a-qt4vtkcgal-project/)
 * [CPack Generators](https://cmake.org/Wiki/CMake:CPackPackageGenerators)