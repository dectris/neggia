// SPDX-License-Identifier: MIT

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <array>
#include <iostream>
#include <memory>

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

// We need to use a TestFixture here as each test has
// to call dlclose() on TearDown.
// Otherwise it will lead to errors in XdsPlugin
class TestXdsPlugin : public ::testing::Test {
public:
    void SetUp() {
        pluginHandle = dlopen(PATH_TO_XDS_PLUGIN, RTLD_NOW);
        open_file = (plugin_open_file)dlsym(pluginHandle, "plugin_open");
        get_header =
                (plugin_get_header)dlsym(pluginHandle, "plugin_get_header");
        get_data = (plugin_get_data)dlsym(pluginHandle, "plugin_get_data");
        close_file = (plugin_close_file)dlsym(pluginHandle, "plugin_close");
    }
    void TearDown() { dlclose(pluginHandle); }
    void* pluginHandle;
    plugin_open_file open_file;
    plugin_get_header get_header;
    plugin_get_data get_data;
    plugin_close_file close_file;
    int error_flag;
    int info_array[1024];
    void CheckXdsPlugin(const std::string& filename,
                        int width,
                        int height,
                        float x_pixel_size,
                        float y_pixel_size,
                        int number_of_images,
                        int number_of_triggers) {
        int error_flag;
        int info_array[1024];

        open_file(filename.c_str(), info_array, &error_flag);
        ASSERT_EQ(error_flag, 0);

        int nx, ny, nbytes, number_of_frames;
        float qx, qy;
        get_header(&nx, &ny, &nbytes, &qx, &qy, &number_of_frames, info_array,
                   &error_flag);
        ASSERT_EQ(error_flag, 0);

        // Assert values from 'get_header'
        ASSERT_EQ(nx, width);
        ASSERT_EQ(ny, height);
        ASSERT_EQ(qx, x_pixel_size);
        ASSERT_EQ(qy, y_pixel_size);
        ASSERT_EQ(number_of_frames, number_of_images * number_of_triggers);

        // Assert that all frames can be extracted with 'get_data'
        auto dataArrayExtracted = std::unique_ptr<int[]>(new int[nx * ny]);
        for (int i = 0; i < number_of_images * number_of_triggers; ++i) {
            int frameNumber = i + 1;
            get_data(&frameNumber, &nx, &ny, dataArrayExtracted.get(),
                     info_array, &error_flag);
            ASSERT_EQ(error_flag, 0);
        }

        close_file(&error_flag);
        ASSERT_EQ(error_flag, 0);
    };
};

/* All tests were generated with describe_dataset_xds.py
   which uses h5py to extract all values from hdf5 files.
   The tests therefore check if we can extract the
   same values with the xds plugin. */

TEST_F(TestXdsPlugin, Eiger1MasterOnlyBSLZ4) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_0datafiles_4images_bslz4_master.h5",
            1030, 1065, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger1With2DatafilesBSLZ4) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_2datafiles_4images_bslz4_master.h5",
            1030, 1065, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger1With2DatafilesLZ4) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_2datafiles_4images_lz4_master.h5",
            1030, 1065, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger2With2DatafilesLZ4) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_lz4_master.h5",
            1028, 512, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger2MasterOnlyBSLZ4) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_0datafiles_4images_bslz4_master.h5",
            1028, 512, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger2With2DatafilesBSLZ4) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_bslz4_master.h5",
            1028, 512, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger2With2DatafilesBSLZ4Uint8) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_bslz4_uint8_master.h5",
            1028, 512, 7.5e-5, 7.5e-5, 4, 1);
}

TEST_F(TestXdsPlugin, Eiger2With2DatafilesBSLZ4Uint32) {
    CheckXdsPlugin(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_bslz4_uint32_master.h5",
            1028, 512, 7.5e-5, 7.5e-5, 4, 1);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(catch_exceptions) = false;
    return RUN_ALL_TESTS();
}
