Neggia
===========

XDS plugin that reads hdf5 files written by Dectris Eiger Detectors.
Please note that a recent version of XDS (Version June 1, 2017 or later) is required for this to work.

## Build

### Prerequisites
* gcc-4.8 or higher
* On a centos6 machine install and enable devtoolset
  * yum -y install centos-release-scl-rh
  * yum install -y devtoolset-6-gcc-c++
  * scl enable devtoolset-6 bash
* cmake 3.6 or later
  * CentOS: yum install -y cmake3
  * OSX: install binary from https://cmake.org
* For running tests
  * Google Test (https://github.com/google/googletest.git)

### Building
Use cmake or cmake3, depending on what your cmake version 3 executable is called.
* mkdir build
* cd build
* cmake ..
* make

The plugin file is found in build/src/dectris/neggia/plugin/dectris-neggia.so
