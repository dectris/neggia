// SPDX-License-Identifier: MIT

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
