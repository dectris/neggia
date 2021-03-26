// SPDX-License-Identifier: MIT

#include <dectris/neggia/data/H5BLinkNode.h>
#include <dectris/neggia/data/H5DataLayoutMsg.h>
#include <dectris/neggia/data/H5DataspaceMsg.h>
#include <dectris/neggia/data/H5DatatypeMsg.h>
#include <dectris/neggia/data/H5FilterMsg.h>
#include <dectris/neggia/data/H5LinkInfoMessage.h>
#include <dectris/neggia/data/H5LinkMsg.h>
#include <dectris/neggia/data/H5Superblock.h>
#include <dectris/neggia/data/JenkinsLookup3Checksum.h>
#include <dectris/neggia/user/Dataset.h>
#include <dectris/neggia/user/H5File.h>
#include <gtest/gtest.h>
#include <iostream>

template <typename ValueType>
void CheckFrames(const std::string& filename,
                 const std::string& h5path,
                 bool isChunked) {
    Dataset ds(H5File(filename), h5path);
    ASSERT_EQ(ds.dataSize(), sizeof(ValueType));
    ASSERT_EQ(ds.isChunked(), isChunked);
    size_t nframes = ds.dim().at(0);
    ASSERT_GT(nframes, 0);
    size_t pixel_count = ds.dim().at(1) * ds.dim().at(2);
    ASSERT_GT(pixel_count, 0);
    if (isChunked) {
        auto datasetArray =
                std::unique_ptr<ValueType[]>(new ValueType[pixel_count]);
        for (size_t frame = 0; frame < nframes; ++frame) {
            ds.read(datasetArray.get(), {frame, 0, 0});
            ValueType value = frame;
            for (size_t i = 0; i < pixel_count; ++i) {
                ASSERT_EQ(datasetArray[i], value)
                        << "expected pixel value of " << value << " for frame "
                        << frame << " [pixel index: " << i << "]";
            }
        }
    } else {
        auto datasetArray = std::unique_ptr<ValueType[]>(
                new ValueType[pixel_count * nframes]);
        ds.read(datasetArray.get());
        for (size_t frame = 0; frame < nframes; ++frame) {
            ValueType value = frame;
            for (size_t i = 0; i < pixel_count; ++i) {
                ASSERT_EQ(datasetArray[i + frame * pixel_count], value)
                        << "expected pixel value of " << value << " for frame "
                        << frame << " [pixel index: " << i + frame * pixel_count
                        << "]";
            }
        }
    }
}

template <typename ValueType>
void CheckDataMessages(const std::string& filename,
                       const H5Path& path,
                       bool is_chunked) {
    H5File h5File(filename);
    H5Superblock superblock(h5File.fileAddress());
    auto resolvedPath = superblock.resolve(path);
    ASSERT_FALSE(resolvedPath.externalFile);

    uint8_t dataspace_version = (superblock.version() == 0) ? 1 : 2;
    uint8_t datalayout_version = (superblock.version() < 3) ? 3 : 4;
    uint8_t datalayout_class = is_chunked ? 2 : 1;

    size_t testedMessages = 0;
    for (int i = 0, e = resolvedPath.objectHeader.numberOfMessages(); i < e;
         ++i) {
        auto msg = resolvedPath.objectHeader.headerMessage(i);
        switch (msg.type) {
            case H5DataspaceMsg::TYPE_ID: {
                auto typedMsg = H5DataspaceMsg(msg.object);
                ASSERT_EQ(typedMsg.version(), dataspace_version);
                ASSERT_EQ(typedMsg.rank(), 3);
                testedMessages++;
                break;
            }
            case H5DataLayoutMsg::TYPE_ID: {
                auto typedMsg = H5DataLayoutMsg(msg.object);
                ASSERT_EQ(typedMsg.version(), datalayout_version);
                ASSERT_EQ(typedMsg.layoutClass(), datalayout_class);
                testedMessages++;
                break;
            }
            case H5DatatypeMsg::TYPE_ID: {
                auto typedMsg = H5DatatypeMsg(msg.object);
                ASSERT_EQ(typedMsg.version(), 1);
                ASSERT_EQ(typedMsg.typeId(), 0);
                ASSERT_EQ(typedMsg.dataSize(), sizeof(ValueType));
                testedMessages++;
                break;
            }
        }
    }
    ASSERT_EQ(testedMessages, 3);
}

template <typename ValueType>
void CheckNotChunkedData(const std::string& filename) {
    CheckDataMessages<ValueType>(filename, H5Path("/data/not_chunked"), false);
    CheckFrames<ValueType>(filename, "/data/not_chunked", false);
    CheckFrames<ValueType>(filename, "/data/not_chunked_hard", false);
    CheckFrames<ValueType>(filename, "/data/not_chunked_soft", false);
    CheckFrames<ValueType>(filename, "/data/not_chunked_external", false);
}

template <typename ValueType>
void CheckChunkedData(const std::string& filename) {
    CheckDataMessages<ValueType>(filename, H5Path("/data/chunked"), true);

    // Verify content of frames
    CheckFrames<ValueType>(filename, "/data/chunked", true);
    CheckFrames<ValueType>(filename, "/data/chunked_bslz4", true);
    CheckFrames<ValueType>(filename, "/data/chunked_fixed_array", true);
    CheckFrames<ValueType>(filename, "/data/chunked_fixed_array_bslz4", true);
    CheckFrames<ValueType>(filename, "/data/chunked_fixed_array_paged_bslz4",
                           true);
    CheckFrames<ValueType>(filename, "/data/chunked_unlimited_maxshape", true);
    CheckFrames<ValueType>(filename, "/data/chunked_unlimited_maxshape_bslz4",
                           true);

    // Try accessing the same data through links
    CheckFrames<ValueType>(filename, "/data/chunked_hard", true);
    CheckFrames<ValueType>(filename, "/data/chunked_soft", true);
    CheckFrames<ValueType>(filename, "/data/chunked_external", true);
    CheckFrames<ValueType>(filename, "/sub/chunked_hard", true);
    CheckFrames<ValueType>(filename, "/sub/chunked_soft", true);
    CheckFrames<ValueType>(filename, "/sub/chunked_external", true);
    CheckFrames<ValueType>(filename, "/data/sub_hard/chunked_hard", true);
    CheckFrames<ValueType>(filename, "/data/sub_hard/chunked_soft", true);
    CheckFrames<ValueType>(filename, "/data/sub_hard/chunked_external", true);
    CheckFrames<ValueType>(filename, "/data/sub_soft/chunked_hard", true);
    CheckFrames<ValueType>(filename, "/data/sub_soft/chunked_soft", true);
    CheckFrames<ValueType>(filename, "/data/sub_soft/chunked_external", true);
}

template <typename ValueType>
void CheckFile(const std::string& filename, uint8_t superblock_version) {
    H5File h5File(filename);
    H5Superblock superblock(h5File.fileAddress());
    ASSERT_EQ(superblock.version(), superblock_version);
    auto resolvedPath = superblock.resolve(H5Path("/data"));
    int links = 0;
    for (int i = 0, e = resolvedPath.objectHeader.numberOfMessages(); i < e;
         ++i) {
        // We need to make sure that each type of link exists in /data
        // as the creation of these messages in superblock version 0
        // is creation order dependent and under some circumstances
        // hard link messages won't be created.
        auto msg = resolvedPath.objectHeader.headerMessage(i);
        switch (msg.type) {
            case H5LinkMsg::TYPE_ID: {
                auto typedMsg = H5LinkMsg(msg.object);
                links |= (1 << typedMsg.linkType());
                break;
            }
            case H5LinkInfoMsg::TYPE_ID: {
                auto typedMsg = H5LinkInfoMsg(msg.object);
                auto hard = typedMsg.getLinkMessage(h5File.fileAddress(),
                                                    "chunked_hard");
                links |= (1 << hard.linkType());
                auto soft = typedMsg.getLinkMessage(h5File.fileAddress(),
                                                    "chunked_soft");
                links |= (1 << soft.linkType());
                auto ext = typedMsg.getLinkMessage(h5File.fileAddress(),
                                                   "chunked_external");
                links |= (1 << ext.linkType());
                break;
            }
        }
    }
    ASSERT_EQ(links, 0x7);
    CheckChunkedData<ValueType>(filename);
    CheckNotChunkedData<ValueType>(filename);
}

TEST(TestEarliest, CanReadData) {
    CheckFile<uint32_t>(
            "h5-testfiles/datasets_different_h5ver/dataset_earliest.h5", 0);
}

TEST(TestV108, CanReadData) {
    CheckFile<uint32_t>("h5-testfiles/datasets_different_h5ver/dataset_v108.h5",
                        2);
}

TEST(TestV110, CanReadData) {
    CheckFile<uint32_t>("h5-testfiles/datasets_different_h5ver/dataset_v110.h5",
                        3);
}

TEST(TestV112, CanReadData) {
    CheckFile<uint32_t>("h5-testfiles/datasets_different_h5ver/dataset_v112.h5",
                        3);
}
