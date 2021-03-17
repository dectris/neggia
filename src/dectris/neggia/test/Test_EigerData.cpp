// SPDX-License-Identifier: MIT

#include <dectris/neggia/data/H5Superblock.h>
#include <dectris/neggia/data/JenkinsLookup3Checksum.h>
#include <dectris/neggia/user/Dataset.h>
#include <dectris/neggia/user/H5File.h>
#include <gtest/gtest.h>
#include <numeric>
#include <type_traits>

struct DatasetValues {
    std::string entry;
    std::vector<size_t> dim;
    uint32_t checksum;
};

template <typename FloatType, typename IntegerType, typename PixelType>
struct ExpectedValues {
    uint8_t superblock_version;
    IntegerType width;
    IntegerType height;
    FloatType x_pixel_size;
    FloatType y_pixel_size;
    uint32_t flatfield_checksum;
    uint32_t pixel_mask_checksum;
    std::vector<DatasetValues> datasets;
};

template <typename ValueType>
void CheckDataset(const std::string& filename,
                  const std::string& h5path,
                  const ValueType& expected) {
    Dataset ds(H5File(filename), h5path);
    ASSERT_TRUE(ds.dim().empty());
    ASSERT_FALSE(ds.isChunked());
    if (std::is_same<ValueType, float>::value ||
        std::is_same<ValueType, double>::value)
    {
        ASSERT_EQ(ds.dataTypeId(), 1);
    } else {
        ASSERT_EQ(ds.dataTypeId(), 0);
    }
    ASSERT_EQ(ds.dataSize(), sizeof(ValueType));
    ValueType val;
    ds.read(&val);
    ASSERT_EQ(val, expected);
};

template <typename ValueType>
void CheckDatasetChecksum(const std::string& filename,
                          const std::string& h5path,
                          const std::vector<size_t>& dim,
                          uint32_t checksum) {
    Dataset ds(H5File(filename), h5path);
    ASSERT_EQ(ds.dim(), dim);
    ASSERT_EQ(ds.dataSize(), sizeof(ValueType));
    uint32_t checkSumCalculated = 0;
    auto updateChecksum = [&checkSumCalculated, &ds](
                                  size_t pixel_count,
                                  std::vector<size_t> chunkOffset) {
        auto datasetArray =
                std::unique_ptr<ValueType[]>(new ValueType[pixel_count]);
        ds.read(datasetArray.get(), chunkOffset);
        checkSumCalculated = JenkinsLookup3Checksum(
                std::string((const char*)datasetArray.get(),
                            pixel_count * sizeof(ValueType)),
                checkSumCalculated);
    };
    switch (ds.dim().size()) {
        case 2: {
            size_t pixel_count = ds.dim().at(0) * ds.dim().at(1);
            updateChecksum(pixel_count, {});
            break;
        }
        case 3: {
            size_t pixel_count = ds.dim().at(1) * ds.dim().at(2);
            for (size_t frame = 0; frame < ds.dim().at(0); ++frame) {
                updateChecksum(pixel_count, {frame, 0, 0});
            }
            break;
        }
        default:
            throw std::runtime_error("dataset with dimensionality of " +
                                     std::to_string(ds.dim().size()) +
                                     " not supported.");
    }

    ASSERT_EQ(checkSumCalculated, checksum);
}

template <typename FloatType, typename IntegerType, typename PixelType>
void CheckHdf5(
        const std::string& filename,
        const ExpectedValues<FloatType, IntegerType, PixelType>& expected) {
    H5File h5File(filename);
    H5Superblock superblock(h5File.fileAddress());
    ASSERT_EQ(superblock.version(), expected.superblock_version);
    CheckDataset(
            filename,
            "/entry/instrument/detector/detectorSpecific/x_pixels_in_detector",
            expected.width);
    CheckDataset(
            filename,
            "/entry/instrument/detector/detectorSpecific/y_pixels_in_detector",
            expected.height);
    CheckDataset(filename, "/entry/instrument/detector/x_pixel_size",
                 expected.x_pixel_size);
    CheckDataset(filename, "/entry/instrument/detector/y_pixel_size",
                 expected.y_pixel_size);
    CheckDatasetChecksum<uint32_t>(
            filename, "/entry/instrument/detector/detectorSpecific/pixel_mask",
            {expected.height, expected.width}, expected.pixel_mask_checksum);
    CheckDatasetChecksum<float>(
            filename, "/entry/instrument/detector/detectorSpecific/flatfield",
            {expected.height, expected.width}, expected.flatfield_checksum);
    for (auto dataset : expected.datasets) {
        CheckDatasetChecksum<PixelType>(filename,
                                        "/entry/data/" + dataset.entry,
                                        dataset.dim, dataset.checksum);
    }
};

/* All tests were generated with describe_dataset.py
   which uses h5py to extract all values from hdf5 files.
   The tests therefore check if we can extract the
   same values with neggia */

TEST(TestDatasetEiger1, MasterOnlyNoCompression) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_0datafiles_4images_none_uint16_master.h5",
            ExpectedValues<float, uint32_t, uint16_t>{
                    0,
                    1030,
                    1065,
                    7.5e-5,
                    7.5e-5,
                    706946666,
                    2854193483,
                    {{"data", {4, 1065, 1030}, 64404092}},
            });
}

TEST(TestDatasetEiger1, MasterOnlyBSLZ4) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_0datafiles_4images_bslz4_master.h5",
            ExpectedValues<float, uint32_t, uint16_t>{
                    0,
                    1030,
                    1065,
                    7.5e-5,
                    7.5e-5,
                    706946666,
                    2854193483,
                    {{"data", {4, 1065, 1030}, 1530321565}},
            });
}

TEST(TestDatasetEiger1, With2DatafilesBSLZ4) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_2datafiles_4images_bslz4_master.h5",
            ExpectedValues<float, uint32_t, uint16_t>{
                    0,
                    1030,
                    1065,
                    7.5e-5,
                    7.5e-5,
                    706946666,
                    2854193483,
                    {{"data_000001", {2, 1065, 1030}, 3687867133},
                     {"data_000002", {2, 1065, 1030}, 2342047550}},
            });
}

TEST(TestDatasetEiger1, With2DatafilesLZ4) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger1/"
            "eiger1_testmode10_2datafiles_4images_lz4_master.h5",
            ExpectedValues<float, uint32_t, uint16_t>{
                    0,
                    1030,
                    1065,
                    7.5e-5,
                    7.5e-5,
                    706946666,
                    2854193483,
                    {{"data_000001", {2, 1065, 1030}, 3141050780},
                     {"data_000002", {2, 1065, 1030}, 1526186353}},
            });
}

TEST(TestDatasetEiger2, With2DataFilesLZ4) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_lz4_master.h5",
            ExpectedValues<double, uint64_t, uint16_t>{
                    2,
                    1028,
                    512,
                    7.5e-5,
                    7.5e-5,
                    2112054611,
                    1258047512,
                    {{"data_000001", {2, 512, 1028}, 1542871599},
                     {"data_000002", {2, 512, 1028}, 849427427}},
            });
}

TEST(TestDatasetEiger2, MasterOnlyBSLZ4) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_0datafiles_4images_bslz4_master.h5",
            ExpectedValues<double, uint64_t, uint16_t>{
                    2,
                    1028,
                    512,
                    7.5e-5,
                    7.5e-5,
                    2112054611,
                    1258047512,
                    {{"data", {4, 512, 1028}, 1910193827}},
            });
}

TEST(TestDatasetEiger2, With2DataFilesBSLZ4) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_bslz4_master.h5",
            ExpectedValues<double, uint64_t, uint16_t>{
                    2,
                    1028,
                    512,
                    7.5e-5,
                    7.5e-5,
                    2112054611,
                    1258047512,
                    {{"data_000001", {2, 512, 1028}, 341628856},
                     {"data_000002", {2, 512, 1028}, 133841592}},
            });
}

TEST(TestDatasetEiger2, With2DataFilesBSLZ4Uint8) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_bslz4_uint8_master.h5",
            ExpectedValues<double, uint64_t, uint8_t>{
                    2,
                    1028,
                    512,
                    7.5e-5,
                    7.5e-5,
                    2112054611,
                    1258047512,
                    {{"data_000001", {2, 512, 1028}, 3448392636},
                     {"data_000002", {2, 512, 1028}, 4277511416}},
            });
}

TEST(TestDatasetEiger2, With2DataFilesBSLZ4Uint32) {
    CheckHdf5(
            "h5-testfiles/datasets_eiger2/"
            "eiger2_simread7_2datafiles_4images_bslz4_uint32_master.h5",
            ExpectedValues<double, uint64_t, uint32_t>{
                    2,
                    1028,
                    512,
                    7.5e-5,
                    7.5e-5,
                    2112054611,
                    1258047512,
                    {{"data_000001", {2, 512, 1028}, 2022778456},
                     {"data_000002", {2, 512, 1028}, 4211060786}},
            });
}
