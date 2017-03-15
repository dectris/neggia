Neggia
===========

XDS plugin that reads hdf5 files written by Dectris Eiger Detectors

## Build
### Prerequisites
* gcc-4.8
* On a centos6 machine install and enable devtoolset-2
  * wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo
  * yum install -y devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++
  * scl enable devtoolset-2 bash
* cmake 3.6
* DectrisHdf5 (https://github.com/dectris/DectrisHdf5.git)
* Google Test (https://github.com/google/googletest.git)
### Building
* mkdir build
* cd build
* cmake ..
* make
The plugin file is found in src/dectris/neggia/plugin/dectris-neggia.so


