// SPDX-License-Identifier: MIT

#include <dectris/neggia/data/H5DataspaceMsg.h>
#include <gtest/gtest.h>

TEST(TestH5DataspaceMsgV1, CanBeParsed) {
    // see "Dataspace Message - Version 1"
    // https://support.hdfgroup.org/HDF5/doc/H5.format.html#DataspaceMessage
    // clang-format off
    const unsigned char data[56] = {
        0x01, 0x03, 0x01, 0x00, // version 1, dimensionality 3, flags 0x00 (max dimensions contained)
        0x00, 0x00, 0x00, 0x00, // reserved 4 bytes
        0x05, 0x00, 0x00, 0x00, // dimension #1: 5
        0x00, 0x00, 0x00, 0x00,
        0x0d, 0x00, 0x00, 0x00, // dimension #2: 13
        0x00, 0x00, 0x00, 0x00,
        0x0b, 0x00, 0x00, 0x00, // dimension #3: 11
        0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, // max dimension #1: 0xffffffffffffffff
        0xff, 0xff, 0xff, 0xff,
        0x0d, 0x00, 0x00, 0x00, // max dimension #2: 13
        0x00, 0x00, 0x00, 0x00,
        0x0b, 0x00, 0x00, 0x00, // max dimension #3: 11
        0x00, 0x00, 0x00, 0x00,
    };
    // clang-format on
    const auto dims = std::vector<uint64_t>{5, 13, 11};
    const auto maxDims = std::vector<uint64_t>{0xffffffffffffffff, 13, 11};
    const auto msg = H5DataspaceMsg((const char*)data, 0);
    ASSERT_EQ(msg.version(), 1);
    ASSERT_EQ(msg.rank(), 3);
    ASSERT_EQ(msg.maxDims(), true);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(msg.dim(i), dims.at(i));
        EXPECT_EQ(msg.maxDim(i), maxDims.at(i));
    }
}

TEST(TestH5DataspaceMsgV2, CanBeParsed) {
    // see "Dataspace Message - Version 2"
    // https://support.hdfgroup.org/HDF5/doc/H5.format.html#DataspaceMessage
    // clang-format off
    const unsigned char data[52] = {
        0x02, 0x03, 0x01, 0x01, // version 2, dimensionality 3, flags 0x00 (max dimensions contained), type 0x1 (simple)
        0x03, 0x00, 0x00, 0x00, // dimension #1: 3
        0x00, 0x00, 0x00, 0x00,
        0x28, 0x04, 0x00, 0x00, // dimension #2: 1064
        0x00, 0x00, 0x00, 0x00,
        0x06, 0x04, 0x00, 0x00, // dimension #3: 1030
        0x00, 0x00, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff, // max dimension #1: 0xffffffffffffffff
        0xff, 0xff, 0xff, 0xff,
        0x28, 0x04, 0x00, 0x00, // max dimension #2: 1064
        0x00, 0x00, 0x00, 0x00,
        0x06, 0x04, 0x00, 0x00, // max dimension #3: 1030
        0x00, 0x00, 0x00, 0x00,
    };
    // clang-format on
    const auto dims = std::vector<uint64_t>{3, 1064, 1030};
    const auto maxDims = std::vector<uint64_t>{0xffffffffffffffff, 1064, 1030};
    const auto msg = H5DataspaceMsg((const char*)data, 0);
    ASSERT_EQ(msg.version(), 2);
    ASSERT_EQ(msg.rank(), 3);
    ASSERT_EQ(msg.maxDims(), true);
    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(msg.dim(i), dims.at(i));
        EXPECT_EQ(msg.maxDim(i), maxDims.at(i));
    }
}
