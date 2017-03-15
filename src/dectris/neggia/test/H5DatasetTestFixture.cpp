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

#include "H5DatasetTestFixture.h"
#include <dectris/neggia/data/H5Path.h>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <unistd.h>
#include "Lz4Filter.hpp"
#include "bshuf_h5filter.h"
#include <thread>


constexpr double H5DatasetTestFixture::X_PIXEL_SIZE;
constexpr double H5DatasetTestFixture::Y_PIXEL_SIZE;
constexpr size_t H5DatasetTestFixture::WIDTH;
constexpr size_t H5DatasetTestFixture::HEIGHT;

namespace  {
const H5::PredType & getH5Type(uint8_t *) {
    return H5::PredType::NATIVE_UINT8;
}
const H5::PredType & getH5Type(uint16_t *) {
    return H5::PredType::NATIVE_UINT16;
}
const H5::PredType & getH5Type(uint32_t *) {
    return H5::PredType::NATIVE_UINT32;
}
const H5::PredType & getH5Type(uint64_t *) {
    return H5::PredType::NATIVE_UINT64;
}
const H5::PredType & getH5Type(int8_t *) {
    return H5::PredType::NATIVE_INT8;
}
const H5::PredType & getH5Type(int16_t *) {
    return H5::PredType::NATIVE_INT16;
}
const H5::PredType & getH5Type(int32_t *) {
    return H5::PredType::NATIVE_INT32;
}
const H5::PredType & getH5Type(int64_t *) {
    return H5::PredType::NATIVE_INT64;
}
const H5::PredType & getH5Type(float *) {
    return H5::PredType::NATIVE_FLOAT;
}
const H5::PredType & getH5Type(double *) {
    return H5::PredType::NATIVE_DOUBLE;
}
template<class T> const H5::PredType & getH5Type(T = T()){
    using bareType = typename std::remove_pointer<T>::type;
    return getH5Type((bareType*)nullptr);
}

std::string getThreadId() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

std::vector<std::string> splitPathStringIntoComponents(const std::string &path)
{
    std::stringstream pathStream(path);
    std::string item;
    std::vector<std::string> returnValue;
    while(std::getline(pathStream, item, '/')) {
        if(!item.empty()) returnValue.push_back(item);
    }
    return returnValue;
}

bool isAbsolutePath(const std::string & path) {
    return path.size()>0 && path[0] == '/';
}

}



H5DatasetTestFixture::H5DatasetTestFixture():
    testDir("/tmp/neggia/test_" + std::to_string(getpid()) + "_" + getThreadId()),
    sourceFile("test_master.h5") {}

void H5DatasetTestFixture::SetUp()
{
    mkdirRecursively(testDir);
    const size_t NUMBER_OF_DATASETS = getNumberOfDatasets();
    targetFile.resize(NUMBER_OF_DATASETS);
    targetDataset.resize(NUMBER_OF_DATASETS);
    for(size_t i=0; i<NUMBER_OF_DATASETS; ++i) {
        std::stringstream str;
        str << "test_data_"<< std::setfill('0') << std::setw(6) << i+1 << ".h5";
        targetFile[i] = str.str();
    }
    for(size_t i=0; i<NUMBER_OF_DATASETS; ++i) {
        std::stringstream str;
        str << "/entry/data/data_"<< std::setfill('0') << std::setw(6) << i+1;
        targetDataset[i] = str.str();
    }
    setupPixelMaskData();
    setupData();
    setupH5MasterFile(NUMBER_OF_DATASETS);
    setupH5DataFiles();
}

void H5DatasetTestFixture::TearDown()
{
    unlink(getPathToSourceFile().c_str());
    const size_t NUMBER_OF_DATASETS = getNumberOfDatasets();
    for(size_t datasetid=0; datasetid < NUMBER_OF_DATASETS; ++datasetid) {
        unlink(getPathToTargetFile(datasetid).c_str());
    }
}

std::string H5DatasetTestFixture::getPathToSourceFile() const
{
    return testDir + "/" + sourceFile;
}

std::string H5DatasetTestFixture::getPathToTargetFile(size_t i) const
{
    return testDir + "/" + targetFile[i];
}

std::string H5DatasetTestFixture::getSourceFile() const
{
    return sourceFile;
}

std::string H5DatasetTestFixture::getTargetFile(size_t i) const
{
    return targetFile[i];
}

std::string H5DatasetTestFixture::getTargetDataset(size_t i) const
{
    return targetDataset[i];
}

size_t H5DatasetTestFixture::getNumberOfDatasets() const
{
    return 1;
}

size_t H5DatasetTestFixture::getNumberOfImages() const
{
    return N_FRAMES_PER_DATASET * getNumberOfDatasets();
}

size_t H5DatasetTestFixture::getNumberOfTriggers() const
{
    return 1;
}

void H5DatasetTestFixture::setupPixelMaskData()
{
    for(size_t i=0; i<HEIGHT*WIDTH; ++i) pixelMaskData[i] = i%3;
}

void H5DatasetTestFixture::setupData()
{
    for(size_t i=0; i<HEIGHT*WIDTH; ++i) dataArray[i] = (DATA_TYPE)i;
}

template<class T> void H5DatasetTestFixture::writeValue(H5::Group & g, const std::string &identifier, const T & data)
{
    auto dataset = g.createDataSet(identifier, getH5Type(data), H5::DataSpace());
    dataset.write(&data, getH5Type(data));
}

void H5DatasetTestFixture::writePixelMask(H5::Group & g)
{
    hsize_t dims[2] = {HEIGHT, WIDTH};
    H5::DataSpace dataspace( 2, dims, dims);
    auto pixelMask = g.createDataSet("pixel_mask", getH5Type(pixelMaskData), dataspace);
    pixelMask.write(pixelMaskData, getH5Type(pixelMaskData));
}

void H5DatasetTestFixture::writeNumberOfImages(H5::Group & g)
{
    writeValue(g, "nimages", (int)getNumberOfImages());
}

void H5DatasetTestFixture::writeNumberOfTriggers(H5::Group & g)
{
    writeValue(g, "ntrigger", (int)getNumberOfTriggers());
}

void H5DatasetTestFixture::writeXPixelSize(H5::Group &  g)
{
    writeValue(g, "x_pixel_size", (float)X_PIXEL_SIZE);
}

void H5DatasetTestFixture::writeYPixelSize(H5::Group &  g)
{
    writeValue(g, "y_pixel_size", (float)Y_PIXEL_SIZE);
}

void H5DatasetTestFixture::mkdirRecursively(const std::string &directory)
{
    auto path(splitPathStringIntoComponents(directory));
    char buffer[1024];
    std::string directoryToBeCreated = isAbsolutePath(directory)? "" : getcwd(buffer, sizeof(buffer));
    for(const auto & item: path) {
        directoryToBeCreated = directoryToBeCreated + "/" + item;
        mkdir(directoryToBeCreated.c_str(), 0777);
    }
}

void H5DatasetTestFixture::setupH5MasterFile(size_t numberOfDatasets)
{
    H5::H5File h5Source(getPathToSourceFile(), H5F_ACC_TRUNC);
    H5::Group entry = h5Source.createGroup("entry");
    auto g = entry.createGroup("instrument");
    g = g.createGroup("detector");
    writeXPixelSize(g);
    writeYPixelSize(g);

    H5Lcreate_soft("/entry/instrument/detector", h5Source.getId(), "/entry/link_to_detector_group", H5P_DEFAULT, H5P_DEFAULT);

    g = g.createGroup("detectorSpecific");
    writePixelMask(g);
    writeNumberOfImages(g);
    writeNumberOfTriggers(g);
    entry.createGroup("data");
    for(size_t i=0; i<numberOfDatasets; ++i) {
        H5Lcreate_external(targetFile[i].c_str(), "/entry/data/data",
                           h5Source.getId(), targetDataset[i].c_str(), H5P_DEFAULT, H5P_DEFAULT);
    }
    h5Source.close();
}

void H5DatasetTestFixture::setupH5DataFiles()
{
    for(size_t datasetid=0; datasetid<getNumberOfDatasets(); ++datasetid) {
        H5::H5File h5target(getPathToTargetFile(datasetid), H5F_ACC_TRUNC);
        H5::Group entry = h5target.createGroup("entry");
        H5::Group dataGroup = entry.createGroup("data");

        H5::DSetCreatPropList propertyList;
        hsize_t chunkDimensions[3] = {1,HEIGHT,WIDTH};
        propertyList.setChunk( 3, chunkDimensions);
        h5RegisterLZ4Comression();
        bshuf_register_h5filter();
        propertyList.setFilter(h5LZ4FilterId(), H5Z_FLAG_MANDATORY, 0, NULL);
        hsize_t maxDims[3] = {H5S_UNLIMITED, HEIGHT, WIDTH};
        H5::DataSpace dataspace( 3, chunkDimensions, maxDims);
        H5::DataSet dataset(dataGroup.createDataSet( "data", getH5Type(dataArray), dataspace, propertyList));
        for(size_t i=0; i<N_FRAMES_PER_DATASET; ++i){
            hsize_t offsetArray[3] = {i,0,0};
            hsize_t extensionArray[3] = {i+1,HEIGHT,WIDTH};
            dataset.extend(extensionArray);
            H5::DataSpace fileSpace(dataset.getSpace());
            fileSpace.selectHyperslab(H5S_SELECT_SET, chunkDimensions, offsetArray);
            H5::DataSpace memSpace(3,chunkDimensions);
            dataset.write(dataArray,  getH5Type(dataArray), memSpace, fileSpace);
        }
        dataset.close();
        h5target.close();
    }
}

