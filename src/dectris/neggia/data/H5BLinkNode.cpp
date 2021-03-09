// SPDX-License-Identifier: MIT

#include "H5BLinkNode.h"
#include <assert.h>
#include <string>

H5BLinkNode::H5BLinkNode(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    assert(std::string(address(), 4) == "TREE");
}

H5BLinkNode::H5BLinkNode(const H5Object& other) : H5Object(other) {
    assert(std::string(address(), 4) == "TREE");
}
