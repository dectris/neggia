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

#ifndef DATASETS_FIXTURE_H
#define DATASETS_FIXTURE_H

#include <gtest/gtest.h>
#include <sstream>
#include <string>

class TestDataset : public ::testing::Test {
public:
    void SetUp();
    std::string getTargetDataset(size_t i) const;
    size_t getNumberOfImages() const;
    size_t getNumberOfTriggers() const;

    constexpr static size_t WIDTH = 11;
    constexpr static size_t HEIGHT = 13;
    constexpr static size_t N_FRAMES_PER_DATASET = 5;
    constexpr static double X_PIXEL_SIZE = 0.25;
    constexpr static double Y_PIXEL_SIZE = 0.5;

    unsigned int pixelMaskData[WIDTH * HEIGHT];
    using DATA_TYPE = uint16_t;
    DATA_TYPE dataArray[HEIGHT * WIDTH];

protected:
    virtual size_t getNumberOfDatasets() const = 0;
    virtual std::string getPathToSourceFile() const = 0;
};

class TestDatasetArtificialSmall001 : public TestDataset {
protected:
    size_t getNumberOfDatasets() const override { return 1; }
    std::string getPathToSourceFile() const override {
        return "h5-testfiles/dataset_artificial_small_001/test_master.h5";
    }
};

class TestDatasetArtificialLarge001 : public TestDataset {
protected:
    size_t getNumberOfDatasets() const override { return 1000; }
    std::string getPathToSourceFile() const override {
        return "h5-testfiles/dataset_artificial_large_001/test_master.h5";
    }
};

#endif  // DATASETS_FIXTURE_H