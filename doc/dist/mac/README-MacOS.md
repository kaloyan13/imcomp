# MacOS Compilation and Build

## Compiling Dependencies
```
brew install gcc@6 # we use gcc compiler to compiler dependencies and imcomp
cd /Users/tlm/dev/
git clone git@gitlab.com:vgg/imcomp.git
./scripts/install_mac_dep.sh /Users/tlm/deps/imcomp/

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