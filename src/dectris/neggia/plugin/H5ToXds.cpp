/**
MIT License

Copyright (c) 2017 DECTRIS Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "H5ToXds.h"
#include <dectris/neggia/user/H5File.h>
#include <dectris/neggia/user/Dataset.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include "H5Error.h"


namespace {

struct H5DataCache {
   std::string filename;
   H5File h5File;
   int dimx;
   int dimy;
   int datasize;
   int nframesPerDataset;
   std::unique_ptr<uint32_t[]> pixelMask;
   float xpixelSize;
   float ypixelSize;
};

std::unique_ptr<H5DataCache> GLOBAL_HANDLE = nullptr;

void printVersionInfo() {
   std::cout << "This is neggia " << VERSION << " (Copyright Dectris 2017)" << std::endl;
}

template<class T> void applyMaskAndTransformToInt32(const T * indata, int outdata[], const uint32_t * maskData, size_t size) {
    constexpr size_t maxSigned = (size_t)std::numeric_limits<int>::max();
    for(size_t j=0; j<size; ++j) {
        if(maskData[j] & 0x1) {
            outdata[j] = -1;
        } else if(maskData[j] & 30) { // 30 = 2 + 4 + 8 + 16
            outdata[j] = -2;
        } else if((size_t)indata[j] >= maxSigned) {
            outdata[j] = -1;
        } else {
            outdata[j] = (int)indata[j];
        }
    }
}

template<class Type> Type readFromDataset(const Dataset & d) {
    Type val;
    d.read(&val);
    return val;
}

size_t readSizeTypeFromDataset(const Dataset &d) {
    assert(d.dataTypeId() == 0);
    switch(d.dataSize()) {
    case sizeof(uint8_t):
        return readFromDataset<uint8_t>(d);
    case sizeof(uint16_t):
        return readFromDataset<uint16_t>(d);
    case sizeof(uint32_t):
        return readFromDataset<uint32_t>(d);
    case sizeof(uint64_t):
        return readFromDataset<uint64_t>(d);
    default:
        throw H5Error(-4, "UNSUPPORTED DATATYPE");
    }
}

double readFloatFromDataset(const Dataset & d) {
    assert(d.dataTypeId() == 1);
    switch(d.dataSize()) {
    case sizeof(float):
        return readFromDataset<float>(d);
    case sizeof(double):
        return readFromDataset<double>(d);
    default:
        throw H5Error(-4, "UNSUPPORTED DATATYPE");
    }
}

H5DataCache * getPreopenedDataCache () {
    H5DataCache * dataCache = GLOBAL_HANDLE.get();
    if (!dataCache) {
       throw H5Error(-2, "ERROR: NO FILE HAS BEEN OPENED YET");
    }
    return dataCache;
}

size_t correctFrameNumberOffset(int frameNumberStartingFromOne) {
    if(frameNumberStartingFromOne < 1) {
        throw H5Error(-2, "ERROR: Framenumbers start from 1");
    }
    return (size_t) frameNumberStartingFromOne - 1;
}

size_t getFrameNumberWithinDataset(size_t globalFrameNumber, const H5DataCache * dataCache) {
    return globalFrameNumber %  (size_t)dataCache->nframesPerDataset;
}

std::string getPathToDataset(size_t globalFrameNumber, const H5DataCache* dataCache)
{
    size_t datasetNumber = globalFrameNumber / dataCache->nframesPerDataset + 1;
    std::stringstream ss;
    ss << "/entry/data/data_" << std::setw(6) << std::setfill('0') << datasetNumber;
    return ss.str();
}

void setXPixelSize(H5DataCache* dataCache)
{
    try {
       Dataset d(dataCache->h5File, "/entry/instrument/detector/x_pixel_size");
       dataCache->xpixelSize = (float) readFloatFromDataset(d);
    } catch (const std::out_of_range &) {
       dataCache->xpixelSize = 0;
    }
}

void setYPixelSize(H5DataCache* dataCache)
{
    try {
       Dataset d(dataCache->h5File, "/entry/instrument/detector/y_pixel_size");
       dataCache->ypixelSize = (float) readFloatFromDataset(d);
    } catch (const std::out_of_range &) {
       dataCache->ypixelSize = 0;
    }
}

void setPixelMask(H5DataCache* dataCache) {
    try {
       Dataset pixelMask(dataCache->h5File, "/entry/instrument/detector/detectorSpecific/pixel_mask");
       assert(pixelMask.dataTypeId() == 0);
       assert(pixelMask.dataSize() == sizeof(uint32_t));
       auto dim(pixelMask.dim());
       assert(dim.size() == 2);
       dataCache->dimx = (int)dim[1];
       dataCache->dimy = (int)dim[0];
       size_t s = (size_t )(dataCache->dimx * dataCache->dimy);
       dataCache->pixelMask.reset(new uint32_t[s]);
       pixelMask.read(dataCache->pixelMask.get());
    } catch (const std::out_of_range &) {
       throw H5Error(-4, "ERROR: CANNOT READ PIXEL MASK FROM ", dataCache->filename);
    }
}

size_t getNumberOfImages(const H5DataCache * dataCache) {
    try {
       Dataset d(dataCache->h5File, "/entry/instrument/detector/detectorSpecific/nimages");
       return readSizeTypeFromDataset(d);
    } catch (const std::out_of_range&) {
       throw H5Error(-4, "ERROR: CANNOT READ N_IMAGES FROM ", dataCache->filename);
    } catch (const H5Error &) {
        throw H5Error(-4, "ERROR: UNSUPPORTED DATATYPE FOR N_IMAGES");
    }
}

size_t getNumberOfTriggers(const H5DataCache * dataCache) {
    try {
       Dataset d(dataCache->h5File, "/entry/instrument/detector/detectorSpecific/ntrigger");
       return readSizeTypeFromDataset(d);
    } catch (const std::out_of_range&) {
       throw H5Error(-4, "ERROR: CANNOT READ N_TRIGGER FROM ", dataCache->filename);
    } catch (const H5Error &) {
        throw H5Error(-4, "ERROR: UNSUPPORTED DATATYPE FOR N_TRIGGER");
    }
}

void setNFramesPerDataset(H5DataCache* dataCache)
{
    try {
       Dataset dataset(dataCache->h5File, "/entry/data/data_000001");
       auto dim = dataset.dim();
       assert(dim.size() == 3);
       dataCache->nframesPerDataset = dim[0];
       assert(dataCache->dimy == dim[1]);
       assert(dataCache->dimx == dim[2]);
       dataCache->datasize = dataset.dataSize();
       assert(dataset.dataTypeId() == 0);
       assert(dataset.isChunked());
       assert(dataset.chunkSize() == std::vector<size_t>({1,(unsigned int)dataCache->dimy,(unsigned int)dataCache->dimx}));
    } catch (const std::out_of_range &) {
        throw H5Error(-4, "ERROR: CANNOT OPEN /entry/data/data_000001 FROM ", dataCache->filename);
    }
}

void applyMaskAndTransformToInt32(const H5DataCache* dataCache, const void * indata, int outdata[])
{
    switch(dataCache->datasize) {
    case 1:
        applyMaskAndTransformToInt32((const uint8_t*)indata, outdata, dataCache->pixelMask.get(), dataCache->dimx*dataCache->dimy);
        break;
    case 2:
        applyMaskAndTransformToInt32((const uint16_t*)indata, outdata, dataCache->pixelMask.get(), dataCache->dimx*dataCache->dimy);
        break;
    case 4:
        applyMaskAndTransformToInt32((const uint32_t*)indata, outdata, dataCache->pixelMask.get(), dataCache->dimx*dataCache->dimy);
        break;
    default: {
        throw H5Error(-3, "ERROR: DATATYPE NOT SUPPORTED");
    }
    }
}

void readDataset(int *frame_number, int data_array[], const H5DataCache* dataCache)
{
    size_t globalFrameNumber = correctFrameNumberOffset(*frame_number);
    std::string pathToDataset = getPathToDataset(globalFrameNumber, dataCache);
    try {
        Dataset dataset(dataCache->h5File, pathToDataset);
        size_t totNumberOfDatasets = dataset.dim()[0];
        size_t datasetFrameNumber = getFrameNumberWithinDataset(globalFrameNumber, dataCache);
        if(datasetFrameNumber >= totNumberOfDatasets) throw std::out_of_range("frame_number out of range");
        std::unique_ptr<char[]> buffer(new char[dataCache->dimx*dataCache->dimy*dataCache->datasize]);
        dataset.read(buffer.get(), std::vector<size_t>({datasetFrameNumber, 0, 0}));
        applyMaskAndTransformToInt32(dataCache, buffer.get(), data_array);
    } catch(const std::out_of_range &) {
        throw H5Error(-2, "ERROR: CANNOT OPEN FRAME ", *frame_number);
    }
}

void setInfoArray(int info[1024])
{
    info[0] = DECTRIS_H5TOXDS_CUSTOMER_ID;        // Customer ID [1:Dectris]
    info[1] = DECTRIS_H5TOXDS_VERSION_MAJOR;      // Version  [Major]
    info[2] = DECTRIS_H5TOXDS_VERSION_MINOR;      // Version  [Minor]
    info[3] = DECTRIS_H5TOXDS_VERSION_PATCH;      // Version  [Patch]
    info[4] = DECTRIS_H5TOXDS_VERSION_TIMESTAMP;  // Version  [timestamp]
}


} // namespace

std::unique_ptr<H5DataCache> retVal(new H5DataCache);

extern "C" {


void plugin_open(const char * filename,
                      int info_array[1024],
                      int * error_flag) {
    setInfoArray(info_array);
    *error_flag = 0;
    printVersionInfo();
    std::unique_ptr<H5DataCache> dataCache(new H5DataCache);
    try {
        dataCache->filename = filename;
        dataCache->h5File   = H5File(filename);
    } catch (const std::out_of_range &) {
        std::cerr << "ERROR: CANNOT OPEN " << filename << std::endl;
        *error_flag = -4;
        return;
    }
    if(GLOBAL_HANDLE) {
        std::cerr << "ERROR: CAN ONLY OPEN ONE FILE AT A TIME " << std::endl;
        *error_flag = -4;
        return;
    } else {
        GLOBAL_HANDLE = std::move(dataCache);
    }
}



void plugin_get_header(int *nx, int *ny,
                       int *nbytes,
                       float *qx, float *qy,
                       int * number_of_frames,
                       int info[1024],
                       int *error_flag)
{
    setInfoArray(info);
    try {
        H5DataCache * dataCache = getPreopenedDataCache();
        setXPixelSize(dataCache);
        setYPixelSize(dataCache);
        setPixelMask(dataCache);
        size_t nimages = getNumberOfImages(dataCache);
        size_t ntrigger = getNumberOfTriggers(dataCache);
        setNFramesPerDataset(dataCache);

        *nx = dataCache->dimx;
        *ny = dataCache->dimy;
        *nbytes = dataCache->datasize;
        *qx = dataCache->xpixelSize;
        *qy = dataCache->ypixelSize;
        *number_of_frames= (int)(nimages * ntrigger);

    } catch (const H5Error & error) {
        std::cerr << error.what() << std::endl;
        *error_flag = error.getErrorCode();
        return;
    }
    *error_flag=0;
    return;
}

void plugin_get_data(int *frame_number,
                     int * nx, int * ny,
                     int data_array[],
                     int info_array[1024],
                     int *error_flag)
{
    setInfoArray(info_array);
    try {
        H5DataCache * dataCache = getPreopenedDataCache();
        readDataset(frame_number, data_array, dataCache);
    } catch (const H5Error & error) {
        std::cerr << error.what() << std::endl;
        *error_flag = error.getErrorCode();
        return;
    }
    *error_flag = 0;

}

void plugin_close(int *error_flag){
   GLOBAL_HANDLE.reset();
}

} // extern "C"


