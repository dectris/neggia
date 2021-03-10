// SPDX-License-Identifier: MIT

#include <dectris/neggia/user/H5File.h>
#include <dlfcn.h>
#include <array>
#include <iostream>
#include "DatasetsFixture.h"

typedef void (*plugin_open_file)(const char*,
                                 int info_array[1024],
                                 int* error_flag);

typedef void (*plugin_get_header)(int* nx,
                                  int* ny,
                                  int* nbytes,
                                  float* qx,
                                  float* qy,
                                  int* number_of_frames,
                                  int info_array[1024],
                                  int* error_flag);

typedef void (*plugin_get_data)(int* frame_number,
                                int* nx,
                                int* ny,
                                int data_array[],
                                int info_array[1024],
                                int* error_flag);

typedef void (*plugin_close_file)(int* error_flag);

class TestXdsPlugin : public TestDatasetArtificialSmall001 {
public:
    void SetUp() {
        TestDataset::SetUp();
        pluginHandle = dlopen(PATH_TO_XDS_PLUGIN, RTLD_NOW);
        open_file = (plugin_open_file)dlsym(pluginHandle, "plugin_open");
        get_header =
                (plugin_get_header)dlsym(pluginHandle, "plugin_get_header");
        get_data = (plugin_get_data)dlsym(pluginHandle, "plugin_get_data");
        close_file = (plugin_close_file)dlsym(pluginHandle, "plugin_close");
        error_flag = 1;
        memset(info_array, 0, sizeof(info_array));
    }
    void TearDown() { dlclose(pluginHandle); }
    void* pluginHandle;
    plugin_open_file open_file;
    plugin_get_header get_header;
    plugin_get_data get_data;
    plugin_close_file close_file;
    int error_flag;
    int info_array[1024];
    constexpr static int DECTRIS_VENDOR = 1;
    using PIXEL_MASK_CORRECTED_ARRAY = std::array<int, WIDTH * HEIGHT>;
    PIXEL_MASK_CORRECTED_ARRAY applyPixelMaskCorrections(
            DATA_TYPE testDataArray[WIDTH * HEIGHT]);
};

constexpr int TestXdsPlugin::DECTRIS_VENDOR;

TestXdsPlugin::PIXEL_MASK_CORRECTED_ARRAY
TestXdsPlugin::applyPixelMaskCorrections(
        DATA_TYPE testDataArray[WIDTH * HEIGHT]) {
    PIXEL_MASK_CORRECTED_ARRAY returnValue;
    for (size_t i = 0; i < WIDTH * HEIGHT; ++i) {
        if (pixelMaskData[i] & 0x1)
            returnValue[i] = -1;
        else if (pixelMaskData[i] & 30)
            returnValue[i] = -2;
        else
            returnValue[i] = testDataArray[i];
    }
    return returnValue;
}

TEST_F(TestXdsPlugin, TestHasOpenMethod) {
    ASSERT_NE(dlsym(pluginHandle, "plugin_open"), nullptr);
}

TEST_F(TestXdsPlugin, TestHasGetHeaderMethod) {
    ASSERT_NE(dlsym(pluginHandle, "plugin_get_header"), nullptr);
}

TEST_F(TestXdsPlugin, TestHasGetDataMethod) {
    ASSERT_NE(dlsym(pluginHandle, "plugin_get_data"), nullptr);
}

TEST_F(TestXdsPlugin, TestHasGetCloseMethod) {
    ASSERT_NE(dlsym(pluginHandle, "plugin_close"), nullptr);
}

TEST_F(TestXdsPlugin, TestOpenAndCloseFile) {
    open_file(getPathToSourceFile().c_str(), info_array, &error_flag);
    ASSERT_EQ(error_flag, 0);
    close_file(&error_flag);
    ASSERT_EQ(error_flag, 0);
}

TEST_F(TestXdsPlugin, TestInfoArray) {
    open_file(getPathToSourceFile().c_str(), info_array, &error_flag);
    ASSERT_EQ(info_array[0], DECTRIS_VENDOR);
    close_file(&error_flag);
}

TEST_F(TestXdsPlugin, TestGetHeader) {
    open_file(getPathToSourceFile().c_str(), info_array, &error_flag);
    int nx, ny, nbytes, number_of_frames;
    float qx, qy;
    get_header(&nx, &ny, &nbytes, &qx, &qy, &number_of_frames, info_array,
               &error_flag);
    ASSERT_EQ(error_flag, 0);
    ASSERT_EQ(nx, WIDTH);
    ASSERT_EQ(ny, HEIGHT);
    ASSERT_EQ(qx, X_PIXEL_SIZE);
    ASSERT_EQ(qy, Y_PIXEL_SIZE);
    ASSERT_EQ(info_array[0], DECTRIS_VENDOR);
    ASSERT_EQ(info_array[1], 0);  // Enforced by XDS
    ASSERT_EQ(number_of_frames, getNumberOfImages() * getNumberOfTriggers());
    close_file(&error_flag);
}

TEST_F(TestXdsPlugin, TestInfoArrayFromGetData) {
    open_file(getPathToSourceFile().c_str(), info_array, &error_flag);
    int nx, ny, nbytes, number_of_frames;
    float qx, qy;
    get_header(&nx, &ny, &nbytes, &qx, &qy, &number_of_frames, info_array,
               &error_flag);
    int frameNumber = 1;
    int dataArrayCompare[nx * ny];
    get_data(&frameNumber, &nx, &ny, dataArrayCompare, info_array, &error_flag);
    ASSERT_EQ(info_array[0], DECTRIS_VENDOR);
}

TEST_F(TestXdsPlugin, TestGetData) {
    open_file(getPathToSourceFile().c_str(), info_array, &error_flag);
    int nx, ny, nbytes, number_of_frames;
    float qx, qy;
    get_header(&nx, &ny, &nbytes, &qx, &qy, &number_of_frames, info_array,
               &error_flag);

    int dataArrayCompare[nx * ny];
    auto expectedArray = applyPixelMaskCorrections(this->dataArray);
    for (int i = 0; i < getNumberOfImages() * getNumberOfTriggers(); ++i) {
        int frameNumber = i + 1;
        get_data(&frameNumber, &nx, &ny, dataArrayCompare, info_array,
                 &error_flag);
        for (size_t i = 0; i < WIDTH * HEIGHT; ++i) {
            ASSERT_EQ(dataArrayCompare[i], expectedArray[i]);
        }
    }
    close_file(&error_flag);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;
    return RUN_ALL_TESTS();
}
