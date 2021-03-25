// SPDX-License-Identifier: MIT

#include "H5BTreeVersion2.h"
#include <assert.h>
#include <cmath>
#include <iostream>
#include <map>
#include "JenkinsLookup3Checksum.h"

H5BTreeVersion2::H5BTreeVersion2() {
    this->init();
}

H5BTreeVersion2::H5BTreeVersion2(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->init();
}

H5BTreeVersion2::H5BTreeVersion2(const H5Object& obj) : H5Object(obj) {
    this->init();
}

size_t H5BTreeVersion2::getNumberOfRecords() const {
    return _totalNumberOfRecords;
}

H5BTreeVersion2::Node H5BTreeVersion2::getRootNode() const {
    Node rootNode(H5Object(fileAddress(), _rootNodeAddress));
    rootNode.numberOfRecords = _numberOfRecordsInRootNode;
    rootNode.depth = _depth;
    return rootNode;
}

size_t H5BTreeVersion2::getLinkAddressByName(
        const std::string& linkName) const {
    assert(_btreeType == 5);
    Node rootNode = getRootNode();
    uint32_t hash = JenkinsLookup3Checksum(linkName);
    return getRecordAddressWithinInternalNodeFromLinkHash(hash, rootNode);
}

namespace {
template <class T1, class T2>
int chunkCompare(const T1* key0, const T2* key1, size_t len) {
    for (size_t i = len; i > 0; --i) {
        if (key0[i - 1] < key1[i - 1])
            return -1;
        if (key0[i - 1] > key1[i - 1])
            return 1;
    }
    return 0;
}
}  // namespace

size_t H5BTreeVersion2::getChunkAddressByOffset(
        const std::vector<size_t> chunkOffset) const {
    switch (_btreeType) {
        case 10:
        case 11: {
            size_t numDimensions = (_recordSize - 8) / 8;
            Node rootNode = getRootNode();
            return getChunkAddressByOffsetWithinInternalNode(chunkOffset,
                                                             rootNode);
        }
        default:
            throw std::runtime_error("btree type " +
                                     std::to_string((int)_btreeType) +
                                     " not supported to extract data chunks.");
    }
}

size_t H5BTreeVersion2::getChunkAddressByOffsetWithinInternalNode(
        const std::vector<size_t> chunkOffset,
        const Node& node) const {
    bool found = false;
    for (size_t record = 0; record < node.numberOfRecords; ++record) {
        size_t recordOffset = 6 + record * _recordSize;
        size_t chunkAddress = node.read_u64(recordOffset);
        int compare = chunkCompare(
                chunkOffset.data(),
                (const uint64_t*)node.address(recordOffset + _recordSize -
                                              chunkOffset.size() * 8),
                chunkOffset.size());
        if (compare < 0) {
            if (node.depth == 0)
                throw std::out_of_range("chunk not found");
            return getChunkAddressByOffsetWithinInternalNode(
                    chunkOffset, getChildNode(node, record));
        }
        if (compare == 0) {
            return chunkAddress;
        }
    }
    if (node.depth == 0)
        throw std::out_of_range("hash not found");
    return getChunkAddressByOffsetWithinInternalNode(
            chunkOffset, getChildNode(node, node.numberOfRecords));
}

void H5BTreeVersion2::init() {
    std::string signature = std::string(address(), 4);
    assert(signature == "BTHD");
    uint32_t version = read_u8(4);
    assert(version == 0);
    _btreeType = read_u8(5);
    _nodeSize = read_u32(6);
    _recordSize = read_u16(10);
    _depth = read_u16(12);
    _rootNodeAddress = read_u64(16);
    _numberOfRecordsInRootNode = read_u16(24);
    _totalNumberOfRecords = read_u64(26);

    _maximumNumberOfRecordsOfLeaveNodes =
            (_nodeSize - PREFIX_SIZE) / _recordSize;
    _sizeOfNumberOfRecordsForChildNode = getNumberOfBytesNeededToStoreValue(
            _maximumNumberOfRecordsOfLeaveNodes);
    _sizeOfTotalNumberOfRecordsForChild =
            getSizeOfTotalNumberOfRecordsForChildNode();
}

std::vector<size_t> H5BTreeVersion2::getSizeOfTotalNumberOfRecordsForChildNode()
        const {
    std::vector<size_t> sizeOfTotalNumberOfRecordsForChild(_depth + 1);
    sizeOfTotalNumberOfRecordsForChild[0] = 0;
    if (_depth > 0) {
        size_t totalMaximumNumberOfRecordsOfChildNodes =
                _maximumNumberOfRecordsOfLeaveNodes;
        sizeOfTotalNumberOfRecordsForChild[1] = 0;
        for (size_t d = 2; d <= _depth; ++d) {
            size_t internalPointerMultipletSizeOfChildNodes =
                    8 + _sizeOfNumberOfRecordsForChildNode +
                    sizeOfTotalNumberOfRecordsForChild[d - 1];
            size_t maximumNumberOfRecordsOfChildNodes =
                    (_nodeSize -
                     (PREFIX_SIZE + internalPointerMultipletSizeOfChildNodes)) /
                    (_recordSize + internalPointerMultipletSizeOfChildNodes);
            totalMaximumNumberOfRecordsOfChildNodes =
                    ((maximumNumberOfRecordsOfChildNodes + 1) *
                     totalMaximumNumberOfRecordsOfChildNodes) +
                    maximumNumberOfRecordsOfChildNodes;
            sizeOfTotalNumberOfRecordsForChild[d] =
                    getNumberOfBytesNeededToStoreValue(
                            totalMaximumNumberOfRecordsOfChildNodes);
        }
    }
    return sizeOfTotalNumberOfRecordsForChild;
}

size_t H5BTreeVersion2::getSizeOfChildPointerMultiplet(size_t depth) const {
    size_t returnValue = 8 + _sizeOfNumberOfRecordsForChildNode +
                         _sizeOfTotalNumberOfRecordsForChild[depth];
    return returnValue;
}

size_t H5BTreeVersion2::getRecordAddressWithinInternalNode(
        size_t record,
        const Node& node) const {
    if (record < node.numberOfRecords) {
        return node.offset() + 6 + record * _recordSize;
    } else {
        size_t cumulatedNumberOfRecordsLowerLimit = node.numberOfRecords;
        for (size_t i = 0; i <= node.numberOfRecords; ++i) {
            size_t cumulatedNumberOfRecordsUpperLimit =
                    cumulatedNumberOfRecordsLowerLimit +
                    getTotalNumberOfRecordsForChildNode(node, i);
            if (record < cumulatedNumberOfRecordsUpperLimit) {
                return getRecordAddressWithinInternalNode(
                        record - cumulatedNumberOfRecordsLowerLimit,
                        getChildNode(node, i));
            } else {
                cumulatedNumberOfRecordsLowerLimit =
                        cumulatedNumberOfRecordsUpperLimit;
            }
        }
        throw std::runtime_error("file corrupted");
    }
}

size_t H5BTreeVersion2::getRecordAddressWithinInternalNodeFromLinkHash(
        uint32_t linkHash,
        const Node& node) const {
    for (size_t record = 0; record < node.numberOfRecords; ++record) {
        size_t recordOffset = 6 + record * _recordSize;
        uint32_t recordHash = node.read_u32(recordOffset);
        if (linkHash < recordHash) {
            if (node.depth == 0)
                throw std::out_of_range("hash not found");
            return getRecordAddressWithinInternalNodeFromLinkHash(
                    linkHash, getChildNode(node, record));
        } else if (linkHash == recordHash) {
            return node.offset() + recordOffset;
        } else {
            continue;
        }
    }
    if (node.depth == 0)
        throw std::out_of_range("hash not found");
    return getRecordAddressWithinInternalNodeFromLinkHash(
            linkHash, getChildNode(node, node.numberOfRecords));
}

H5BTreeVersion2::Node H5BTreeVersion2::getChildNode(
        const H5BTreeVersion2::Node& parentNode,
        size_t childNodeNumber) const {
    Node childNode(H5Object(fileAddress(),
                            getChildNodeAddress(parentNode, childNodeNumber)));
    childNode.numberOfRecords =
            getNumberOfRecordsForChildNode(parentNode, childNodeNumber);
    childNode.depth = parentNode.depth - 1;
    return childNode;
}

size_t H5BTreeVersion2::getChildNodeAddress(const Node& parentNode,
                                            size_t childNodeNumber) const {
    assert(parentNode.depth > 0);
    size_t address =
            6 + _recordSize * parentNode.numberOfRecords +
            childNodeNumber * getSizeOfChildPointerMultiplet(parentNode.depth);
    return parentNode.read_u64(address);
}

size_t H5BTreeVersion2::getNumberOfRecordsForChildNode(
        const Node& parentNode,
        size_t childNodeNumber) const {
    assert(parentNode.depth > 0);
    size_t address =
            6 + _recordSize * parentNode.numberOfRecords +
            childNodeNumber * getSizeOfChildPointerMultiplet(parentNode.depth) +
            8;
    return parentNode.readIntegerAt(address,
                                    _sizeOfNumberOfRecordsForChildNode);
}

size_t H5BTreeVersion2::getTotalNumberOfRecordsForChildNode(
        const Node& parentNode,
        size_t childNodeNumber) const {
    assert(parentNode.depth > 0);
    if (parentNode.depth == 1) {
        return getNumberOfRecordsForChildNode(parentNode, childNodeNumber);
    } else {
        size_t address = 6 + _recordSize * parentNode.numberOfRecords +
                         childNodeNumber * getSizeOfChildPointerMultiplet(
                                                   parentNode.depth) +
                         8 + _sizeOfNumberOfRecordsForChildNode;
        return parentNode.readIntegerAt(
                address, _sizeOfTotalNumberOfRecordsForChild[parentNode.depth]);
    }
}

size_t H5BTreeVersion2::getNumberOfBytesNeededToStoreValue(size_t value) const {
    return (size_t)((std::log2(value) / 8) + 1);
}

H5BTreeVersion2::Node::Node(const H5Object& obj) : H5Object(obj) {}
