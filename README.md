Neggia
===========

XDS plugin that reads hdf5 files written by Dectris Eiger Detectors.
Please note that a recent version of XDS (Version June 1, 2017 or later) is required for this to work.

## Naming

Neggia - mountain pass in the Italian-speaking part of Switzerland pronounced "neh-jah".
Wikipedia: https://en.wikipedia.org/wiki/Alpe_di_Neggia

## HDF5 file compatibility requirements

```
/entry/instrument/detector/x_pixel_size
    type: float32 or float64
    if missing neggia will set qx to 0.0 when calling get_header()

/entry/instrument/detector/y_pixel_size
    type: float32 or float64
    if missing neggia will set qy to 0.0 when calling get_header()

/entry/instrument/detector/detectorSpecific/nimages
    type: any integer
    neggia will throw an error if value zero or negative

/entry/instrument/detector/detectorSpecific/ntrigger
    type: any integer
    neggia will throw an error if value zero or negative
    neggia will assume ntrigger = 1 if value is missing

/entry/instrument/detector/detectorSpecific/pixel_mask
    type: uint32
    neggia will apply pixel_mask and set data to
          -1 for pixel_mask & 0b00001
          -2 for pixel_mask & 0b11110
          -1 if data value larger than max of signed int32
    neggia will convert to uint32 if type is different and
    throw an error if any value is negative

/entry/data
    group must contain links to datasets:
    'data_000001' to 'data_999999'
    for a single h5 file without links to external datasets
    '/entry/data/data' will be used to extract image data
```

You can check the compatibility requirements by running our test script
against your hdf5 master files:

```
pip3 install hdf5plugin h5py

bin/check_h5_compatibility.py your_master_file.h5
```

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
