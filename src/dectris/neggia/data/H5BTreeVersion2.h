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
    size_t getRecordAddress(size_t record) const;
    size_t getRecordAddress(const std::string& linkName) const;

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
    Node getChildNode(const Node& parentNode, size_t childNodeNumber) const;
    size_t getChildNodeAddress(const Node& parentNode,
                               size_t childNodeNumber) const;
    size_t getNumberOfRecordsForChildNode(const Node& parentNode,
                                          size_t childNodeNumber) const;
    size_t getTotalNumberOfRecordsForChildNode(const Node& parentNode,
                                               size_t childNodeNumber) const;
    Node getRootNode() const;

    constexpr static size_t PREFIX_SIZE = 10;
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
