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

#include "DatasetsFixture.h"

constexpr double TestDataset::X_PIXEL_SIZE;
constexpr double TestDataset::Y_PIXEL_SIZE;
constexpr size_t TestDataset::WIDTH;
constexpr size_t TestDataset::HEIGHT;

void TestDataset::SetUp() {
    for (size_t i = 0; i < HEIGHT * WIDTH; ++i)
        pixelMaskData[i] = i % 3;
    for (size_t i = 0; i < HEIGHT * WIDTH; ++i)
        dataArray[i] = (DATA_TYPE)i;
}

std::string TestDataset::getTargetDataset(size_t i) const {
    std::stringstream str;
    str << "/entry/data/data_" << std::setfill('0') << std::setw(6) << i + 1;
    return str.str();
};

size_t TestDataset::getNumberOfImages() const {
    return N_FRAMES_PER_DATASET * getNumberOfDatasets();
}

size_t TestDataset::getNumberOfTriggers() const {
    return 1;
}