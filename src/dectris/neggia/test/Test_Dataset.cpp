// SPDX-License-Identifier: MIT

#include <dectris/neggia/user/Dataset.h>
#include <dectris/neggia/user/H5File.h>
#include "DatasetsFixture.h"

TEST_F(TestDatasetArtificialSmall001, KeepsFileOpen) {
    Dataset xp(H5File(getPathToSourceFile()),
               "/entry/instrument/detector/x_pixel_size");
    float val;
    xp.read(&val);
    ASSERT_EQ(val, X_PIXEL_SIZE);
}

TEST_F(TestDatasetArtificialSmall001, MasterFile) {
    {
        Dataset xp(H5File(getPathToSourceFile()),
                   "/entry/instrument/detector/x_pixel_size");
        assert(xp.dim().empty());
        assert(xp.isChunked() == false);
        assert(xp.dataTypeId() == 1);
        assert(xp.dataSize() == sizeof(float));
        float val;
        xp.read(&val);
        ASSERT_EQ(val, X_PIXEL_SIZE);
    }
    {
        Dataset yp(H5File(getPathToSourceFile()),
                   "/entry/instrument/detector/y_pixel_size");
        assert(yp.dim().empty());
        assert(yp.isChunked() == false);
        assert(yp.dataTypeId() == 1);
        assert(yp.dataSize() == sizeof(float));
        float val;
        yp.read(&val);
        ASSERT_EQ(val, Y_PIXEL_SIZE);
    }
    {
        Dataset pixelMask(
                H5File(getPathToSourceFile()),
                "/entry/instrument/detector/detectorSpecific/pixel_mask");
        unsigned int pixelMaskArray[HEIGHT * WIDTH];
        pixelMask.read(pixelMaskArray);
        ASSERT_EQ(memcmp(pixelMaskArray, pixelMaskData,
                         WIDTH * HEIGHT * sizeof(int)),
                  0);
    }
}

TEST_F(TestDatasetArtificialSmall001, FollowLinkToGroup) {
    Dataset(H5File(getPathToSourceFile()),
            "/entry/link_to_detector_group/x_pixel_size");
}

TEST_F(TestDatasetArtificialSmall001, DataFile) {
    for (size_t datasetid = 0; datasetid < getNumberOfDatasets(); ++datasetid) {
        Dataset dataset(H5File(getPathToSourceFile()),
                        getTargetDataset(datasetid));
        for (size_t i = 0; i < N_FRAMES_PER_DATASET; ++i) {
            DATA_TYPE dataArrayCompare[HEIGHT * WIDTH];
            dataset.read(dataArrayCompare, {i, 0, 0});
            ASSERT_EQ(memcmp(dataArrayCompare, dataArray, sizeof(dataArray)),
                      0);
        }
    }
}

TEST_F(TestDatasetArtificialLarge001, LargeDataFile) {
    for (size_t datasetid = 0; datasetid < getNumberOfDatasets(); ++datasetid) {
        Dataset dataset(H5File(getPathToSourceFile()),
                        getTargetDataset(datasetid));
        DATA_TYPE dataArrayCompare[HEIGHT * WIDTH];
        size_t testFrame = N_FRAMES_PER_DATASET - 1;
        dataset.read(dataArrayCompare, {testFrame, 0, 0});
        ASSERT_EQ(memcmp(dataArrayCompare, dataArray, sizeof(dataArray)), 0);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;
    return RUN_ALL_TESTS();
}
