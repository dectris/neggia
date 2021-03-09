Neggia
===========

XDS plugin that reads hdf5 files written by Dectris Eiger Detectors.
Please note that a recent version of XDS (Version June 1, 2017 or later) is required for this to work.

## Naming

Neggia - mountain pass in the Italian-speaking part of Switzerland pronounced "neh-jah".
Wikipedia: https://en.wikipedia.org/wiki/Alpe_di_Neggia

## Build & Test

Please use only tagged release commits for your production environment.
See https://github.com/dectris/neggia/releases

### Prerequisites
* gcc-4.8 or higher
* On a centos6 machine install and enable devtoolset
  * yum -y install centos-release-scl-rh
  * yum install -y devtoolset-6-gcc-c++
  * scl enable devtoolset-6 bash
* cmake 3.6 or later
  * CentOS: yum install -y cmake3
  * OSX: install binary from https://cmake.org

### Building
Use cmake or cmake3, depending on what your cmake version 3 executable is called.
* mkdir build
* cd build
* cmake .. -DCMAKE_BUILD_TYPE=Release
* cmake --build .

The plugin file is found in `build/src/dectris/neggia/plugin/dectris-neggia.so`

### Testing
Use cmake or cmake3, depending on what your cmake version 3 executable is called.
* git submodule update --init
* mkdir build
* cd build
* cmake .. -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
* cmake --build .
* ctest --output-on-failure
