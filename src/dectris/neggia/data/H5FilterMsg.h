// SPDX-License-Identifier: MIT

#ifndef H5FILTERMSG_H
#define H5FILTERMSG_H
#include <string>
#include <vector>
#include "H5Object.h"
/// https://support.hdfgroup.org/HDF5/doc/H5.format.html#FilterMessage

struct FilterDescription {
    uint16_t id;
    std::string name;
    std::vector<int32_t> client_data;
};

class H5FilterMsg : public H5Object {
public:
    H5FilterMsg() = default;
    H5FilterMsg(const char* fileAddress, size_t offset);
    H5FilterMsg(const H5Object&);
    uint8_t version() const;
    unsigned int nFilters() const;
    uint16_t filterId(int i) const;
    std::string filterName(int i) const;
    std::vector<int32_t> clientData(int i) const;
    constexpr static unsigned int TYPE_ID = 0xb;

private:
    void _init();
    void _initV1();
    void _initV2();

    std::vector<FilterDescription> _filters;
};

#endif  // H5FILTERMSG_H
