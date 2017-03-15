#ifndef TESTH5DATASET_H
#define TESTH5DATASET_H

#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <cpp/H5Cpp.h>
#include <hdf5_hl.h>


class H5DatasetTestFixture: public ::testing::Test {
public:
    H5DatasetTestFixture();
    virtual void SetUp();
    virtual void TearDown();

    std::string getPathToSourceFile() const;
    std::string getPathToTargetFile(size_t i) const;
    std::string getSourceFile() const;
    std::string getTargetFile(size_t i) const;
    std::string getTargetDataset(size_t i) const;

    virtual size_t getNumberOfDatasets() const;
    size_t getNumberOfImages() const;
    size_t getNumberOfTriggers() const;
    constexpr static size_t WIDTH = 11;
    constexpr static size_t HEIGHT = 13;
    constexpr static size_t N_FRAMES_PER_DATASET = 5;
    constexpr static double X_PIXEL_SIZE = 0.25;
    constexpr static double Y_PIXEL_SIZE = 0.5;
    unsigned int pixelMaskData[WIDTH*HEIGHT];
    using DATA_TYPE = uint16_t;
    DATA_TYPE dataArray[HEIGHT*WIDTH];

private:
    const std::string testDir;
    const std::string sourceFile;
    std::vector<std::string> targetFile;
    std::vector<std::string> targetDataset;
    void setupPixelMaskData();
    void setupData();
    void setupH5MasterFile(size_t numberOfDatasets);
    void setupH5DataFiles();
    template<class T> void writeValue(H5::Group &, const std::string & identifier, const T & data);
    void writePixelMask(H5::Group &);
    void writeNumberOfImages(H5::Group &);
    void writeNumberOfTriggers(H5::Group &);
    void writeXPixelSize(H5::Group &g);
    void writeYPixelSize(H5::Group &g);
    void mkdirRecursively(const std::string & directory);
};

#endif // TESTH5DATASET_H
