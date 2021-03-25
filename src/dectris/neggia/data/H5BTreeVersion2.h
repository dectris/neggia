// SPDX-License-Identifier: MIT

#ifndef BTREEVERSION2_H
#define BTREEVERSION2_H
#include <dectris/neggia/data/H5Object.h>
#include <string>
#include <vector>

class H5BTreeVersion2 : public H5Object {
public:
    H5BTreeVersion2();
    H5BTreeVersion2(const char* fileAddress, size_t offset);
    H5BTreeVersion2(const H5Object& obj);
    size_t getNumberOfRecords() const;
    size_t getLinkAddressByName(const std::string& linkName) const;
    size_t getChunkAddressByOffset(const std::vector<size_t> chunkOffset) const;

private:
    struct Node : public H5Object {
        Node(const H5Object&);
        size_t numberOfRecords;
        size_t depth;
    };

    void init();
    size_t getNumberOfBytesNeededToStoreValue(size_t value) const;
    std::vector<size_t> getSizeOfTotalNumberOfRecordsForChildNode() const;
    size_t getSizeOfChildPointerMultiplet(size_t depth) const;
    size_t getRecordAddressWithinInternalNode(size_t record,
                                              const Node& node) const;
    size_t getRecordAddressWithinInternalNodeFromLinkHash(
            uint32_t linkHash,
            const Node& node) const;
    size_t getChunkAddressByOffsetWithinInternalNode(
            const std::vector<size_t> chunkOffset,
            const Node& node) const;
    Node getChildNode(const Node& parentNode, size_t childNodeNumber) const;
    size_t getChildNodeAddress(const Node& parentNode,
                               size_t childNodeNumber) const;
    size_t getNumberOfRecordsForChildNode(const Node& parentNode,
                                          size_t childNodeNumber) const;
    size_t getTotalNumberOfRecordsForChildNode(const Node& parentNode,
                                               size_t childNodeNumber) const;
    Node getRootNode() const;

    constexpr static size_t PREFIX_SIZE = 10;
    uint8_t _btreeType;
    size_t _nodeSize;
    size_t _recordSize;
    size_t _depth;
    uint64_t _rootNodeAddress;
    size_t _numberOfRecordsInRootNode;
    size_t _totalNumberOfRecords;
    size_t _maximumNumberOfRecordsOfLeaveNodes;
    size_t _sizeOfNumberOfRecordsForChildNode;
    std::vector<size_t> _sizeOfTotalNumberOfRecordsForChild;
};

#endif  // BTREEVERSION2_H
